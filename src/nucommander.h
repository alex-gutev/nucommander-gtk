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

#ifndef NUCOMMANDER_H
#define NUCOMMANDER_H

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/aboutdialog.h>
#include <glibmm.h>

namespace nuc {
    class app_window;

    /**
     * Application class.
     */
    class NuCommander : public Gtk::Application {
    public:
        /**
         * Returns the singleton instance.
         */
        static Glib::RefPtr<NuCommander> instance();

        /**
         * Creates a new NuCommander application object.
         */
        static Glib::RefPtr<NuCommander> create();

        /**
         * Quits the application.
         */
        void quit();

        /**
         * Shows the about dialog.
         */
        void show_about();

        /**
         * Opens the preferences dialog.
         */
        static void preferences();

    protected:
        NuCommander();

        /**
         * Activate signal handler.
         */
        void on_activate() override;

        /**
         * Startup signal handler.
         */
        void on_startup() override;

    private:
        /**
         * The about dialog
         */
        std::unique_ptr<Gtk::AboutDialog> about;

        /**
         * Adds the menu item actions.
         */
        void add_actions();
        /**
         * Sets the application menu.
         */
        void set_menu();

        /**
         * Creates a new application window.
         */
        app_window *create_app_window();

        /**
         * Window hide signal handler.
         *
         * @param window The window.
         */
        void on_hide_window(app_window* window);
    };
}

#endif // NUCOMMANDER_H

// Local Variables:
// mode: c++
// End:
