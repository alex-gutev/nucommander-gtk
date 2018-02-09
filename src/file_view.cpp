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

#include "file_view.h"

#include "async_task.h"

#include <gdk/gdkkeysyms.h>

#include <algorithm>
#include <functional>

// For debugging only
//#include <iostream>
//#include <stdio.h>

using namespace nuc;


//// Constructor

file_view::file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> & builder)
    : Gtk::Frame(cobject) {
    builder->get_widget("path_entry", path_entry);
    builder->get_widget("file_list", file_list);
    builder->get_widget("scroll_window", scroll_window);
    
    init_vfs();
    init_file_list();
    init_path_entry();

    // Exclude entry widget from tab order focus chain.
    set_focus_chain({file_list});;
}


//// Initialization

void file_view::init_file_list() {
    init_model();
    
    // Create a seperate vertical adjustment object for the tree view
    // in order to disable "smooth scrolling" when navigating using
    // the arrow keys.
        
    auto adj = Gtk::Adjustment::create(0,0,0);
    
    // Signal handlers are added to the change events of both
    // adjustment objects in order to propagate the changes to the
    // other adjustment object. Currently GTK compares the new values
    // set to the previous values and fires change signals only if
    // they are different, thus no flag is needed to determine which
    // adjustment object fired the initial change signal.
    
    adj->signal_changed().connect([this] {
        auto adj = file_list->get_vadjustment();
        scroll_window->get_vadjustment()->configure(
            adj->get_value(),
            adj->get_lower(),
            adj->get_upper(),
            adj->get_step_increment(),
            adj->get_page_increment(),
            adj->get_page_size()
        );
    });
    
    adj->signal_value_changed().connect([this] {
        scroll_window->get_vadjustment()->set_value(file_list->get_vadjustment()->get_value());
    });
    
    file_list->set_vadjustment(adj);

    // Scroll window adjustment signals
    
    scroll_window->get_vadjustment()->signal_value_changed().connect([=] {
        file_list->get_vadjustment()->set_value(scroll_window->get_vadjustment()->get_value());
    });
    
    
    // Add tree view signal handlers
    
    file_list->signal_row_activated().connect(sigc::mem_fun(this, &file_view::on_row_activate));
    file_list->get_selection()->signal_changed().connect(sigc::mem_fun(this, &file_view::on_selection_changed));

    
    // Add X event handlers
    
    file_list->add_events(Gdk::KEY_PRESS_MASK);
    file_list->signal_key_press_event().connect(sigc::mem_fun(this, &file_view::on_keypress), false);
}

void file_view::init_model() {
    cur_list = create_model();
    new_list = create_model();
    empty_list = create_model();
    
    file_list->set_model(cur_list);
    
    // TODO: Move creation of columns and cells into seperate function
    
    auto cell = Gtk::manage(new Gtk::CellRendererText);
    int ncols = file_list->append_column("Name", *cell);
    
    auto column = file_list->get_column(ncols - 1);
    
    column->add_attribute(cell->property_text(), columns.name);
    column->add_attribute(cell->property_foreground_rgba(), columns.color);
    column->set_sort_column(columns.name);
}

Glib::RefPtr<Gtk::ListStore> file_view::create_model() {
    auto list_store = Gtk::ListStore::create(columns);
    
    // Set "Name" as the default sort column
    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);
    
    return list_store;
}

void file_view::init_path_entry() {
    path_entry->signal_activate().connect(sigc::mem_fun(this, &file_view::on_path_entry_activate));
}


/// VFS Initialization

void file_view::init_vfs() {
    using namespace std::placeholders;
    
    vfs.callback_begin(std::bind(&file_view::vfs_begin, this, _1));
    vfs.callback_new_entry(std::bind(&file_view::vfs_new_entry, this, _1));
    vfs.callback_finish(std::bind(&file_view::vfs_finish, this, _1, _2));
    
    parent_entry.subpath("..");
}

void file_view::vfs_begin(bool refresh) {
    if (!is_root_path(cur_path))
        create_row(*new_list->append(), parent_entry);    
}

void file_view::vfs_new_entry(dir_entry &ent) {
    create_row(*new_list->append(), ent);
}

