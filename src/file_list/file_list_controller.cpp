/*
 * NuCommander
 * Copyright (C) 2018  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "file_list_controller.h"

#include <algorithm>
#include <time.h>

#include "file_list/sort_func.h"

#include "tasks/async_task.h"
#include "directory/icon_loader.h"

#include "operations/copy.h"

#include "columns.h"

using namespace nuc;

//// Private Functions

/// Sorting

/**
 * Called when the sort order changes.
 *
 * @param list_store The list store of which the sort order
 *   changed.
 */
static void sort_changed(Gtk::ListStore *list_store);


/// Creating Entries

/**
 * Fills in the tree view row's columns with the details of
 * the entry 'ent'.
 *
 * @param row The tree view row.
 * @param ent The entry.
 */
static void create_row(Gtk::TreeRow row, dir_entry &ent);


/// Icons

/**
 * Loads the icons for all entries in @a new_list.
 *
 * @param new_list The list containing the icons to be loaded.
 */
static void load_icons(Glib::RefPtr<Gtk::ListStore> new_list);

/**
 * Loads the icon for the entry at row @a row, and stores the
 * entry in the icon column of the row.
 *
 * @param row The row.
 */
static void load_icon(Gtk::TreeRow row);


//// Initialization

std::shared_ptr<file_list_controller> file_list_controller::create() {
    struct enable_make_shared : public file_list_controller {
        using file_list_controller::file_list_controller;
    };

    auto flist = std::make_shared<enable_make_shared>();

    flist->init_vfs();
    return flist;
}

file_list_controller::file_list_controller() {
    cur_list = create_model();
    empty_list = create_model();
}


Glib::RefPtr<Gtk::ListStore> file_list_controller::create_model() {
    auto list_store = make_liststore();

    init_liststore(list_store);

    return list_store;
}

Glib::RefPtr<Gtk::ListStore> file_list_controller::make_liststore() {
    auto list_store = Gtk::ListStore::create(file_model_columns::instance());

    // The operator->() hack is necessary to get a raw pointer to the
    // ListStore object.
    //
    // A raw pointer is necessary, as using the Glib::RefPtr instead,
    // will result in a reference cycle. The list_store object holds a
    // strong reference to itself and will thus never be deallocated.

    list_store->signal_sort_column_changed().connect(sigc::bind(&sort_changed, list_store.operator->()));

    return list_store;
}

void file_list_controller::init_liststore(Glib::RefPtr<Gtk::ListStore> list_store) {
    // Set "Name" as the default sort column
    list_store->set_sort_column(file_model_columns::instance().name, Gtk::SortType::SORT_ASCENDING);

    for (auto &col : file_column_descriptors) {
        list_store->set_sort_func(col->id, col->sort_func());
    }
}


void file_list_controller::init_vfs() {
    using namespace std::placeholders;

    vfs = nuc::vfs::create();

    auto ptr = std::weak_ptr<file_list_controller>(shared_from_this());

    vfs->callback_changed([=] {
        if (auto self = ptr.lock())
            return self->vfs_dir_changed();

        return std::shared_ptr<vfs::delegate>();
    });

    vfs->signal_deleted().connect([=] (paths::pathname path) {
        if (auto self = ptr.lock())
            self->vfs_dir_deleted(path);
    });
}


//// VFS Delegates

/// Read Delegate

file_list_controller::read_delegate::read_delegate(std::weak_ptr<file_list_controller> flist) :
    flist(flist), list(make_liststore()) {
}

void file_list_controller::read_delegate::begin() {}

void file_list_controller::read_delegate::new_entry(dir_entry &ent) {
    Gtk::TreeRow row = *list->append();
    create_row(row, ent);
}

void create_row(Gtk::TreeRow row, dir_entry &ent) {
    auto &columns = file_model_columns::instance();

    row[columns.name] = ent.file_name();
    row[columns.ent] = &ent;
    row[columns.marked] = false;

    ent.context.row = row;
}

void file_list_controller::read_delegate::finish(bool cancelled, int error) {
    if (auto ptr = flist.lock()) {
        if (error || cancelled) {
            ptr->reset_list();
        }
        else {
            ptr->finish_read(list);
        }
    }
}


/// Update Delegate

void file_list_controller::update_delegate::finish(bool cancelled, int error) {
    if (auto ptr = flist.lock()) {
        if (!error && !cancelled) {
            ptr->set_updated_list(list);
        }
    }
}

/// Move Up Delegate

void file_list_controller::move_up_delegate::finish(bool cancelled, int error) {
    if (auto ptr = flist.lock()) {
        if (cancelled || error) {
            ptr->read_parent_dir(path);
        }
        else {
            ptr->finish_read(list);
        }
    }
}


