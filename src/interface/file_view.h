/*
 * NuCommander
 * Copyright (C) 2018-2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef NUC_INTERFACE_FILE_VIEW_H
#define NUC_INTERFACE_FILE_VIEW_H

#include <glibmm.h>

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>

#include <unordered_map>
#include <vector>

#include "paths/pathname.h"

#include "file_list/file_list_controller.h"

namespace nuc {
    /**
     * File view derived widget.
     *
     * Manages the file view widget which contains a text entry
     * displaying the current path, and a tree-view widget where the
     * file-list is displayed.
     *
     * The class also initiates background read operations in response
     * to user events triggered by the child widgets.
     */
    class file_view : public Gtk::Frame {
    public:
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
         * The opposite file_view, i.e. the destination pane.
         */
        file_view * next_file_view;


        /**
         * Derived widget constructor.
         */
        file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> &builder);


        /* File List Controller */

        /**
         * Returns the file list controller.
         *
         * @return file_list_controller
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


        /* Path */

        /**
         * Returns the path to the file view's current directory.
         */
        const pathname &path() const {
            return flist->path();
        }

        /**
         * Changes the current path of the file view, this changes the
         * path displayed and begins a background read operation for
         * the new path.
         */
        void path(const pathname &path, bool move_to_old = false);


        /* Changing Keyboard Focus */

        /**
         * Moves the keyboard focus to the path entry widget.
         */
        void focus_path();


        /* Getting Selected Entries */

        /**
         * Return the selected entry or NULL if there is no selected
         * entry.
         *
         * @return dir_entry *
         */
        dir_entry *selected_entry();

        /**
         * Returns the list of all marked entries. If there are no
         * marked entries, a list containing only the selected entry
         * is returned. If there is no selected entry an empty list is
         * returned.
         *
         * @return List of entries.
         */
        std::vector<dir_entry*> selected_entries() const;


        /* Directory VFS */

        /**
         * Returns a pointer to the vfs object for reading the current
         * directory.
         *
         * @return Pointer to the vfs object or NULL if the file_view
         * is not currently displaying a directory.
         */
        nuc::vfs * dir_vfs() const;


        /* Filtering */

        /**
         * Begins filtering.
         *
         * Shows the filter entry.
         */
        void begin_filter();

        /**
         * Begins filtering and sets the contents of the filter text
         * entry.
         *
         * @param str The contents of the filter entry.
         */
        void begin_filter(const Glib::ustring &str);


        /* Signals */

        /**
         * Activate entry signal. This signal is emitted whenever an
         * entry (a row in the file tree view) is activated either by
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

    private:
        /**
         * Stores the connections to the file list controller signals.
         */
        struct list_controller_signals {
            sigc::connection path;
            sigc::connection model_change;
            sigc::connection select_row;
        };


        /* File List Controller */

        /**
         * File list controller for the current directory.
         */
        std::shared_ptr<file_list_controller> flist;

        /**
         * Stack of previous file_list_controller's.
         */
        std::vector<std::weak_ptr<file_list_controller>> flist_stack;

        /**
         * File list controller signal connections.
         */
        list_controller_signals signals;


        /* Filtering */

        /**
         * Filtered list controller, which filters out entries based
         * on the current filter.
         *
         * When not filtering, this is simply set to flist.
         */
        std::shared_ptr<list_controller> filtered_list;

        /**
         * Flag: True if the list is currently being filtered.
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


        /* Signals */

        /**
         * Activate entry signal.
         */
        signal_activate_entry_type m_signal_activate_entry;


        /* Marking */

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


        /* Keyboard Events */

        /**
         * Handler for the up/down arrow key press event.
         *
         * @param e The keyboard event.
         */
        void keypress_arrow(const GdkEventKey *e);

        /**
         * Handler for a keypress which changes the current selection:
         * Home/End/Page Up/Down.
         *
         * Sets 'mark_rows' to true in order for all rows between the
         * current selection and the new selection to be marked (when
         * the selection changed signal is emitted), if the shift
         * modifier key was held down.
         *
         * @param e The keyboard event.
         *
         * @param mark_sel True if the new selected row should also be
         *   marked, false if the last row to be marked is the row
         *   before the selected row.
         */
        void keypress_change_selection(const GdkEventKey *e, bool mark_selected);


        /* File List Controller Signal Handlers */

        /**
         * Connects the signal handlers of the 'change_model' and
         * 'select_row' signals of @a list.
         *
         * @param list The list controller.
         */
        void connect_model_signals(const std::shared_ptr<list_controller> &list);

        /**
         * Signal handler for the file list controller's path changed
         * signal.
         */
        void on_path_changed(const pathname &path);

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
         * Creates a new filtered_list_controller and stores it in
         * 'filtered_list'.
         */
        void make_filter_model();

        /**
         * Ends filtering.
         *
         * Clears and hides the filter entry and resets filtered_list
         * to flist.
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
    };
}

#endif // NUC_INTERFACE_FILE_VIEW_H

// Local Variables:
// mode: c++
// End:
