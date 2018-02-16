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

#include "async_task.h"


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
    cur_list = create_model();
    new_list = create_model();
    empty_list = create_model();
    
    view->set_model(cur_list);
    
    // TODO: Move creation of columns and cells into seperate function
    
    auto cell = Gtk::manage(new Gtk::CellRendererText);
    int ncols = view->append_column("Name", *cell);
    
    auto column = view->get_column(ncols - 1);
    
    column->add_attribute(cell->property_text(), columns.name);
    column->add_attribute(cell->property_foreground_rgba(), columns.color);
    column->set_sort_column(columns.name);
}

Glib::RefPtr<Gtk::ListStore> file_list_controller::create_model() {
    auto list_store = Gtk::ListStore::create(columns);
    
    // Set "Name" as the default sort column
    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);
    
    return list_store;
}


/// VFS Initialization

void file_list_controller::init_vfs() {
    using namespace std::placeholders;
    
    vfs.callback_begin(std::bind(&file_list_controller::vfs_begin, this, _1));
    vfs.callback_new_entry(std::bind(&file_list_controller::vfs_new_entry, this, _1));
    vfs.callback_finish(std::bind(&file_list_controller::vfs_finish, this, _1, _2));
    
    parent_entry.subpath("..");
}

void file_list_controller::vfs_begin(bool refresh) {
    reading = true;
	
    // Disable sorting while adding new entries to improve performance.
    new_list->set_sort_column(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, Gtk::SortType::SORT_ASCENDING);
    
    if (!is_root_path(cur_path))
        create_row(*new_list->append(), parent_entry);
}

void file_list_controller::vfs_new_entry(dir_entry &ent) {
    create_row(*new_list->append(), ent);
}

void file_list_controller::vfs_finish(bool cancelled, int error) {
    dispatch_main([=] {
        reading = false;

        if (!error && !cancelled) {
            set_new_list();
        }
        else {
            reset_list();
        }
    });
}


void file_list_controller::reset_list() {
	m_signal_path.emit(old_path);
	
    std::swap(old_path, cur_path);

    // Clear new list
    new_list->clear();
    
    // Reset to old list
    view->set_model(cur_list);

    // Select previously selected row
    select_row(selected_row);
    
    // Reset move to old flag
    move_to_old = false;
}

void file_list_controller::set_new_list() {
    vfs.commit_read();

    // Clear marked set
    marked_set.clear();

    // Sort new_list using cur_list's sort order
    set_sort_column();
    
    // Swap models and switch model to 'new_list'
    cur_list.swap(new_list);
    view->set_model(cur_list);

    // Clear old list
    new_list->clear();
    
    // Selection
    if (move_to_old) {
        move_to_old = false;
        select_old();
    }
    else {
        select_row(0);
    }
    
    // Clear previous path
    old_path.clear();
}

void file_list_controller::set_sort_column() {
    int col_id;
    Gtk::SortType order;
    
    if (cur_list->get_sort_column_id(col_id, order)) {
        new_list->set_sort_column(col_id, order);
    }
}


void file_list_controller::create_row(Gtk::TreeRow row, dir_entry &ent) {
    row[columns.name] = ent.file_name();
    row[columns.ent] = &ent;
    row[columns.marked] = false;    
}


void file_list_controller::select_old() {
    int selection = 0;
    std::string old_name(file_name(old_path));
    
    auto rows = cur_list->children();
    auto row = std::find_if(rows.begin(), rows.end(), [this, &old_name] (const Gtk::TreeRow &row) {
        dir_entry *ent = row[columns.ent];
        return ent->file_name() == old_name;
    });
    
    if (row) {
        selection = view->get_model()->get_path(row)[0];
    }
    
    select_row(selection);
}


//// Marking Rows

void file_list_controller::mark_row(Gtk::TreeRow row) {
    dir_entry *ent = row[columns.ent];
    
    if (ent->type() != DT_PARENT) {
        bool marked = row[columns.marked] = !row[columns.marked];

        row[columns.color] = Gdk::RGBA(marked ? "#FF0000" : "#000000");

        if (marked) {
            marked_set.emplace(ent->file_name(), row);
        }
        else {
            marked_set.erase(ent->file_name());
        }
    }
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
        // by the widget however the signal is not emiited if the
        // selection was changed programmatically.
        view->row_activated(cur_list->get_path(row), *view->get_column(0));
        return true;
    }

    return false;
}

bool file_list_controller::keypress_escape() {
    if (reading) {
        vfs.cancel();
        return true;
    }

    return false;
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


// Changing the path

void file_list_controller::path(const std::string &path, bool move_to_old) {
    selected_row = selected_row_index();
    
    // Set model to empty list to display an empty tree view without
    // discarding the old list
    view->set_model(empty_list);
    
    std::swap(old_path, cur_path);
    cur_path = path;

	this->move_to_old = move_to_old;
	
    vfs.read(path);
}
