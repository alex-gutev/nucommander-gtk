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

#ifndef NUC_DIR_TREE_LISTER_H
#define NUC_DIR_TREE_LISTER_H

#include "tree_lister.h"

#include "paths/pathname.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

namespace nuc {
    /**
     * Regular Directory Tree Lister.
     */
    class dir_tree_lister : public tree_lister {
        /**
         * FTS directory tree handle.
         */
        FTS *handle = nullptr;

        /**
         * Last entry read.
         */
        FTSENT *last_ent = nullptr;


        /**
         * Converts the value of the fts_info field, of the entry, to
         * a dirent file type constant.
         *
         * @param ent The FTS entry.
         *
         * @return The type of the entry as a dirent constant.
         */
        static int get_type(FTSENT *ent);

        /**
         * Checks whether there was an error obtaining the stat
         * attributes of the entry.
         *
         * @param ent The FTS entry.
         *
         * @return True if there was an error obtaining the stat
         *    attributes of the entry.
         */
        static bool stat_err(FTSENT *ent);

        /**
         * Returns the new current directory.
         *
         * @param ent The FTS entry.
         * @param name The name of the entry.
         * @param dir The current directory.
         *
         * @return The new current directory.
         */
        static paths::pathname set_dir(FTSENT *ent, const paths::string &name, paths::pathname dir);

        /**
         * Returns the visit info for the entry @a ent. If the entry
         * is not a regular directory visit_preorder is always
         * returned.
         *
         * @param ent The FTS entry.
         */
        static visit_info get_visit_info(FTSENT *ent);

    public:

        /**
         * Creates a directory tree lister for the
         * sub-directories/files @a path in the directory at @a base.
         *
         * @param base Path to the base directory of the tree.
         *
         * @param paths Array of names of sub-directories/files which
         *    are to be listed.
         */
        dir_tree_lister(const paths::pathname &base, const std::vector<paths::pathname> &paths);

        virtual ~dir_tree_lister();

        /* Method Overrides */

        virtual void list_entries(const list_callback &fn);

        virtual std::string symlink_path();

        virtual instream * open_entry();
    };
}

#endif // NUC_DIR_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
