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

#ifndef NUC_ARCHIVE_TREE_LISTER_H
#define NUC_ARCHIVE_TREE_LISTER_H

#include <map>
#include <set>

#include "tree_lister.h"
#include "archive_lister.h"

#include "paths/pathname.h"

namespace nuc {
    /**
     * Archive tree lister.
     *
     * Lists directory trees within archives.
     */
    class archive_tree_lister : public tree_lister {
        /**
         * Set of subpaths to visit (base paths).
         */
        std::set<paths::pathname> visit_paths;
        /**
         * Attributes of the visited directories.
         */
        std::map<paths::pathname, std::pair<struct stat, bool>> visited_dirs;

        /**
         * Archive lister for reading the archive.
         */
        archive_lister listr;

        /**
         * Checks whether the entry @a path should be visited,
         * i.e. whether it is a child of any entry in
         * visit_paths. Adds the intermediate directory components of
         * @a path to visited_dirs if not already in visited_dirs and
         * calls the callback function for each component (if it
         * hasn't been visited yet).
         *
         * @param path Subpath of the entry.
         *
         * @return The offset within, @a path, of the base component
         *   (of the paths in visit_paths of which @a path is a
         *   subpath), if the path should be
         *   visited. paths::string::npos if the path should not be
         *   visisted.
         */
        size_t path_offset(const paths::pathname &path);

        /**
         * Adds the intermediate directory components of @a path to
         * visited_dirs, and calls @a fn for each component if it has
         * not been visited yet.
         *
         * @param base_offset Index of the first character of the
         *    path, following the base path (passed to the
         *    constructor). Only the components following this index
         *    will be added to visited_dirs.
         *
         * @param path Subpath of the entry being visited.
         */
        bool add_visited_dirs(size_t base_offset, const paths::pathname &path);

        /**
         * Adds the stat attributes of a directory to the visited_dirs
         * map.
         *
         * @param name Subpath to the directory.
         * @param st Stat attributes
         *
         * @return True if the directory has not yet been visited, false
         *   if it has been visited.
         */
        bool add_dir_stat(const paths::pathname &name, const struct stat *st);

    public:

        /**
         * Creates an archive tree lister.
         *
         * @param plugin Plugin for reading the archive.
         *
         * @param archive Path to the actual archive file.
         *
         * @param paths Subpaths, within the archive, of the directory
         *    trees to list (base paths). All paths should be
         *    canonicalized and paths to directories should have a
         *    trailing slash '/'.
         */
        archive_tree_lister(archive_plugin *plugin, const paths::pathname &archive, const std::vector<paths::pathname> &paths);


        /* Method Overrides */

        virtual void list_entries(const list_callback &fn);

        virtual instream * open_entry() {
            return listr.open_entry();
        }

        virtual std::string symlink_path();
    };
}

#endif // NUC_ARCHIVE_TREE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
