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
        /* Paths */
        
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


        /* Background operation state */
        
        /**
         * VFS object - performs the actual reading of the directory.
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
        

        /* Tree View Model */
        
        /**
         * List store model. Stores the rows (entries) of the tree
         * view widget.
         */
        Glib::RefPtr<Gtk::ListStore> list_store;
        
        /**
         * Empty list store.
         * 
         * The list store model is used to display an empty tree view,
         * while reading the new directory list, without discarding
         * the old list.
         * 
         * When a directory read operation is initiated, the tree
         * view's model is switched to this model. When the operation
         * completes successfully the 'list_store' model is cleared,
         * the new entries are added to the model and the tree view's
         * model is switched back to it. If the operation fails or is
         * cancelled the model is simply switched to 'list_store' to
         * redisplay the old list
         */
        Glib::RefPtr<Gtk::ListStore> empty_list;
        
        /**
         * Tree view column model.
         */
        file_model_columns columns;

        
        /* Tree view state */
        
        /**
         * The previously selected row, prior to beginning the read
         * operation.
         */
        size_t selected_row = 0;
        
        /**
         * True if the rows between the previous selection and current
         * selection should be marked. Used to mark a range of rows in
         * response to the Home/End/Page Up/Down, with the shift
         * modifier, key press.
         */
        bool mark_rows = false;
        
        /**
         * The offset, from the current selected row, of the last row
         * to mark. If 0, the current selected row is marked. Used in
         * conjunction with 'mark_rows'.
         */
        int mark_end_offset = 0;

        /**
         * Set of marked rows.
         *
         * The set is represented as a map where the key is the name
         * of the entry and the value is the row in tree view
         * corresponding to the entry.
         */
        std::unordered_map<std::string, Gtk::TreeRow> marked_set;
        

        /* Private Methods */
        
        /**
         * Initializes the file list tree view widget: sets the model
         * and connects the signal handlers.
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
         * Initializes the VFS: sets the callback and creates the
         * pseudo parent entry.
         */
        void init_vfs();


        /* Setting the path */
        
        /**
         * Begins reading the directory at 'path'.
         */
        void read_path(const std::string &path);
        /**
         * Sets the contents of the path text entry widget, to 'path'.
         */
        void entry_path(const std::string &path);

        
        /* Callbacks */
        
        /**
         * VFS callback method.
         */
        void vfs_callback(nuc::vfs::op_stage stage);
        /**
         * Called when the vfs callback is called at the BEGIN stage.
         */
        void begin_read();
        /**
         * Called when the vfs callback is called at the FINISH or
         * CANCELLED stage.
         */
        void finish_read();


        /* Adding/removing rows from the list store */
        
        /**
         * Restores the old file list and path.
         */
        void reset_list();

        /**
         * Replaces the file list with the new list, read in the last
         * completed operation.
         */
        void get_new_list();
        
        /**
         * Adds the new rows to the file list store.
         */
        void add_rows();
        /**
         * Adds a single row (entry) to the file list store.
         */
        void add_row(dir_entry &ent);

        
        /* Selection */
        
        /**
         * Returns the index of the current selected row.
         */
        size_t selected_row_index() const;
        
        /**
         * Selects the row at index 'row'.
         */
        void select_row(size_t row);
        
        /**
         * Moves the selection to the entry corresponding to the
         * previous directory (the entry with the same name as the
         * basename of 'old_path').
         */
        void select_old();

        
        /* Marking */

        /**
         * Marks the row at index 'row'.
         */
        void mark_row(Gtk::TreeRow row);
        
        
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
         * Signal handler for the selection changed event.
         *
         * If the selection was changed using the Home/End/Page
         * Up/Down keys, while the shift was held down ('mark_rows' is
         * true), marks all rows between the previous selected row
         * ('selected_row'), and the current selected row (minus
         * 'mark_end_offset' rows the selected row).
         */
        void on_selection_changed();
        
        /**
         * Treeview keypress event handler.
         */
        bool on_keypress(const GdkEventKey *e);
        

        
        /** Keypress handlers */

        /**
         * Handler for the enter/return key press event.
         */
        bool keypress_return();
        /**
         * Handler for the escape key press event.
         */
        bool keypress_escape();

        /**
         * Handler for the up/down arrow key press event.
         * 
         * e: The keyboard event.
         */
        bool keypress_arrow(const GdkEventKey *e);
      
        /**
         * Handler for a keypress which changes the current selection:
         * Home/End/Page Up/Down.
         * 
         * Sets 'mark_rows' to true in order for all rows between the
         * current selection and the new selection to be marked (when
         * the selection changed signal is emitted), if the shift
         * modifier key was held down.
         * 
         * e:        The keyboard event.
         *
         * mark_sel: True if the new selected row should also be
         *           marked, false if the last row to be marked is the
         *           row before the selected row.
         */
        void keypress_change_selection(const GdkEventKey *e, bool mark_sel);
        
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
