/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef NUC_FILE_LIST_LIST_CONTROLLER_H
#define NUC_FILE_LIST_LIST_CONTROLLER_H

#include <unordered_map>

#include <gtkmm/liststore.h>

#include "directory/dir_entry.h"


namespace nuc {
    /**
     * Abstract File List Controller Interface.
     *
     * Provides an interface to a generic file list controller, which
     * is responsible for maintaining and updating the list of file
     * entries, the set of marked entries and the selection.
     */
    class list_controller {
    public:
        /* Signal Types */

        /**
         * Tree model changed signal type.
         *
         * Prototype: void(Glib::RefPtr<Gtk::ListStore> model)
         *
         * @param model The new tree view model.
         */
        typedef sigc::signal<void, Glib::RefPtr<Gtk::ListStore>> signal_change_model_type;

        /**
         * Select row signal type.
         *
         * Prototype: void(Gtk::TreeRow row)
         *
         * @param row The row that should be selected.
         */
        typedef sigc::signal<void, Gtk::TreeRow> signal_select_type;


        /* Destructor */
        virtual ~list_controller() noexcept = default;


        /**
         * Model changed signal.
         *
         * Emitted when the underlying list store model has
         * changed. The new model is passed as an argument to the
         * signal handler.
         */
        signal_change_model_type signal_change_model() {
            return m_signal_change_model;
        }

        /**
         * Select row signal.
         *
         * Emitted when the selection should be changed to the row,
         * passed as an argument to the handler.
         */
        signal_select_type signal_select() {
            return m_signal_select;
        }


        /**
         * Returns the list store model.
         *
         * @return The Gtk::ListStore
         */
        virtual Glib::RefPtr<Gtk::ListStore> list() = 0;

        /**
         * Returns the selected row.
         *
         * @return The row.
         */
        virtual Gtk::TreeRow selected() const = 0;

        /**
         * Returns the list of selected/marked entries.
         *
         * @return An std::vector of the dir_entry objects
         *   corresponding to the selected/marked entries.
         */
        virtual std::vector<dir_entry*> selected_entries() const = 0;


        /**
         * Toggles the marked state of a row, and updates the marked
         * set.
         *
         * If the row is not marked, it is marked and added to the
         * marked set. If the row is marked it is unmarked and removed
         * from the marked set.
         *
         * @param row Iterator to the row to mark/unmark.
         */
        virtual void mark_row(Gtk::TreeRow row) = 0;


        /* Tree View Events */

        /**
         * Should be called when the tree view's selection has been
         * changed, both for external and internal, in response to the
         * 'select_row' signal, changes.
         *
         * @param row The selected row.
         */
        virtual void on_selection_changed(Gtk::TreeRow row) = 0;

    protected:
        /**
         * Model changed signal.
         */
        signal_change_model_type m_signal_change_model;

        /**
         * Select row signal.
         */
        signal_select_type m_signal_select;
    };
}  // nuc

#endif /* NUC_FILE_LIST_LIST_CONTROLLER_H */

// Local Variables:
// mode: c++
// End:
