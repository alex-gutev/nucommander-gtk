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

#ifndef NUC_FILE_VIEW_H
#define NUC_FILE_VIEW_H

#include <glibmm.h>

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>

#include <unordered_map>

#include "file_list_controller.h"

namespace nuc {
    /**
     * File view derived widget.
     *
     * Manages the file view widget which contains a text entry, where
     * the current path is displayed and entred, and a tree-view
     * widget where the file-list is displayed.
     *
     * The class also initiates background read operations in response
     * to user events triggered by the child widgets.
     */
    class file_view : public Gtk::Frame {
        /**
         * File list controller. Populates the tree view's model with
         * the directory's contents.
         */
        file_list_controller flist;
        

        /* Widgets */
        
        /**
         * Path text entry widget.
         */
        Gtk::Entry *path_entry;
        /**
         * File list tree view widget.
         */
        Gtk::TreeView *file_list;

        /**
         * Scrolled window widget in which the file list tree view
         * widget is contained.
         */
        Gtk::ScrolledWindow *scroll_window;
        

        /* Private Methods */
        
        /**
         * Initializes the file list tree view widget: sets the model
         * and connects the signal handlers.
         */
        void init_file_list();        
        
        /**
         * Initializes the path text entry. Connects a signal handler,
         * to the activate signal, which begins a background read
         * operation for the new directory.
         */
        void init_path_entry();


        /* Setting the path */
        
        /**
         * Sets the contents of the path text entry widget, to 'path'.
         */
        void entry_path(const std::string &path);
                
        
        /** Signal Handlers */
        
        /**
         * Path entry "activate" signal handler. Called when the text
         * in the path entry is changed and the enter key is pressed.
         *
         * Initiatiates a new background read operation, and returns
         * focus to the tree view.
         */
        void on_path_entry_activate();
        
        /**
         * Signal handler for the row activate signal of the tree
         * view.  The signal is emitted when a row is "double clicked"
         * or the enter key is pressed while a row is selected.
         *
         * If the entry corresponding to the selected row is a
         * directory, changes the path to the directory.
         */
        void on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);

        /**
         * Signal handler for the file list controller's path changed
         * signal.
         */
        void on_path_changed(const std::string &path);

        
    public:
        /**
         * Derived widget constructor.
         */
        file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Changes the current path of the file view, this changes the
         * path displayed and begins a background read operation for
         * the new path.
         */
        void path(const std::string &path, bool move_to_old = false);

        /**
         * Asynchronous cleanup method.
         *
         * @param fn The cleanup function to call once it is safe to
         *           deallocate the object.
         *
         * This method should only be called on the main thread. The
         * function fn will be called on the main thread.
         */
        template <typename F>
        void cleanup(F fn) {
            flist.cleanup(fn);
        }
    };
}

#endif // NUC_FILE_VIEW_H

// Local Variables:
// mode: c++
// End:
