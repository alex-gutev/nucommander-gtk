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
#include <gtkmm/treemodelfilter.h>

#include <unordered_map>
#include <vector>

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
         * File list controller of the file view.
         */
        std::shared_ptr<file_list_controller> flist;

        /**
         * Stack of previous file_list_controller's
         */
        std::vector<std::weak_ptr<file_list_controller>> flist_stack;


        /* Filtering */

        /**
         * Filtered model, which filters out entries based on the
         * current filter.
         */
        Glib::RefPtr<Gtk::TreeModelFilter> filter_model;

        /**
         * Flag: True if the entry list is currently being filtered.
         */
        bool m_filtering = false;


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
         * Search Entry Widget.
         */
        Gtk::Entry *filter_entry;

        /**
         * Activate entry signal.
         */
        signal_activate_entry_type m_signal_activate_entry;


        /* Initialization */

        /**
         * Initializes the file list tree view widget.
         */
        void init_file_list();

        /**
         * Adds the columns of the file list tree view widget.
         */
        void init_columns();

        /**
         * Initializes the scroll adjustments of the tree view and
         * scrolled window.
         */
        void init_scroll_adjustments();

        /**
         * Adds signal handlers for the tree view's event signals.
         */
        void init_file_list_events();


        /**
         * Initializes the path text entry. Connects a signal handler,
         * to the activate signal, which begins a background read
         * operation for the new directory.
         */
        void init_path_entry();

        /**
         * Initializes the filter entry.
         */
        void init_filter_entry();


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
         * Signal handler for the selection changed event of the tree
         * view.
         *
         * Calls the on_selection_changed method of the file list
         * controller.
         */
        void on_selection_changed();

        /**
         * Signal handler for the key press event of the tree view.
         *
         * Calls the on_keypress method of the file list controller.
         */
        bool on_file_list_keypress(GdkEventKey *e);


        /* File List Controller Signal Handlers */

        /**
         * Signal handler for the file list controller's path changed
         * signal.
         */
        void on_path_changed(const paths::pathname &path);

        /**
         * Signal handler for the file list controller's change model
         * signal.
         */
        void change_model(Glib::RefPtr<Gtk::ListStore> model);

        /**
         * Signal handler for the file list controller's select row
         * signal.
         */
        void select_row(Gtk::TreeRow row);


        /* Filtering */

        /**
         * Returns true if the file list is being filtered.
         *
         * @return true.
         */
        bool filtering() const;

        /**
         * Creates a new TreeModelFilter, for the model @a model, and
         * stores it in 'filter_model'.
         *
         * @param model The TreeModel containing the file list.
         */
        void make_filter_model(Glib::RefPtr<Gtk::ListStore> model);

        /**
         * Ends filtering.
         *
         * Clears and hides the filter entry and resets the filter,
         * such that no entries are filtered out.
         */
        void end_filter();

        /**
         * Signal handler for the 'changed' event of the filter entry.
         *
         * Updates the filter.
         */
        void filter_changed();

        /**
         * Signal handler for the keypress event of the filter
         * entry. Connected after the default handler.
         *
         * Forwards the event to the tree view.
         *
         * @param e The event.
         */
        bool filter_key_press(GdkEventKey *e);

        /**
         * Signal handler for the keypress event of the filter
         * entry. Connected before the default handler.
         *
         * Captures the Enter key press and forwards it to the tree
         * view.
         *
         * @param e The event.
         */
        bool filter_key_press_before(GdkEventKey *e);

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
        std::shared_ptr<file_list_controller> file_list() {
            return flist;
        }

        /**
         * Sets the file view's file list controller.
         *
         * @param flist The new file list controller.
         *
         * @param push_old If true the previous file list controller,
         *   is pushed onto the file list controller stack.
         */
        void file_list(std::shared_ptr<file_list_controller> flist, bool push_old = true);

        /**
         * Pops a file list controller off the file list controller
         * stack.
         *
         * @return Shared pointer to the file list controller or null
         *   if the stack is empty.
         */
        std::shared_ptr<file_list_controller> pop_file_list();

        /**
         * Returns the path to the file view's current directory.
         */
        const paths::pathname &path() const {
            return flist->path();
        }

        /**
         * Changes the current path of the file view, this changes the
         * path displayed and begins a background read operation for
         * the new path.
         */
        void path(const paths::pathname &path, bool move_to_old = false);

        /**
         * Moves the keyboard focus to the path entry widget.
         */
        void focus_path();

        /**
         * Return the selected entry or NULL if there is no selected
         * entry.
         *
         * @return dir_entry *
         */
        dir_entry *selected_entry();


        /* Search */

        /**
         * Begins filtering.
         *
         * Shows the filter entry.
         */
        void begin_filter();


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
    };
}

#endif // NUC_FILE_VIEW_H

// Local Variables:
// mode: c++
// End:
