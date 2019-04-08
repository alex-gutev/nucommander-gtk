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

#ifndef FILE_LIST_FILTER_LIST_CONTROLLER_H
#define FILE_LIST_FILTER_LIST_CONTROLLER_H

#include <functional>
#include <tuple>

#include "list_controller.h"
#include "file_model_columns.h"

namespace nuc {
    /**
     * Filtered List Controller.
     *
     * Mains a list of entries which are filtered out from another
     * list.
     */
    class filtered_list_controller : public list_controller, public sigc::trackable {
    public:
        /**
         * Filter Function.
         *
         * Prototype: std::pair<bool, float>(Gtk::TreeRow row)
         *
         * @param row The row of the original file list.
         *
         * @return A pair of two values: a boolean, which if true, the
         *   row is included in the filtered list, and a score
         *   indicating how closely the filter matches the
         *   entry. Entries are ordered by the score in ascending
         *   order, that is lower scores indicate greater accuracy.
         */
        typedef std::function<std::pair<bool, float>(Gtk::TreeRow)> filter_fn;

        /**
         * Creates a filtered_list controller.
         *
         * @param flist The list controller, the file list of which,
         *   to filter.
         *
         * @param filter The filter function.
         */
        static std::shared_ptr<filtered_list_controller> create(std::shared_ptr<list_controller> flist, filter_fn filter);

        /* Constructor */
        filtered_list_controller(std::shared_ptr<list_controller> flist, filter_fn filter);

        /**
         * Rebuilds the filtered file list by calling the filter
         * function on each row of the original file list.
         *
         * @param selection The row which should be the new selected
         *   row. If not provided defaults to the original file list
         *   controller's current selection.
         */
        void refilter();
        void refilter(Gtk::TreeRow selection);


        /* list_controller Methods */

        virtual Glib::RefPtr<Gtk::ListStore> list() {
            return m_list;
        }

        virtual Gtk::TreeRow selected() const {
            return selected_row;
        }

        virtual std::vector<dir_entry*> selected_entries() const;

        virtual void mark_row(Gtk::TreeRow row);

        virtual void on_selection_changed(Gtk::TreeRow row);

    private:
        /** Filter Function */
        filter_fn m_filter;

        /** Original File List */
        std::shared_ptr<list_controller> flist;

        /** Filtered Liststore model */
        Glib::RefPtr<Gtk::ListStore> m_list;

        /** Selected row in filtered list. */
        Gtk::TreeRow selected_row;


        /**
         * Add a new row, of the original file list, to the filtered
         * file list.
         *
         * @param row The row of the original file list.
         * @param score The accuracy score of the row.
         *
         * @return The new row in the filtered file list.
         */
        Gtk::TreeRow add_row(Gtk::TreeRow row, float score);

        /**
         * Copy the values of all columns, excluding 'score', from row
         * @a src, to row @a dest.
         *
         * @param src The row whose column values to copy.
         * @param dest The row whose column values to set.
         */
        void copy_columns(Gtk::TreeRow src, Gtk::TreeRow dest);


        /* List Controller Signal Handlers */

        /**
         * Handler for the change model signal.
         */
        void change_model(Glib::RefPtr<Gtk::ListStore> model);
        /**
         * Handler for the select row signal.
         */
        void select_row(Gtk::TreeRow row);
    };

}  // nuc

#endif /* FILE_LIST_FILTER_LIST_CONTROLLER_H */

// Local Variables:
// mode: c++
// End:
