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

#include "app_window.h"

#include <exception>

using namespace nuc;

app_window* app_window::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/window.glade");
    
    app_window *window = nullptr;
    
    builder->get_widget_derived("commander_window", window);
    
    if (!window)
        throw std::runtime_error("No \"commander_window\" object in window.glade");
    
    return window;
}

app_window::app_window(Gtk::ApplicationWindow::BaseObjectType* cobject, 
                            const Glib::RefPtr< Gtk::Builder >& builder)
    : Gtk::ApplicationWindow(cobject), builder(builder) {
    // TODO: Add error checking
    
    set_default_size(500, 600);
        
    builder->get_widget("pane_view", pane_view);
    
    add_file_view(left_view, 1);
    add_file_view(right_view, 2);
    
    pane_view->show_all();
    
    left_view->path("/");
    right_view->path("/");
    
    init_pane_view();
}

void app_window::init_pane_view() {
    set_focus_chain({pane_view});
    
    pane_view->set_focus_chain({left_view, right_view});
}


void app_window::add_file_view(nuc::file_view* & ptr, int pane) {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/fileview.glade");
    
    // TODO: Add error checking
    
    builder->get_widget_derived("file_view", ptr);
    
    if (pane == 1) {
        pane_view->pack1(*ptr, true, true);
    }
    else {
        pane_view->pack2(*ptr, true, true);
    }
}


Glib::RefPtr< Gtk::Builder > app_window::file_view_builder() {
    return Gtk::Builder::create_from_resource("/org/agware/nucommander/fileview.glade");
}