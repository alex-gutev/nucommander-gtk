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

#ifndef NUC_FILE_LIST_H
#define NUC_FILE_LIST_H

#include <glibmm.h>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include <unordered_map>

#include "file_model_columns.h"
#include "vfs.h"

namespace nuc {
    /**
     * Control's the tree model of the tree view where the file list
     * is displayed.
     *
     * Responsible for intiating background read operations,
     * populating the model with the content's of the directory, and
     * resetting the list when the background operation is cancelled.
     */
    class file_list_controller {
        /**
         * Path changed signal type.
         *
         * Prototype: void(const path_str &path)
         *
         * @param path The new path.
         */
        typedef sigc::signal<void, const path_str &> signal_path_type;

        
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

        /**
         * Path changed signal.
         */
        signal_path_type m_signal_path;
        
        
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
         * Flag: If true the selection should be moved to the entry
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
         * File list tree view widget.
         */
        Gtk::TreeView *view;

        
        /* Tree View Model */
        
        /**
         * Current list store model. Stores the entries currently
         * visible in the tree view widget.
         */
        Glib::RefPtr<Gtk::ListStore> cur_list;
        /**
         * New list store model. Stores the new entires, currently
         * being read. After the operation completes successfully,
         * this list store is set as the tree view's model, 'cur_list'
         * is cleared and 'new_list' and 'cur_list' are swapped.
         */
        Glib::RefPtr<Gtk::ListStore> new_list;
        
        /**
         * Empty list store.
         * 
         * The list store model is used to display an empty tree view,
         * while reading the new directory list, without discarding
         * the old list.
         */
        Glib::RefPtr<Gtk::ListStore> empty_list;
        

        /* Tree view state */
        
        /**
         * The previously selected row, prior to beginning the read
         * operation.
         */
        size_t selected_row = 0;
        
        /**
         * True if the rows between the previous selection and current
         * selection should be marked. Used to mark a range of rows in
         * response to the Home/End/Page Up/Down key press event.
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
         * of the entry and the value is the tree view row
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
         * Initializes the VFS, sets the operation callbacks.
         */
        void init_vfs();


        /* Callbacks */

        /**
         * VFS begin callback.
         */
        void vfs_begin(bool refresh);
        /**
         * VFS new entry callback.
         */
        void vfs_new_entry(dir_entry &entry);
        /**
         * VFS finish callback.
         */
        void vfs_finish(bool cancelled, int error);


        /* Resetting/Setting the treeview model */
        
        /**
         * Restores the old file list and path.
         */
        void reset_list();

        /**
         * Switches the tree view's model to 'new_list', clears
         * 'cur_list' and swaps the two models.
         */
        void set_new_list();

        /**
         * Fills in the tree view row's columns with the details of
         * the entry 'ent'.
         */
        void create_row(Gtk::TreeRow row, dir_entry &ent);
        
        /**
         * Sets the sort column (and sort order) of 'new_list' to be
         * the same as 'old_list'.
         */
        void set_sort_column();
        

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
         * Signal handler for the selection changed event.
         *
         * If the selection was changed using the Home/End/Page
         * Up/Down keys, while the shift was held down ('mark_rows' is
         * true), marks all rows between the previous selected row
         * ('selected_row'), and the current selected row (minus
         * 'mark_end_offset' rows from the selected row).
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
         * Tree view column model.
         */
        file_model_columns columns;


        /** Constructor */
        file_list_controller();

        /**
         * Sets the tree view, which is to be controlled by the file
         * list controller.
         *
         * Creates and sets the tree view's model.
         */
        void tree_view(Gtk::TreeView *view);

        
        /**
         * Changes the current path of the file view, this changes the
         * path displayed and begins a background read operation for
         * the new path.
         */
        void path(const std::string &path, bool move_to_old = false);

        /**
         * Returns the current path.
         */
        const std::string &path() const {
            return cur_path;
        }

        /**
         * Path changed signal.
         *
         * Emitted when a background operation fails, or is cancelled,
         * and the path is reset to the previous path.
         */
        signal_path_type signal_path() {
            return m_signal_path;
        }

        /**
         * Returns the current list store model.
         */
        Glib::RefPtr<Gtk::ListStore> list() {
            return cur_list;
        }
    };
}

#endif // NUC_FILE_LIST_H

// Local Variables:
// mode: c++
// End:
