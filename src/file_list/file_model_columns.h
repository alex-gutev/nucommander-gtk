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

#ifndef NUC_FILE_LIST_FILE_MODEL_COLUMNS_H
#define NUC_FILE_LIST_FILE_MODEL_COLUMNS_H

#include <gtkmm/treemodelcolumn.h>
#include <gdkmm/rgba.h>
#include <gdkmm/pixbuf.h>

#include "directory/dir_entry.h"
#include "columns.h"

namespace nuc {
    /**
     * Model for the file list tree view.
     */
    class file_model_columns : public Gtk::TreeModelColumnRecord {
    public:
        /**
         * Pointer to the directory entry.
         */
        Gtk::TreeModelColumn<dir_entry *> ent;

        /**
         * Flag: true if the row is marked.
         */
        Gtk::TreeModelColumn<bool> marked;

        /**
         * Filter match score.
         */
        Gtk::TreeModelColumn<float> score;

        /**
         * Text colour of the row.
         */
        Gtk::TreeModelColumn<Gdk::RGBA> color;

        /**
         * File icon.
         */
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;

        /**
         * Array of column descriptors of the columns which are
         * displayed.
         */
        std::vector<column_descriptor*> columns;

        /**
         * Returns the index of the TreeModelColumn corresponding to
         * the value displayed in the first column.
         *
         * @param The index.
         */
        size_t first_column_index() const {
            return color.index() + 1;
        }

        /**
         * Returns the singleton instance.
         */
        static file_model_columns &instance();

    private:
        /** Constructor */
        file_model_columns();
    };
}

#endif // NUC_FILE_LIST_FILE_MODEL_COLUMNS_H

// Local Variables:
// mode: c++
// End:
