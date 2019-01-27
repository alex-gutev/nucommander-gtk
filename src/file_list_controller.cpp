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

using namespace nuc;

file_list_controller::file_list_controller() {
    init_vfs();
}


//// Initialization

void file_list_controller::tree_view(Gtk::TreeView *view) {
    this->view = view;
    init_file_list();
}

void file_list_controller::init_file_list() {
    init_model();

    // Connect tree view signal handlers

    view->get_selection()->signal_changed().connect(sigc::mem_fun(this, &file_list_controller::on_selection_changed));

    // Connect X event signal handlers

    view->add_events(Gdk::KEY_PRESS_MASK);
    view->signal_key_press_event().connect(sigc::mem_fun(this, &file_list_controller::on_keypress), false);
}

void file_list_controller::init_model() {
    auto &columns = file_model_columns::instance();

    cur_list = create_model();
    new_list = create_model();
    empty_list = create_model();

    view->set_model(cur_list);

    // Icon and Text

    auto *column = create_column("Name");

    column->pack_start(columns.icon, false);
    add_text_cell(column, columns.name);

    column->set_expand(true);

    // Size

    auto *size = create_column("Size");
    auto *cell = add_text_cell(size);

    size->set_cell_data_func(*cell, sigc::ptr_fun(on_size_data));
    size->set_expand(false);
    size->set_sort_column(column_size);

    // Date Modified

    auto *date = create_column("Date Modified");
    auto *date_cell = add_text_cell(date);

    date->set_cell_data_func(*date_cell, sigc::ptr_fun(on_date_data));
    date->set_expand(false);
    date->set_sort_column(column_date);
}

Glib::RefPtr<Gtk::ListStore> file_list_controller::create_model() {
    auto list_store = Gtk::ListStore::create(file_model_columns::instance());

    // Set "Name" as the default sort column
    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);

    list_store->set_sort_func(column_name, make_sort_func(column_name));
    list_store->set_sort_func(column_size, make_sort_func(column_size));
    list_store->set_sort_func(column_date, make_sort_func(column_date));

    // The operator->() hack is necessary to get a raw pointer to the
    // ListStore object.
    //
    // A raw pointer is necessary, as using the Glib::RefPtr instead,
    // will result in a reference cycle. The list_store object holds a
    // strong reference to itself and will thus never be deallocated.

    list_store->signal_sort_column_changed().connect(sigc::bind(&file_list_controller::sort_changed, list_store.operator->()));

    return list_store;
}

Gtk::TreeView::Column *file_list_controller::create_column(const Glib::ustring &title) {
    Gtk::TreeView::Column *col = Gtk::manage(new Gtk::TreeView::Column(title));

    col->set_resizable();
    view->append_column(*col);

    return col;
}

Gtk::CellRendererText * file_list_controller::add_text_cell(Gtk::TreeView::Column *col, Gtk::TreeModelColumn<Glib::ustring> data) {
    auto cell = add_text_cell(col);

    col->add_attribute(cell->property_text(), data);
    col->set_sort_column(data);

    return cell;
}

Gtk::CellRendererText * file_list_controller::add_text_cell(Gtk::TreeView::Column *col) {
    auto cell = Gtk::manage(new Gtk::CellRendererText());

    col->pack_start(*cell);
    col->add_attribute(cell->property_foreground_rgba(), file_model_columns::instance().color);

    return cell;
}


/// VFS Initialization

void file_list_controller::init_vfs() {
    using namespace std::placeholders;

    vfs = nuc::vfs::create();

    vfs->callback_begin(std::bind(&file_list_controller::vfs_begin, this, _1));
    vfs->callback_new_entry(std::bind(&file_list_controller::vfs_new_entry, this, _1, _2));

    vfs->callback_changed(std::bind(&file_list_controller::vfs_dir_changed, this));

    vfs->signal_deleted().connect(sigc::mem_fun(this, &file_list_controller::vfs_dir_deleted));
}


