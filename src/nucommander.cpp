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

#include "nucommander.h"

#include "app_window.h"

#include "async_task.h"

#include <sigc++/sigc++.h>

nuc::NuCommander::NuCommander() : Gtk::Application("org.agware.nucommander") {}

Glib::RefPtr<nuc::NuCommander> nuc::NuCommander::create() {
    return Glib::RefPtr<NuCommander>(new NuCommander());
}

nuc::app_window *nuc::NuCommander::create_app_window() {
    auto window = app_window::create();
    
    add_window(*window);
    
    window->signal_hide().connect(sigc::bind<app_window*>(sigc::mem_fun(*this, &NuCommander::on_hide_window), window));
    
    return window;
}

void nuc::NuCommander::on_activate() {
    //TODO: Add exception handling
    
    init_threads();
    
    auto window = create_app_window();
    window->present();
}

void nuc::NuCommander::on_hide_window(app_window *window) {
    window->cleanup([=] {
        delete window;
    });
}
