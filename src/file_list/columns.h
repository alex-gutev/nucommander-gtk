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

#ifndef FILE_LIST_COLUMNS_H
#define FILE_LIST_COLUMNS_H

#include <glibmm.h>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "file_model_columns.h"
#include "sort_func.h"

namespace nuc {
    /**
     * Column Descriptor.
     *
     * Provides an interface for creating the column and its sort
     * function.
     */
    struct column_descriptor {
        /**
         * Column Identifier
         */
        const int id;

        /**
         * Constructor.
         *
         * @param id Column Identifier
         */
        column_descriptor(int id) : id(id) {}

        virtual ~column_descriptor() = default;


        /**
         * Creates a new instance of the column.
         *
         * @return The Gtk::TreeView::Column instance.
         */
        virtual Gtk::TreeView::Column *create() = 0;

        /**
         * Creates the column's sort function.
         *
         * @param order The sort order.
         *
         * @return The sort function.
         */
        virtual Gtk::TreeSortable::SlotCompare sort_func(Gtk::SortType order = Gtk::SortType::SORT_ASCENDING) = 0;
    };

    /**
     * Contains all file column descriptors.
     *
     * Automatically initialized with the built-in descriptors. The
     * id's of the built-in descriptors match the indices at which
     * they are located within the vector.
     */
    extern std::vector<std::unique_ptr<column_descriptor>> file_column_descriptors;
};

#endif

// Local Variables:
// mode: c++
// End:
