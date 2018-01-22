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

nuc::app_window::app_window(Gtk::ApplicationWindow::BaseObjectType* cobject, 
                            const Glib::RefPtr< Gtk::Builder >& builder)
    : Gtk::ApplicationWindow(cobject), builder(builder) {
}

nuc::app_window* nuc::app_window::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/window.glade");
    
    app_window *window = nullptr;
    
    builder->get_widget_derived("commander_window", window);
    
    if (!window)
        throw std::runtime_error("No \"commander_window\" object in window.glade");
    
    return window;
}



