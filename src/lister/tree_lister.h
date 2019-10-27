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

#ifndef NUC_LISTER_TREE_LISTER_H
#define NUC_LISTER_TREE_LISTER_H

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
         *
         * When visiting a directory and the return value is true,
         * traversal continues into the directory, otherwise if the
         * return value is false the contents of the directory are not
         * visited.
         *
         * When visiting a any file which is not a directory, the
         * return value is ignored.
         */
        typedef std::function<bool(const lister::entry &, const struct stat *, visit_info)> list_callback;

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
         * Add a callback function which will be called when each
         * entry is visited. The function will be called before the
         * current function is called if any.
         *
         * @param fn The callback function.
         */
        void add_list_callback(const list_callback &fn);

        /**
         * Reads all entries in the tree and calls @a fn on each
         * entry.
         *
         * @param fn A function to call on each entry.
         */
        virtual void list_entries(const list_callback &fn) = 0;

        /**
         * Retrieves the realpath of the last entry read if it was a
         * symbolic link.
         *
         * @return Target path of the symbolic link.
         */
        virtual std::string symlink_path() = 0;

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
         * Function which is called when each entry is visited.
         *
         * Subclasses should use add_list_callback to add the function
         * passed to list_entries, and then call the function stored
         * in this member variable.
         */
        list_callback list_fn;

        /**
         * Throws an exception with error code @a code.
         */
        void raise_error(int code) {
            throw error(code);
        }
    };
}


#endif // NUC_LISTER_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
