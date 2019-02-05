/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef INTERFACE_OPEN_DIRS_POPUP_H
#define INTERFACE_OPEN_DIRS_POPUP_H

#include <vector>
#include <functional>
#include <memory>

#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "paths/pathname.h"
#include "file_list/file_list_controller.h"

namespace nuc {
    /**
     * Popup window displaying list of open directories.
     */
    class open_dirs_popup : public Gtk::Window {
        /**
         * Model columns record for the list.
         */
        struct model_columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> path;
            Gtk::TreeModelColumn<file_list_controller *> file_list;

            model_columns();
        };

        /**
         * Directory Chosen Function Type.
         *
         * Prototype void(file_list_controller *flist)
         *
         * @param flist The file_list_controller of the chosen
         *   directory.
         */
        typedef std::function<void(file_list_controller*)> dir_chosen_fn;

        /**
         * Column Model
         */
        model_columns model;

        /**
         * List store containing list of open directories.
         */
        Glib::RefPtr<Gtk::ListStore> dirs_list;

        /**
         * Directory list tree view.
         */
        Gtk::TreeView *dirs_view;

        /**
         * Callback function: called when a directory is chosen.
         */
        dir_chosen_fn m_dir_chosen;

        /**
         * Signal handler for the 'row_activated' signal of the tree
         * view.
         *
         * @param path Path to the activated row.
         * @param column Activated column.
         */
        void dir_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);

        /**
         * Delete event handler.
         *
         * calls gtk_widget_hide_on_delete to prevent the window from
         * being deleted.
         */
        bool on_delete(const GdkEventAny *e);

        /**
         * Key press event handler.
         */
        bool on_key_press_event(GdkEventKey *e);

    public:
        /** constructor */
        open_dirs_popup(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new open directory list popup.
         */
        static open_dirs_popup *create();

        /**
         * Sets the list of open directories displayed in the
         * treeview.
         *
         * Only the file_list_controllers which are not attached to
         * any file_view are displayed in the list.
         *
         * @param dirs Vector of file_list_controller objects for each
         *   open directory.
         */
        void set_dirs(const std::vector<std::unique_ptr<file_list_controller>> &dirs);

        /**
         * Sets the directory chosen callback function.
         *
         * This function is called when a directory is chosen by the
         * user.
         *
         * @param fn The callback function.
         */
        void dir_chosen(dir_chosen_fn fn) {
            m_dir_chosen = std::move(fn);
        }
    };

};

#endif /* INTERFACE_OPEN_DIRS_POPUP_H */

// Local Variables:
// mode: c++
// End:
