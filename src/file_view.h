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

#include "paths/pathname.h"

#include "file_list/file_list_controller.h"

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
         * Activate entry signal type.
         *
         * The signal is passed three arguments:
         *
         *   - Pointer to the "this" file_view.
         *   - Pointer to the file_list_controller.
         *   - Pointer to the activated entry.
         */
        typedef sigc::signal<void, file_view *, file_list_controller *, dir_entry *> signal_activate_entry_type;

        /**
         * Key press event signal type.
         */
        typedef decltype(((Gtk::Frame*)nullptr)->signal_key_press_event()) signal_key_press_event_type;

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
        Gtk::TreeView *file_list_view;

        /**
         * Scrolled window widget in which the file list tree view
         * widget is contained.
         */
        Gtk::ScrolledWindow *scroll_window;

        /**
         * Activate entry signal.
         */
        signal_activate_entry_type m_signal_activate_entry;


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
        void on_path_changed(const paths::pathname &path);


    public:
        /**
         * The opposite file_view, i.e. the destination pane.
         */
        file_view * next_file_view;

        /**
         * Derived widget constructor.
         */
        file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Returns the file list controller.
         *
         * @return The file list controller.
         */
        file_list_controller &file_list() {
            return flist;
        }

        /**
         * Returns the path to the file view's current directory.
         */
        const paths::pathname &path() const {
            return flist.path();
        }

        /**
         * Changes the current path of the file view, this changes the
         * path displayed and begins a background read operation for
         * the new path.
         */
        void path(const paths::pathname &path, bool move_to_old = false);


        /* Copy Tasks */

        /**
         * Creates a task which copies the selected/marked files to a
         * destination directory.
         *
         * @param dest Path to the destination directory.
         *
         * @return The copy task.
         */
        task_queue::task_type make_copy_task(const paths::pathname &dest) {
            return flist.make_copy_task(dest);
        }

        /**
         * Creates a tree lister for listing the marked/selected files
         * and the contents of any marked/selected directories.
         *
         * @return The tree lister.
         */
        tree_lister *get_tree_lister();

        /**
         * Returns a directory writer for modifying files in the
         * file_view's directory.
         *
         * @return The directory writer.
         */
        dir_writer *get_dir_writer();


        /* Signals */

        /**
         * Activate entry signal. This signal is emitted whenever the
         * an (a row in the file tree view) is activate either by
         * double clicking on it or pressing the return key.
         *
         * @return The signal.
         */
        signal_activate_entry_type signal_activate_entry() {
            return m_signal_activate_entry;
        }

        /**
         * Key press event signal, which is emitted for all keypress
         * events emitted while the tree view has the keyboard focus.
         */
        signal_key_press_event_type signal_key_press() {
            return file_list_view->signal_key_press_event();
        }


        /* Cleanup */

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
