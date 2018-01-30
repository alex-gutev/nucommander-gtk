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

using namespace nuc;

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
    // adjustment object fired the initial change signal
    
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
    
    adj->signal_value_changed().connect([=] {
        scroll_window->get_vadjustment()->set_value(adj->get_value());
    });
    
    file_list->set_vadjustment(adj);
    
    scroll_window->get_vadjustment()->signal_value_changed().connect([=] {
        file_list->get_vadjustment()->set_value(scroll_window->get_vadjustment()->get_value());
    });
    

    // Add X event handlers
    
    file_list->add_events(Gdk::KEY_PRESS_MASK);
    file_list->signal_key_press_event().connect(sigc::mem_fun(this, &file_view::on_keypress));
}

void file_view::init_model() {
    list_store = create_model();
    empty_list = create_model();
    
    file_list->set_model(list_store);
    file_list->append_column("Name", columns.name);
    
    auto column = file_list->get_column(0);
    column->set_sort_column(columns.name);
}

Glib::RefPtr<Gtk::ListStore> file_view::create_model() {
    auto list_store = Gtk::ListStore::create(columns);
    
    // Set "Name" as the default sort column
    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);
    
    return list_store;
}


void file_view::init_vfs() {
    vfs.callback = [=] (nuc::vfs &, nuc::vfs::op_stage stage) {
        dispatch_main([=] {
            vfs_callback(stage);
        });
    };
    
    parent_entry.subpath("..");
}

void file_view::init_path_entry() {
    path_entry->signal_activate().connect(sigc::mem_fun(this, &file_view::on_path_entry_activate));
}


void file_view::vfs_callback(vfs::op_stage stage) {
    switch (stage) {
        case nuc::vfs::BEGIN:
            begin_read();
            break;
        
        case nuc::vfs::FINISH:
            finish_read();
            
            if (!vfs.status())
                new_list();
            else
                reset_list();
            
            break;
            
        case nuc::vfs::CANCELLED:
            finish_read();
            reset_list();
            break;
            
        case nuc::vfs::ERROR:
            break;
    }
}

void file_view::begin_read() {}

void file_view::finish_read() {
    reading = false;
    vfs.free_op();
}


void file_view::reset_list() {
    path_entry->set_text(old_path);
    std::swap(old_path, cur_path);
    
    // Reset to old list
    file_list->set_model(list_store);

    // Select previously selected row
    select_row(selected_row);
}

void file_view::new_list() {
    old_path.clear();
    
    vfs.commit_read();
    
    // Clear old list
    list_store->clear();
    file_list->set_model(list_store);

    // Add new rows to list
    add_rows();
    select_row(0);
}



void file_view::add_rows() {
    if (!is_root_path(cur_path))
        add_row(parent_entry);
    
    vfs.for_each([=] (dir_entry &ent) {
        add_row(ent);
    });
}

void file_view::add_row(dir_entry &ent) {
    auto row = *list_store->append();
    row[columns.name] = ent.file_name();
}



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

size_t file_view::selected_row_index() const {
    auto row = file_list->get_selection()->get_selected();
    if (row) {
        auto path = file_list->get_model()->get_path(row);
        return path[0];
    }

    return 0;
}

void file_view::select_row(size_t index) {
    auto row = list_store->children()[index];
    
    if (row)
        file_list->get_selection()->select(row);
}


void file_view::on_path_entry_activate() {
    read_path(path_entry->get_text());
    file_list->grab_focus();
}

bool file_view::on_keypress(GdkEventKey *e) {
    if (e->keyval == GDK_KEY_Escape) {
        if (reading) {
            vfs.cancel();
            return true;
        }
    }
        
    return false;
}
