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
         * The path prior to initiating initiating a read operation
         * for the new path.
         *
         * Used to reset the path if the operation fails or is cancelled.
         */
        std::string old_path;
        /**
         * The current path.
         */
        std::string cur_path;
        
        /**
         * VFS - responsible for reading the directory.
         */
        nuc::vfs vfs;
        
        /**
         * Flag for whether there is an ongoing read operation.
         */
        bool reading = false;
        
        /**
         * Flag: If true the selection is moved to the entry
         * corresponding to the previous directory, after reading the
         * new directory list.
         */
        bool move_to_old = false;
        
        /**
         * Parent directory pseudo entry.
         */
        dir_entry parent_entry{"..", DT_PARENT};
        
        
        /** Widgets */
        
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
        

        /** Tree View Model */
        
        /**
         * List store, storing the rows (entries) of the tree view
         * widget.
         */
        Glib::RefPtr<Gtk::ListStore> list_store;
        
        /**
         * Empty list store.
         * 
         * The list store model is used to display an empty tree view,
         * while reading the new directory list, without discarding
         * the old list.
         * 
         * When a directory read operation is initiated, the model is
         * switched to this model, when the operation completes
         * successfully the 'list_store' model is cleared, the new
         * entries are added to the model and the tree view's model is
         * switched to it. If the operation fails or is cancelled the
         * model is simply switched to 'list_store' to redisplay the
         * old list
         */
        Glib::RefPtr<Gtk::ListStore> empty_list;
        
        /**
         * Tree view column model.
         */
        file_model_columns columns;

        
        /** Tree view state */
        
        /**
         * The previously selected row, prior to beginning the read
         * operation.
         */
        size_t selected_row = 0;
        
        
        
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
         * Creates a list store model, with the column model record
         * 'columns'.
         * 
         * Returns the list store model.
         */
        Glib::RefPtr<Gtk::ListStore> create_model();
        
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

        
        /** Callbacks */
        
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
         * Restores the old file list and path.
         */
        void reset_list();

        /**
         * Replaces the file list with the new list, read in the
         * last completed operation.
         */
        void new_list();
        
        /**
         * Adds the new rows, read, to the file list store.
         */
        void add_rows();
        /**
         * Adds a single row (entry) to the file list store.
         */
        void add_row(dir_entry &ent);

        
        /** Selection */
        
        /**
         * Returns the index of the currently selected row.
         */
        size_t selected_row_index() const;
        
        /**
         * Selects the row at index 'row'.
         */
        void select_row(size_t row);
        
        /**
         * Moves the selection to the entry corresponding to the
         * previous directory.
         */
        void select_old();
        
        
        /** Signal Handlers */
        
        /**
         * Path entry "activate" signal handler. Called when the text
         * in the path entry is changed and the enter key is pressed.
         */
        void on_path_entry_activate();
        
        /**
         * Treeview keypress event handler.
         */
        bool on_keypress(GdkEventKey *e);
        
        /**
         * Signal handler for the row activate signal of the tree
         * view.  The signal is emitted when a row is "double clicked"
         * or the enter key is pressed while there is a selected row.
         */
        void on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);
        
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
