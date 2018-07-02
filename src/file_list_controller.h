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
#include <gtkmm/treerowreference.h>

#include <unordered_map>

#include "file_model_columns.h"
#include "vfs.h"

namespace nuc {
    /**
     * Controls the tree model of the tree view where the file list is
     * displayed.
     *
     * Responsible for intiating background read operations,
     * populating the model with the content's of the directory, and
     * resetting the list when the background operation is cancelled.
     */
    class file_list_controller {
        /**
         * Row index type. Matches the Gtk type.
         */
        typedef Gtk::TreeNodeChildren::size_type index_type;
        
        /**
         * Path changed signal type.
         *
         * Prototype: void(const path_str &path)
         *
         * @param path The new path.
         */
        typedef sigc::signal<void, const paths::string &> signal_path_type;

        /**
         * Marked entries set type.
         *
         * Maps file names to row iterators (Gtk::TreeRow). Row
         * iterators are used, instead of row references, as
         * Gtk::ListStore supports persistent iterators.
         */
        typedef std::unordered_multimap<std::string, Gtk::TreeRow> entry_set;

        /**
         * Pointer to method type with the same signature as the
         * finish callback.
         */
        typedef void(file_list_controller::*finish_method)(bool, int, bool);


        /* Paths */
        
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
        std::shared_ptr<nuc::vfs> vfs;

        /**
         * Flag for whether there is an ongoing read task.
         *
         * Should only be accessed/modified on the main thread.
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
         * being read. 
         *
         * After the operation completes successfully, this list store
         * is set as the tree view's model, 'cur_list' is cleared and
         * 'new_list' and 'cur_list' are swapped.
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
         * Set of marked entries.
         *
         * The set is represented as a map where the key is the name
         * of the entry and the value is the tree view row
         * corresponding to the entry.
         *
         */
        entry_set marked_set;
        

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
        void vfs_new_entry(dir_entry &entry, bool refresh);
        /**
         * VFS finish callback.
         */
        void vfs_finish(bool cancelled, int error, bool refresh);

        /**
         * VFS finish callback for moving up the directory tree when
         * the current directory is deleted.
         *
         * The callback method vfs_finish is replaced with this
         * method, when the current directory is deleted. If the
         * parent directory is read successfully, the file list is
         * displayed and the callback is restored to vfs_finish. If
         * the directory was not read successfully, and it is not the
         * root directory, an attempt is made to read its parent
         * directory.
         */
        void vfs_finish_move_up(paths::string new_path, bool cancelled, int error, bool refresh);

        /**
         * Directory changed callback.
         *
         * Called when the directory has changed, prior to initiating
         * an update operation.
         *
         * @return The finish callback for the update operation.
         */
        vfs::finish_fn vfs_dir_changed();
        
        /**
         * Directory deleted signal handler.
         */
        void vfs_dir_deleted(paths::string new_path);

        
        /* Reading new directories */

        /**
         * Expands relative paths entered by the user into absolute
         * paths, by appending them to the current path.
         *
         * @param path The path to expand.
         *
         * @return The expanded path.
         */
        paths::string expand_path(paths::string path);

        /**
         * Records the current selected row in selected_row, sets the
         * value of move_to_old and sets the view's model to the empty
         * list.
         *
         * @param move_to_old The value of move_to_old to set.
         */
        void prepare_read(bool move_to_old);

        /**
         * Clears the tree view's contents by setting its model to the
         * empty list.
         */
        void clear_view();

        /**
         * Creates the finish callback. Binds the vfs_finish method's
         * this pointer.
         *
         * @return The finish callback function.
         */
        vfs::finish_fn read_finish_callback();

        
        /**
         * Initiates a read operation for the parent directory of the
         * current directory, if it is not the root directory.
         *
         * This function should be used only to move to the parent
         * directory when the current directory has been deleted.
         *
         * @param path The path of the directory whose parent
         *    directory to read.
         */
        void read_parent_dir(paths::string path);
        

        /* Resetting/Setting the treeview model */
        
        /**
         * Restores the old file list and path.
         */
        void reset_list(bool refresh);

        /**
         * Switches the tree view's model to 'new_list' and restores
         * the previous selection if an entry with the same name, as
         * the previously selected entry, still exits. Updates the
         * marked set.
         */
        void set_updated_list();

        /**
         * Updates the marked set, after a directory refresh.
         *
         * Removes entries which no longer exists and updates the row
         * iterators of those entries which do exist to their new row
         * positions.
         */
        void update_marked_set();

        /**
         * Adds the parent ".." pseudo-entry to the new list if the
         * path (@a new_path) is not the root directory.
         *
         * @param new_path The path of the directory being read.
         */
        void add_parent_entry(const paths::string &new_path);
        
        /**
         * Sets the tree view's model to the new list, sets the
         * selection, sets cur_path to the value of 'vfs->path()' and
         * emits a path changed signal.
         */
        void finish_read();
        
        /**
         * Switches the tree view's model to 'new_list', clears
         * 'cur_list' and swaps the two models.
         *
         * @param clear_marked If true the marked set is cleared.
         */
        void set_new_list(bool clear_marked);

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
         * Selects the entry with basename of 'old_path' as its name
         * if 'move_to_old' is true. Otherwise simply selects entry 0.
         */
        void restore_selection();
        
        /**
         * Moves the selection to the entry corresponding to the
         * previous directory (the entry with the same name as the
         * basename of 'old_path').
         */
        void select_old();

        /**
         * Selects the first entry named 'name'. If there is no such
         * entry, selects the row at min(row, number of rows - 1).
         */
        void select_named(const paths::string &name, index_type row = 0);

        
        /* Marking */

        /**
         * Toggles the marked state of a row, and updates the marked
         * set.
         *
         * If the row is not marked, it is marked and added to the
         * marked set. If the row is marked it is unmarked and removed
         * from the marked set.
         *
         * @param row Iterator to the row to toggle mark.
         */
        void mark_row(Gtk::TreeRow row);

        /**
         * Marks/Unmarks a row. 
         *
         * The marked attributes (marked flag, colour, etc) of the row
         * are changed however the marked set is not modified.
         *
         * @param row    Iterator to the row to mark/unmark.
         * @param marked True if the row should be marked.
         */
        void mark_row(Gtk::TreeRow row, bool marked);

        
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

        /** Icons */

        /**
         * Loads the icons for all entries in new_list.
         */
       void load_icons();

        /**
         * Loads the icon for the entry at row @a row, and stores the
         * entry in the icon column of the row.
         *
         * @param row The row.
         */
        void load_icon(Gtk::TreeRow row);

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
         * Attempts to change the directory to the entry @a ent. If @a
         * ent is not a directory entry and is not a file which can be
         * listed, this method does nothing. If @a ent is a directory
         * entry a path changed signal is emitted.
         *
         * @param ent The entry to enter
         *
         * @return true if the entry is a directory entry that can be
         *         listed, false otherwise.
         */
        bool descend(const dir_entry &ent);
        
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
            vfs->cleanup(fn);
        }
    };
}

#endif // NUC_FILE_LIST_H

// Local Variables:
// mode: c++
// End:
