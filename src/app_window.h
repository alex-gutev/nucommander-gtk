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

#ifndef WINDOW_H
#define WINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <glibmm.h>

#include "file_view.h"

namespace nuc {
    class app_window : public Gtk::ApplicationWindow {
    protected:
        Glib::RefPtr<Gtk::Builder> builder;
      
        Gtk::Paned *pane_view;
        
        file_view *left_view;
        file_view *right_view;

        void add_file_view(file_view * &ptr, int pane);
        
        Glib::RefPtr<Gtk::Builder> file_view_builder();
      
    public:
        app_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);
		
		static app_window *create();
    };
}

#endif // WINDOW_H

