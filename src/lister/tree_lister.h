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

#include "lister.h"

namespace nuc {
    /**
     * Generic interface for listing directory trees.
     */
    class tree_lister : public lister {
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

        /** lister methods */

        virtual void close() = 0;

        virtual bool read_entry(entry &ent) = 0;
        virtual bool entry_stat(struct stat &st) = 0;

        virtual instream * open_entry() = 0;

        /**
         * Returns the traversal stage for the last entry.
         *
         * @return A visit_info constant describing the traversal
         *   stage.
         */
        virtual visit_info entry_visit_info() const = 0;
    };
}


#endif // NUC_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
