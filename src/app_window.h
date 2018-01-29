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

#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <glibmm.h>

#include "file_view.h"

namespace nuc {
    /**
     * Main Application Window.
     *
     * Creates the file view widgets and adds them to the paned view
     * container.
     */
    class app_window : public Gtk::ApplicationWindow {
    protected:
        /**
         * The builder object used to created the window widget.
         */
        Glib::RefPtr<Gtk::Builder> builder;

        /**
         * The paned view container widget.
         */
        Gtk::Paned *pane_view;

        /**
         * Left file view widget.
         */
        file_view *left_view;
        /**
         * Right file view widget.
         */
        file_view *right_view;

        /**
         * Creates and adds the file view widgets to the paned view
         * container, and sets up the focus chain.
         */
        void init_pane_view();
        /**
         * Creates a file view widget, stores the pointer to it in
         * 'ptr' and adds to the paned containter, with 'pane'
         * indicating which pane to add it to.
         *
         * ptr:  Reference to where the pointer to the widget is to be
         *       stored.
         *
         * pane: 1 - to add the file view to the left pane.
         *       2 - to add the file view to the right pane.
         */
        void add_file_view(file_view * &ptr, int pane);

        /**
         * Creates a new file view widget builder.
         */
        Glib::RefPtr<Gtk::Builder> file_view_builder();
      
    public:
        /**
         * Constructor.
         */
        app_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new application window object.
         */
        static app_window *create();
    };
}

#endif // APP_WINDOW_H