void file_view::vfs_finish(bool cancelled, int error) {
    dispatch_main([=] {
        reading = false;
        vfs.free_op();

        if (!error && !cancelled) {
            set_new_list();
        }
        else {
            reset_list();
        }
    });
}


void file_view::reset_list() {
    path_entry->set_text(old_path);
    std::swap(old_path, cur_path);

    // Clear new list
    new_list->clear();
    
    // Reset to old list
    file_list->set_model(cur_list);

    // Select previously selected row
    select_row(selected_row);
    
    // Reset move to old flag
    move_to_old = false;
}

void file_view::set_new_list() {
    vfs.commit_read();

    // Clear marked set
    marked_set.clear();

    // Swap models and switch model to 'new_list'
    cur_list.swap(new_list);
    file_list->set_model(cur_list);

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


void file_view::create_row(Gtk::TreeRow row, dir_entry &ent) {
    row[columns.name] = ent.file_name();
    row[columns.ent] = &ent;
    row[columns.marked] = false;    
}

void file_view::select_old() {
    int selection = 0;
    std::string old_name(file_name(old_path));
    
    auto rows = cur_list->children();
    auto row = std::find_if(rows.begin(), rows.end(), [this, &old_name] (const Gtk::TreeRow &row) {
        dir_entry *ent = row[columns.ent];
        return ent->file_name() == old_name;
    });
    
    if (row) {
        selection = file_list->get_model()->get_path(row)[0];
    }
    
    select_row(selection);
}


//// Marking Rows

void file_view::mark_row(Gtk::TreeRow row) {
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

size_t file_view::selected_row_index() const {
    auto row = file_list->get_selection()->get_selected();
    if (row) {
        auto path = file_list->get_model()->get_path(row);
        return path[0];
    }

    return 0;
}

void file_view::select_row(size_t index) {
    auto row = cur_list->children()[index];
    
    if (row) {
        file_list->get_selection()->select(row);
        file_list->scroll_to_row(cur_list->get_path(row));
    }
}


void file_view::on_path_entry_activate() {
    read_path(path_entry->get_text());
    file_list->grab_focus();
}

void file_view::on_row_activate(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn* column) {
    auto row = cur_list->children()[row_path[0]];

    dir_entry &ent = *row[columns.ent];

    switch (ent.type()) {
        case DT_PARENT:
            move_to_old = true;
            path(removed_last_component(cur_path));
            break;
            
        case DT_DIR:
            path(appended_component(cur_path, ent.file_name()));
            break;
            
        case DT_REG:
            break;
    }
}

void file_view::on_selection_changed() {
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


bool file_view::on_keypress(const GdkEventKey *e) {
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



bool file_view::keypress_return() {
    auto row = *file_list->get_selection()->get_selected();

    if (row) {
        // Emit activate signal. This should be emitted by
        // automatically by the widget however the signal is not
        // emiited if the selection was changed programmatically.
        file_list->row_activated(cur_list->get_path(row), *file_list->get_column(0));
        return true;
    }

    return false;
}

bool file_view::keypress_escape() {
    if (reading) {
        vfs.cancel();
        return true;
    }

    return false;
}

bool file_view::keypress_arrow(const GdkEventKey *e) {
    if (e->state & GDK_SHIFT_MASK) {
        auto row = file_list->get_selection()->get_selected();
        if (row) {
            mark_row(*row);
        }
    }
    return false;
}

void file_view::keypress_change_selection(const GdkEventKey *e, bool mark_sel) {
    if (e->state & GDK_SHIFT_MASK) {
        selected_row = selected_row_index();
        mark_rows = true;
        mark_end_offset = mark_sel ? 0 : 1;
    }
}



/// Changing the current path

void file_view::path(const std::string &path) {
    entry_path(path);
    read_path(path);
}

void file_view::entry_path(const std::string &path) {
    path_entry->set_text(path);
}

void file_view::read_path(const std::string &path) {
    selected_row = selected_row_index();
    
    // Set model to empty list to display an empty tree view without
    // discarding the old list
    file_list->set_model(empty_list);
    
    std::swap(old_path, cur_path);
    cur_path = path;

    reading = true;
    vfs.read(path);
}
