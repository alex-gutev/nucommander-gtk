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
#include <gtkmm/liststore.h>

#include "file_model_columns.h"
#include "vfs.h"

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
         * Path text entry widget.
         */
        Gtk::Entry *path_entry;
        /**
         * File list tree view widget.
         */
        Gtk::TreeView *file_list;

        /**
         * List store, storing the rows (entries) of the tree view
         * widget.
         */
        Glib::RefPtr<Gtk::ListStore> list_store;
        /**
         * Tree view column model.
         */
        file_model_columns columns;

        /**
         * VFS - responsible for reading the directory.
         */
        nuc::vfs vfs;

        
        /**
         * Initializes the file list tree view widget: sets the tree
         * view's model.
         */
        void init_file_list();
        /**
         * Initializes the tree view's model, and adds the columns to
         * the tree view.
         */
        void init_model();

        /**
         * Initializes the path text entry. Connects a signal handler,
         * to the activate signal, which begins a background read
         * operation for the new directory.
         */
        void init_path_entry();

        /**
         * Initializes the VFS callback.
         */
        void init_vfs();

        
        /**
         * Begins reading the directory at 'path'.
         */
        void read_path(const std::string &path);
        /**
         * Sets the contents of the path text entry widget, to 'path'.
         */
        void entry_path(const std::string &path);

        
        /**
         * VFS callback method.
         */
        void vfs_callback(nuc::vfs::op_stage stage);
        /**
         * Called when the vfs callback is called at the BEGIN stage.
         *
         * Removes all rows in the file list store.
         */
        void begin_read();
        /**
         * Called when the vfs callback is called at the FINISH or
         * CANCELLED stage.
         */
        void finish_read();

        
        /**
         * Adds the new rows read to the file list store.
         */
        void add_new_rows();
        /**
         * Adds a single row (entry) to the file list store.
         */
        void add_row(dir_entry &ent);

        /**
         * Path entry "activate" signal handler. Called when the text
         * in the path entry is changed and the entre key is pressed.
         */
        void on_path_entry_activate();
        
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
        void path(const std::string &path);
    };
}

#endif // NUC_FILE_VIEW_H
