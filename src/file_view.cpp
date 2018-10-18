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

#include "tasks/async_task.h"

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
    
    init_file_list();
    init_path_entry();

    // Exclude entry widget from tab order focus chain.
    set_focus_chain({file_list});
}


//// Initialization

void file_view::init_file_list() {
    // Associate file list controller with tree view widget
    flist.tree_view(file_list);

    flist.signal_path().connect(sigc::mem_fun(*this, &file_view::on_path_changed));
    
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
}

void file_view::init_path_entry() {
    path_entry->signal_activate().connect(sigc::mem_fun(this, &file_view::on_path_entry_activate));
}



//// Signal Handlers

void file_view::on_path_entry_activate() {
    flist.path(path_entry->get_text());
    file_list->grab_focus();
}

void file_view::on_row_activate(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn* column) {
    auto row = flist.list()->children()[row_path[0]];

    dir_entry &ent = *row[flist.columns.ent];

    flist.descend(ent);
}

void file_view::on_path_changed(const paths::string &path) {
    entry_path(path);
}



//// Changing the current path

void file_view::path(const std::string &path, bool move_to_old) {
    entry_path(path);
    flist.path(path, move_to_old);
}

void file_view::entry_path(const std::string &path) {
    path_entry->set_text(path);
}


//// Getting a tree lister

tree_lister * file_view::get_tree_lister() {
    return flist.get_tree_lister();
}

dir_writer * file_view::get_dir_writer() {
    return dir_type::get_writer(flist.path());
}