void file_list_controller::vfs_begin(bool refresh) {
    // Disable sorting while adding new entries to improve performance.
    new_list->set_sort_column(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, Gtk::SortType::SORT_ASCENDING);
}

void file_list_controller::vfs_new_entry(dir_entry &ent, bool refresh) {
    Gtk::TreeRow row = *new_list->append();
    create_row(row, ent);
}

void file_list_controller::vfs_finish(bool cancelled, int error, bool refresh) {
    if (!error && !cancelled) {
        if (refresh) {
            set_updated_list();
        }
        else {
            finish_read();
        }
    }
    else {
        reset_list(refresh);
    }
}

void file_list_controller::vfs_finish_move_up(paths::pathname path, bool cancelled, int error, bool refresh) {
    if (!cancelled && !error) {
        finish_read();
    }
    else {
        read_parent_dir(std::move(path));
    }
}

vfs::finish_fn file_list_controller::vfs_dir_changed() {
    return read_finish_callback();
}

void file_list_controller::vfs_dir_deleted(paths::pathname new_path) {
    // TODO: Check that a read task has not been initiated
    if (!reading)
        read_parent_dir(new_path.empty() ? cur_path : std::move(new_path));
}


void file_list_controller::read_parent_dir(paths::pathname path) {
    using namespace std::placeholders;

    if (!path.is_root()) {
        path = path.remove_last_component();

        vfs::finish_fn finish = std::bind(&file_list_controller::vfs_finish_move_up, this, path, _1, _2, _3);

        if (!vfs->ascend(finish))
            vfs->read(path, finish);
    }
}


void file_list_controller::reset_list(bool refresh) {
    // Clear new list
    new_list->clear();

    if (!refresh) {
        m_signal_path.emit(cur_path);

        // Reset to old list
        view->set_model(cur_list);

        // Select previously selected row
        select_row(selected_row);

        // Reset move to old flag
        move_to_old = false;
    }
}

void file_list_controller::set_updated_list() {
    auto row = *view->get_selection()->get_selected();

    bool selection = false;
    paths::string name;
    index_type index;

    if (row) {
        // Get name of selected row's entry
        dir_entry *ent = row[file_model_columns::instance().ent];
        name = ent->file_name();

        // Get index of selected row
        index = view->get_model()->get_path(row)[0];

        selection = true;
    }

    set_new_list(false);

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


void file_list_controller::add_parent_entry(const paths::pathname &new_path) {
    if (!new_path.is_root())
        create_row(*new_list->append(), parent_entry);
}

void file_list_controller::finish_read() {
    reading = false;

    set_new_list(true);
    restore_selection();

    cur_path = vfs->path();
    m_signal_path.emit(cur_path);
}

void file_list_controller::set_new_list(bool clear_marked) {
    vfs->commit_read();

    // Clear marked set
    if (clear_marked)
        marked_set.clear();

    add_parent_entry(vfs->path());

    load_icons();

    // Sort new_list using cur_list's sort order
    set_sort_column();

    // Swap models and switch model to 'new_list'
    cur_list.swap(new_list);
    view->set_model(cur_list);

    // Clear old list
    new_list->clear();
}

void file_list_controller::set_sort_column() {
    int col_id;
    Gtk::SortType order;

    if (cur_list->get_sort_column_id(col_id, order)) {
        new_list->set_sort_column(col_id, order);
    }
}

void file_list_controller::create_row(Gtk::TreeRow row, dir_entry &ent) {
    auto &columns = file_model_columns::instance();

    row[columns.name] = ent.file_name();
    row[columns.ent] = &ent;
    row[columns.marked] = false;

    ent.context.row = row;
}


//// Sorting

void file_list_controller::sort_changed(Gtk::ListStore *list_store) {
    int id;
    Gtk::SortType order;

    list_store->get_sort_column_id(id, order);

    // Reset sort function making sure the order of the invariant sort
    // functions is preserved.
    if (id >= 0 && id < column_last)
        list_store->set_sort_func(id, make_sort_func(id, order));
}

Gtk::TreeSortable::SlotCompare file_list_controller::make_sort_func(int id, Gtk::SortType order) {
    switch (id) {
    case column_size:
        return combine_sort(make_invariant_sort(sort_entry_type, order), sort_size, make_invariant_sort(sort_name, order));

    case column_date:
        return combine_sort(make_invariant_sort(sort_entry_type, order), sort_mtime, make_invariant_sort(sort_name, order));

    case column_name:
    default:
        return combine_sort(make_invariant_sort(sort_entry_type, order), sort_name);
    }
}


/// Column Formatting

void file_list_controller::on_size_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
    Gtk::CellRendererText &text_cell = dynamic_cast<Gtk::CellRendererText&>(*cell);

    auto row = *iter;
    dir_entry *ent = row[file_model_columns::instance().ent];

    switch (ent->type()) {
    case dir_entry::type_reg: {
        const char *unit = "";

        size_t size = ent->attr().st_size;
        float rem = 0;

        if (size >= 1073741824) {
            unit = "GB";

            rem = (size % 1073741824) / 1073741824.0;
            size /= 1073741824;
        }
        else if (size >= 1048576) {
            unit = "MB";

            rem = (size % 1048576) / 1048576.0;
            size /= 1048576;
        }
        else if (size >= 1024) {
            unit = "KB";

            rem = (size % 1024) / 1024.0;
            size /= 1024;
        }

        if (rem) {
            text_cell.property_text().set_value(Glib::ustring::compose("%1.%2 %3", size, (int)roundf(rem * 10), unit));
        }
        else {
            text_cell.property_text().set_value(Glib::ustring::compose("%1 %2", size, unit));
        }
    } break;

    case dir_entry::type_dir:
        text_cell.property_text().set_value("<DIR>");
        break;

    default:
        text_cell.property_text().set_value("");
    }
}

void file_list_controller::on_date_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
    Gtk::CellRendererText &text_cell = dynamic_cast<Gtk::CellRendererText&>(*cell);

    auto row = *iter;
    dir_entry *ent = row[file_model_columns::instance().ent];

    if (ent->ent_type() != dir_entry::type_parent) {
        // localtime is NOT THREAD SAFE
        auto tm = localtime(&ent->attr().st_mtim.tv_sec);

        const size_t buf_size = 17;
        char buf[buf_size]  = {0};

        strftime(buf, buf_size, "%d/%m/%Y %H:%M", tm);
        text_cell.property_text().set_value(buf);
    }
    else {
        text_cell.property_text().set_value("");
    }
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

size_t file_list_controller::selected_row_index() const {
    auto row = view->get_selection()->get_selected();
    if (row) {
        auto path = view->get_model()->get_path(row);
        return path[0];
    }

    return 0;
}

void file_list_controller::select_row(size_t index) {
    auto row = cur_list->children()[index];

    if (row) {
        view->get_selection()->select(row);
        view->scroll_to_row(cur_list->get_path(row));
    }
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

    if (view->get_model()->children().size()) {
        auto rows = cur_list->children();
        auto row = std::find_if(rows.begin(), rows.end(), [&] (const Gtk::TreeRow &row) {
            dir_entry *ent = row[file_model_columns::instance().ent];
            return ent->file_name() == name;
        });

        if (row) {
            selection = view->get_model()->get_path(row)[0];
        }
        else {
            selection = std::min(view->get_model()->children().size() - 1, row_ind);
        }

        select_row(selection);
    }
}


void file_list_controller::on_selection_changed() {
    if (mark_rows) {
        size_t selection = selected_row_index();
        size_t start, end;

        if (selection > selected_row) {
            start = selected_row;
            end = selection - mark_end_offset;
        }
        else {
            start = selection + mark_end_offset;
            end = selected_row;
        }

        for (size_t i = start; i <= end; i++) {
            mark_row(cur_list->children()[i]);
        }

        mark_rows = false;
    }
}


// Keypress event handlers

bool file_list_controller::on_keypress(const GdkEventKey *e) {
    switch (e->keyval) {
        case GDK_KEY_Escape:
            return keypress_escape();

        case GDK_KEY_Return:
            return keypress_return();

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


bool file_list_controller::keypress_return() {
    auto row = *view->get_selection()->get_selected();

    if (row) {
        // Emit activate signal. This should be emitted automatically
        // by the widget however the signal is not emitted if the
        // selection was changed programmatically.
        view->row_activated(cur_list->get_path(row), *view->get_column(0));
        return true;
    }

    return false;
}

bool file_list_controller::keypress_escape() {
    return vfs->cancel();
}

bool file_list_controller::keypress_arrow(const GdkEventKey *e) {
    if (e->state & GDK_SHIFT_MASK) {
        auto row = view->get_selection()->get_selected();
        if (row) {
            mark_row(*row);
        }
    }
    return false;
}

void file_list_controller::keypress_change_selection(const GdkEventKey *e, bool mark_sel) {
    if (e->state & GDK_SHIFT_MASK) {
        selected_row = selected_row_index();
        mark_rows = true;
        mark_end_offset = mark_sel ? 0 : 1;
    }
}


/// Icons

void file_list_controller::load_icons() {
    using namespace std::placeholders;

    auto children = new_list->children();

    std::for_each(children.begin(), children.end(), std::bind(&file_list_controller::load_icon, this, _1));
}

void file_list_controller::load_icon(Gtk::TreeRow row) {
    auto &columns = file_model_columns::instance();

    dir_entry &ent = *row[columns.ent];

    row[columns.icon] = icon_loader::instance().load_icon(ent);
}


// Changing the path

void file_list_controller::prepare_read(bool move_to_old) {
    selected_row = selected_row_index();
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
    view->set_model(empty_list);
}

vfs::finish_fn file_list_controller::read_finish_callback() {
    using namespace std::placeholders;

    return std::bind(&file_list_controller::vfs_finish, this, _1, _2, _3);
}

void file_list_controller::path(const paths::pathname &path, bool move_to_old) {
    prepare_read(move_to_old);

    paths::pathname cpath(expand_path(path));
    m_signal_path.emit(cpath);

    vfs->read(cpath, read_finish_callback());
}

bool file_list_controller::descend(const dir_entry& ent) {
    if (ent.ent_type() == dir_entry::type_parent) {
        paths::string new_path(cur_path.remove_last_component());
        auto finish = read_finish_callback();

        m_signal_path.emit(new_path);
        prepare_read(true);

        if (!vfs->ascend(finish)) {
            vfs->read(new_path, finish);
        }

        return true;
    }
    else {
        paths::string new_path(cur_path.append(ent.file_name()));

        if (vfs->descend(ent, read_finish_callback())) {
            prepare_read(false);
            m_signal_path.emit(new_path);

            return true;
        }

        return false;
    }
}


//// Getting a tree lister

tree_lister * file_list_controller::get_tree_lister() {
    return vfs->get_tree_lister(selected_entries());
}

task_queue::task_type file_list_controller::make_copy_task(const paths::pathname &dest) {
    auto entries = selected_entries();

    if (entries.size())
        return ::make_copy_task(vfs->directory_type(), std::move(entries), expand_path(dest));
    else
        return task_queue::task_type();
}

std::vector<dir_entry*> file_list_controller::selected_entries() {
    auto &columns = file_model_columns::instance();

    std::vector<dir_entry*> entries;

    if (marked_set.size()) {
        for (auto &row : marked_set) {
            entries.push_back(row.second[columns.ent]);
        }
    }
    else {
        auto row = view->get_selection()->get_selected();
        if (row) {
            dir_entry *ent = (*row)[columns.ent];

            if (ent->ent_type() != dir_entry::type_parent)
                entries.push_back(ent);
        }
    }

    return entries;
}
