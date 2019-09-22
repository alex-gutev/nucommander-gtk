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
#include <memory>

#include "paths/pathname.h"

#include "list_controller.h"
#include "file_model_columns.h"
#include "directory/vfs.h"

namespace nuc {
    /**
     * Directory File List Controller.
     *
     * Maintains a list of the contents of a directory, and updates
     * the list in response to file system changes.
     */
    class file_list_controller : public list_controller, public std::enable_shared_from_this<file_list_controller> {
    public:
        /* Types */

        /**
         * Row index type. Matches the Gtk type.
         */
        typedef Gtk::TreeNodeChildren::size_type index_type;

        /**
         * Marked entries set type.
         *
         * Maps file names to row iterators (Gtk::TreeRow).
         *
         * Row iterators are used, instead of row references, as
         * Gtk::ListStore supports persistent iterators.
         */
        typedef std::unordered_multimap<std::string, Gtk::TreeRow> entry_set;


        /* Signal Types */

        /**
         * Path changed signal type.
         *
         * Prototype: void(const path_str &path)
         *
         * @param path The new path.
         */
        typedef sigc::signal<void, const pathname &> signal_path_type;


        /**
         * Creates a file_list_controller.
         */
        static std::shared_ptr<file_list_controller> create();


        /* Changing the path */

        /**
         * Changes the current path of the file view.
         *
         * Begins a read operation to read the entries of the
         * directory @a path into the list.
         *
         * Emits the 'path_changed' signal.
         *
         * @param path Path to the directory to read.
         *
         * @param move_to_old If true the selection is moved to the
         *   entry with the same names as the current directory.
         */
        void path(const pathname &path, bool move_to_old = false);

