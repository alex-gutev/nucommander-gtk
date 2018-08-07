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
         * Current subdirectory of the tree.
         */
        paths::string current_dir;

        /**
         * The subpath (from the root of the tree), of the last entry.
         */
        paths::string last_path;

        /**
         * Converts the value of the fts_info field, of the last entry
         * read, to a dirent file type constant.
         *
         * @return The type of the last entry as a dirent constant.
         */
        int get_type() const;

        /**
         * Checks whether there was an error obtaining the stat
         * attributes of the last entry.
         *
         * @return True if there was an error obtaining the stat
         *    attributes of the last entry.
         */
        bool stat_err() const;

        /**
         * If the last entry visited is a directory, updates the
         * current directory (current_dir).
         */
        void set_dir();

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
        dir_tree_lister(const paths::string &base, const std::vector<paths::string> &paths);

        virtual ~dir_tree_lister() {
            close();
        }

        /* Method Overrides */

        virtual void close();

        virtual bool read_entry(entry &ent);
        virtual bool entry_stat(struct stat &st);

        virtual instream * open_entry();

        virtual visit_info entry_visit_info() const;
    };
}

#endif // NUC_DIR_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
