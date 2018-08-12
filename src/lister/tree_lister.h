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

#ifndef NUC_TREE_LISTER_H
#define NUC_TREE_LISTER_H

#include <vector>
#include <functional>

#include "lister.h"

namespace nuc {
    /**
     * Generic interface for listing directory trees.
     */
    class tree_lister {
    public:
        /**
         * Describes the traversal stage of the current entry.
         */
        enum visit_info {
            /**
             * Used for files which are only visited once.
             */
            visit_info_none = 0,

            /**
             * Directory visited in preorder
             */
            visit_preorder,
            /**
             * Directory visited in postorder
             */
            visit_postorder,

            /**
             * Directory visited which causes a cycle in the tree.
             */
            visit_cycle
        };

        /**
         * Callback function type, called for each entry in the tree.
         *
         * Takes three arguments:
         *
         * - The entry's details as a lister::entry struct.
         *
         * - The entry's stat attributes, NULL if the stat attributes
         *   are unavailable (sometimes unavailable for directories
         *   visited in pre-order), or could not be obtained.
         *
         * - The entry's visit info
         */
        typedef std::function<void(const lister::entry &, const struct stat *, visit_info)> list_callback;

        /**
         * Error exception.
         */
        class error : public std::exception {
            const int m_code;

        public:

            error(int code) : m_code(code) {}

            /**
             * Returns the error code.
             */
            int code() const {
                return m_code;
            }
        };

        virtual ~tree_lister() = default;


        /**
         * Reads all entries in the tree and calls @a fn on each
         * entry.
         *
         * @param fn A function to call on each entry.
         */
        virtual void list_entries(const list_callback &fn) = 0;

        /**
         * Opens the last entry read, for reading. This may only be
         * used if the last entry is a regular file.
         *
         * @return An instream object for reading the entry's
         *    contents.
         */
        virtual instream * open_entry() = 0;

    protected:
        /**
         * Throws an exception with error code @a code.
         */
        void raise_error(int code) {
            throw error(code);
        }
    };
}


#endif // NUC_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