//// VFS Callbacks

std::shared_ptr<vfs::delegate> file_list_controller::vfs_dir_changed() {
    return std::make_shared<update_delegate>(shared_from_this());
}

void file_list_controller::vfs_dir_deleted(paths::pathname new_path) {
    if (!reading)
        read_parent_dir(new_path.empty() ? cur_path : std::move(new_path));
}


void file_list_controller::read_parent_dir(paths::pathname path) {
    using namespace std::placeholders;

    if (!path.is_root()) {
        path = path.remove_last_component();

        auto del = std::make_shared<move_up_delegate>(shared_from_this(), path);

        if (!vfs->ascend(del))
            vfs->read(path, del);
    }
}


//// Setting/Resetting The File List

void file_list_controller::reset_list() {
    m_signal_path.emit(cur_path);

    // Reset to old list
    m_signal_change_model.emit(cur_list);

    // Select previously selected row
    select_row(cur_list->get_path(selected_row)[0]);

    // Reset move to old flag
    move_to_old = false;
}

void file_list_controller::set_updated_list(Glib::RefPtr<Gtk::ListStore> new_list) {
    bool selection = false;
    paths::string name;
    index_type index = 0;

    if (selected_row) {
        // Get name of selected row's entry
        dir_entry *ent = selected_row[file_model_columns::instance().ent];
        name = ent->file_name();

        selection = true;
        index = cur_list->get_path(selected_row)[0];
    }

    set_new_list(new_list, false);

    update_marked_set();

    if (selection)
        select_named(name, index);
}

void file_list_controller::update_marked_set() {
    auto it = marked_set.begin(), end = marked_set.end();

    while (it != end) {
        auto entries = vfs->get_entries(it->first);

        if (std::distance(entries.first, entries.second) != 1) {
            it = marked_set.erase(it);
            continue;
        }
        else {
            mark_row(it->second = entries.first->second.context.row, true);
        }

        ++it;
    }
}


void file_list_controller::add_parent_entry(Glib::RefPtr<Gtk::ListStore> new_list, const paths::pathname &new_path) {
    if (!new_path.is_root())
        create_row(*new_list->append(), parent_entry);
}

void file_list_controller::finish_read(Glib::RefPtr<Gtk::ListStore> new_list) {
    reading = false;

    set_new_list(new_list, true);
    restore_selection();

    cur_path = vfs->path();
    m_signal_path.emit(cur_path);
}

void file_list_controller::set_new_list(Glib::RefPtr<Gtk::ListStore> new_list, bool clear_marked) {
    // Clear marked set
    if (clear_marked)
        marked_set.clear();

    add_parent_entry(new_list, vfs->path());

    load_icons(new_list);

    // Sort new_list using cur_list's sort order
    set_sort_column(new_list);

    // Swap models and switch model to 'new_list'
    cur_list.swap(new_list);

    // Emit 'model_changed' signal with new list
    m_signal_change_model.emit(cur_list);
}

void file_list_controller::set_sort_column(Glib::RefPtr<Gtk::ListStore> new_list) {
    int col_id;
    Gtk::SortType order;

    if (cur_list->get_sort_column_id(col_id, order)) {
        new_list->set_sort_column(col_id, order);
    }
}


//// Sorting

void sort_changed(Gtk::ListStore *list_store) {
    int id;
    Gtk::SortType order;

    list_store->get_sort_column_id(id, order);

    // Reset sort function making sure the order of the invariant sort
    // functions is preserved.
    if (id >= 0 && id < file_column_descriptors.size())
        list_store->set_sort_func(id, file_column_descriptors[id]->sort_func(order));
}


//// Marking Rows

void file_list_controller::mark_row(Gtk::TreeRow row) {
    auto &columns = file_model_columns::instance();

    dir_entry *ent = row[columns.ent];

    if (ent->type() != dir_entry::type_parent) {
        bool mark = !row[columns.marked];
        dir_entry *ent = row[columns.ent];

        if (mark) {
            marked_set.emplace(ent->file_name(), row);
        }
        else {
            marked_set.erase(ent->file_name());
        }

        mark_row(row, mark);
    }
}

void file_list_controller::mark_row(Gtk::TreeRow row, bool marked) {
    auto &columns = file_model_columns::instance();
    row[columns.marked] = marked;
    row[columns.color] = Gdk::RGBA(marked ? "#FF0000" : "#000000");
}


//// Selection

void file_list_controller::select_row(index_type index) {
    m_signal_select.emit(cur_list->children()[index]);
}


void file_list_controller::restore_selection() {
    if (move_to_old) {
        move_to_old = false;
        select_old();
    }
    else {
        select_row(0);
    }
}

void file_list_controller::select_old() {
    select_named(cur_path.basename(), 0);
}

void file_list_controller::select_named(const paths::string &name, index_type row_ind) {
    index_type selection = 0;

    if (cur_list->children().size()) {
        auto rows = cur_list->children();

        auto row = std::find_if(rows.begin(), rows.end(), [&] (const Gtk::TreeRow &row) {
            dir_entry *ent = row[file_model_columns::instance().ent];
            return ent->file_name() == name;
        });

        if (row) {
            selection = cur_list->get_path(row)[0];
        }
        else {
            selection = std::min(cur_list->children().size() - 1, row_ind);
        }

        select_row(selection);
    }
}

void file_list_controller::on_selection_changed(Gtk::TreeRow row) {
    if (!reading) {
        if (mark_rows) {
            index_type prev_index = cur_list->get_path(selected_row)[0];
            index_type row_index = cur_list->get_path(row)[0];

            size_t start, end;

            if (row_index > prev_index) {
                start = prev_index;
                end = row_index - mark_end_offset;
            }
            else {
                start = row_index + mark_end_offset;
                end = prev_index;
            }

            for (size_t i = start; i <= end; i++) {
                mark_row(cur_list->children()[i]);
            }

            mark_rows = false;
        }

        selected_row = row;
    }
}


//// Keypress event handlers

bool file_list_controller::on_keypress(const GdkEventKey *e) {
    switch (e->keyval) {
        case GDK_KEY_Up:
        case GDK_KEY_Down:
            return keypress_arrow(e);

        case GDK_KEY_Home:
        case GDK_KEY_End:
            keypress_change_selection(e, true);
            break;

        case GDK_KEY_Page_Down:
        case GDK_KEY_Page_Up:
            keypress_change_selection(e, false);
            break;
    }

    return false;
}

bool file_list_controller::keypress_arrow(const GdkEventKey *e) {
    if (e->state & GDK_SHIFT_MASK) {
        if (selected_row) {
            mark_row(selected_row);
        }
    }
    return false;
}

void file_list_controller::keypress_change_selection(const GdkEventKey *e, bool mark_sel) {
    if (e->state & GDK_SHIFT_MASK) {
        mark_rows = true;
        mark_end_offset = mark_sel ? 0 : 1;
    }
}


//// Icons

void load_icons(Glib::RefPtr<Gtk::ListStore> new_list) {
    using namespace std::placeholders;

    auto children = new_list->children();

    std::for_each(children.begin(), children.end(), std::bind(&load_icon, _1));
}

void load_icon(Gtk::TreeRow row) {
    auto &columns = file_model_columns::instance();
    dir_entry &ent = *row[columns.ent];

    row[columns.icon] = icon_loader::instance().load_icon(ent);
}


//// Beginning Read Operations

void file_list_controller::prepare_read(bool move_to_old) {
    this->move_to_old = move_to_old;
    reading = true;

    clear_view();
}

paths::pathname file_list_controller::expand_path(const paths::pathname &path) {
    return paths::pathname(cur_path, true).merge(path);
}

void file_list_controller::clear_view() {
    // Set model to empty list to display an empty tree view without
    // discarding the old list
    m_signal_change_model.emit(empty_list);
}

void file_list_controller::path(const paths::pathname &path, bool move_to_old) {
    prepare_read(move_to_old);

    paths::pathname cpath(expand_path(path));
    m_signal_path.emit(cpath);

    vfs->read(cpath, std::make_shared<read_delegate>(shared_from_this()));
}

bool file_list_controller::descend(const dir_entry& ent) {
    if (ent.ent_type() == dir_entry::type_parent) {
        paths::string new_path(cur_path.remove_last_component());
        auto del = std::make_shared<read_delegate>(shared_from_this());

        m_signal_path.emit(new_path);
        prepare_read(true);

        if (!vfs->ascend(del)) {
            vfs->read(new_path, del);
        }

        return true;
    }
    else {
        paths::string new_path(cur_path.append(ent.file_name()));

        if (vfs->descend(ent, std::make_shared<read_delegate>(shared_from_this()))) {
            prepare_read(false);
            m_signal_path.emit(new_path);

            return true;
        }

        return false;
    }
}


//// Getting Selected and Marked Entries

std::vector<dir_entry*> file_list_controller::selected_entries() {
    auto &columns = file_model_columns::instance();

    std::vector<dir_entry*> entries;

    if (marked_set.size()) {
        for (auto &row : marked_set) {
            entries.push_back(row.second[columns.ent]);
        }
    }
    else {
        if (selected_row) {
            dir_entry *ent = selected_row[columns.ent];

            if (ent->ent_type() != dir_entry::type_parent)
                entries.push_back(ent);
        }
    }

    return entries;
}