        /**
         * Returns the current path.
         *
         * @return The current path.
         */
        const pathname &path() const {
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


        /* VFS Object */

        /**
         * Returns a reference to the vfs object responsible for
         * reading the directory.
         *
         * @return The vfs object.
         */
        nuc::vfs & dir_vfs() {
            return vfs;
        }


        /* Signals */

        /**
         * Path changed signal.
         *
         * Emitted when the current path has changed.
         */
        signal_path_type signal_path() {
            return m_signal_path;
        }

        /**
         * Returns true if the file_list_controller is attached to a
         * file_view.
         *
         * @return     bool
         */
        bool attached() const {
            return m_signal_path.size();
        }


        /* list_controller Methods */

        virtual Glib::RefPtr<Gtk::ListStore> list() {
            return cur_list;
        }

        virtual Gtk::TreeRow selected() const {
            return selected_row;
        }

        virtual std::vector<dir_entry*> selected_entries() const;

        virtual void mark_row(Gtk::TreeRow row);

        virtual void on_selection_changed(Gtk::TreeRow row);

    private:
        /**
         * The current path.
         */
        pathname cur_path;

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
        dir_entry parent_entry{"..", dir_entry::type_parent};


        /* Tree View Model */

        /**
         * List store model containing the contents of the directory.
         */
        Glib::RefPtr<Gtk::ListStore> cur_list;

        /**
         * Empty list store.
         *
         * Used to display an empty tree view, while reading the new
         * directory list, without discarding the old list.
         */
        Glib::RefPtr<Gtk::ListStore> empty_list;


        /* Selection and Marked Entry State */

        /**
         * The selected row.
         */
        Gtk::TreeRow selected_row;

        /**
         * Set of marked entries.
         *
         * The set is represented as a map where the key is the name
         * of the entry and the value is the tree model row
         * corresponding to the entry.
         */
        entry_set marked_set;


        /* Initialization */

        /**
         * Creates a list store model with the default sort column
         * set.
         *
         * @return The list store model
         */
        static Glib::RefPtr<Gtk::ListStore> create_model();

        /**
         * Creates a list store model, with the 'file_model_columns'
         * column record and a handler for the 'sort_column_changed'
         * signal.
         *
         * @return The list store model.
         */
        static Glib::RefPtr<Gtk::ListStore> make_liststore();

        /**
         * Sets the sort column of the list store to 'name'.
         *
         * @param model The list store model.
         */
        static void init_liststore(Glib::RefPtr<Gtk::ListStore> model);


        /**
         * Initializes the VFS object. Sets the callback functions.
         */
        void init_vfs();


        /* VFS Operation Delegates */

        /**
         * VFS Read Operation Delegate.
         */
        struct read_delegate : public vfs::delegate {
            /** File List Controller */
            std::weak_ptr<file_list_controller> flist;

            /**
             * List store model into which the entries are read.
             */
            Glib::RefPtr<Gtk::ListStore> list;

            read_delegate(std::weak_ptr<file_list_controller> flist);

            virtual void begin();
            virtual void new_entry(dir_entry &ent);
            virtual void finish(bool cancelled, int error);
        };

        /**
         * VFS Refresh Operation Delegate.
         */
        struct update_delegate : public read_delegate {
            using read_delegate::read_delegate;

            virtual void finish(bool cancelled, int error);
        };

        /**
         * VFS read operation delegate for reading the parent
         * directory, after the current directory has been deleted.
         */
        struct move_up_delegate : public read_delegate {
            /** Path to the directory being read */
            pathname path;

            move_up_delegate(std::weak_ptr<file_list_controller> flist, pathname path)
                : read_delegate(flist), path(path) {}

            virtual void finish(bool cancelled, int error);
        };


        /* Callbacks */

        /**
         * Directory changed callback.
         *
         * Called when the directory has changed, prior to initiating
         * an update operation.
         *
         * @return The update operation delegate.
         */
        std::shared_ptr<vfs::delegate> vfs_dir_changed();

        /**
         * Directory deleted signal handler.
         */
        void vfs_dir_deleted(pathname new_path);


        /* Reading new directories */

        /**
         * Expands relative paths entered by the user into absolute
         * paths, by appending them to the current path.
         *
         * @param path The path to expand.
         *
         * @return The expanded path.
         */
        pathname expand_path(const pathname &path);

        /**
         * Performs certain tasks which need to be performed prior to
         * a read operation.
         *
         * Sets the value of move_to_old and emits the 'model_changed'
         * signal with the empty list store model.
         *
         * @param move_to_old The value of move_to_old to set.
         */
        void prepare_read(bool move_to_old);

        /**
         * Emits the 'model_changed' signal with the empty list store
         * model.
         */
        void clear_view();


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
        void read_parent_dir(pathname path);


        /* Setting/Resetting the treeview model */

        /**
         * Sets the new file list, containing the contents of the
         * directory just read.
         *
         * Emits 'model_changed', 'select_row' and 'path_changed'
         * signals.
         *
         * @param new_list The new list to set.
         */
        void finish_read(Glib::RefPtr<Gtk::ListStore> new_list);

        /**
         * Sets 'cur_list' to @a new_list.
         *
         * Emits the 'model_changed' signal.
         *
         * @param new_list The list to set 'cur_list' to.
         *
         * @param clear_marked If true the marked set should be
         *   cleared.
         */
        void set_new_list(Glib::RefPtr<Gtk::ListStore> new_list, bool clear_marked);

        /**
         * Adds the parent ".." pseudo-entry to the new list if the
         * path (@a new_path) is not the root directory.
         *
         * @param new_list The list to add the entry to.
         * @param new_path The path of the directory being read.
         */
        void add_parent_entry(Glib::RefPtr<Gtk::ListStore> new_list, const pathname &new_path);

        /**
         * Sets the sort column (and sort order) of @a new_list to be
         * the same as 'cur_list'.
         *
         * @param new_list The list of which to set the sort column.
         */
        void set_sort_column(Glib::RefPtr<Gtk::ListStore> new_list);


        /**
         * Restores the old file list and path.
         */
        void reset_list();

        /**
         * Sets the new 'refreshed' file list.
         *
         * Emits the 'model_changed' signal with the new list model
         * and restores the previous selection if an entry with the
         * same name, as the previously selected entry, still
         * exits. Updates the marked set.
         *
         * @param new_list The new list to set.
         */
        void set_updated_list(Glib::RefPtr<Gtk::ListStore> new_list);

        /**
         * Updates the marked set, after a directory refresh.
         *
         * Removes entries which no longer exist and updates the row
         * iterators of the remaining entries to their new row
         * iterators.
         */
        void update_marked_set();


        /* Selection */

        /**
         * Emits a 'select_row' signal for a given row.
         *
         * @param row Index of the row to select.
         */
        void select_row(index_type row);

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
        void select_named(const pathname::string &name, index_type row = 0);


        /* Marking */

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


        /** Constructor */
        file_list_controller();
    };
}

#endif // NUC_FILE_LIST_H

// Local Variables:
// mode: c++
// End:
