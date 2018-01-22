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
#include <glibmm.h>

namespace nuc {
    class app_window;
    
    class NuCommander : public Gtk::Application {
    protected:
        NuCommander();
    
        void on_activate() override;
        //void on_open(const Gio::Application::type_vec_files &files, const Glib::ustring &hint) override;
        
    private:
        app_window *create_app_window();
        void on_hide_window(Gtk::Window *window);
        
    public:
        static Glib::RefPtr<NuCommander> create();
    };
}

#endif // NUCOMMANDER_H
