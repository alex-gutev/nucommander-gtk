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

#ifndef NUC_ARCHIVE_TREE_H
#define NUC_ARCHIVE_TREE_H

#include <unordered_map>

#include "dir_tree.h"

namespace nuc {
    /**
     * Archive directory tree.
     *
     * Provides a directory tree abstraction for archives. Archives
     * may contain multiple subdirectories and duplicate files.
     */
	class archive_tree : public dir_tree {
        /**
         * Directory map.
         *
         * Each key is the subpath of the subdirectory, with the
         * corresponding value being an unordered multimap of the
         * child entries of the subdirectory.
         */
        std::unordered_map<paths::string, file_map<dir_entry *>> dirs;

        /**
         * Current subpath within the tree. The empty string indicates
         * the base directory.
         */
        paths::string m_subpath;
        
        /**
         * Extracts and creates the intermediate directory components
         * of an entry.
         *
         * @param path The canonicalized path to the entry.
         * @param ent  Reference to the entry.
         *
         * @return A pointer to the intermediate directory entry which
         *     is a direct child entry of the current subpath. If
         *     there is no entry which is a child of the current path,
         *     or that entry has already been returned once, nullptr
         *     is returned.
         */
        dir_entry *add_components(const paths::string &path, dir_entry &ent);
        
        /**
         * Creates a new directory entry with subpath @a path, if the
         * tree does not contain a directory entry at that subpath
         * already.
         *
         * @param path The subpath of the directory entry.
         *
         * @return Returns a reference to the newly created directory
         * entry or the existing directory entry.
         */
        dir_entry &make_dir_ent(const paths::string &path);

        /**
         * Checks whether a path is a child of the current subpath.
         *
         * @param path The path to test
         *
         * @return true if @a path is a child of the current subpath.
         */
        bool in_subpath(const paths::string &path);

        /**
         * Adds a directory entry to the tree. If the tree already
         * contains a directory entry with the same subpath, it is
         * replaced with @a ent.
         *
         * @param ent The directory entry to add.
         *
         * @return Pointer to the entry within the tree.
         */
        dir_entry * add_dir_entry(dir_entry ent);
        
        /**
         * Adds an entry to a multi-map if the map does not already
         * contain the entry. This is determined by pointer comparison
         *
         * @param map  The multi-map, in which to add the entry.
         * @param name The name of the entry.
         * @param ent  Pointer to the entry.
         *
         * @return True if the entry was added to the map, false if
         *    the map already contained the entry.
         */
        static bool add_to_map(file_map<dir_entry *> &map, const paths::string &name, dir_entry *ent);
        
	public:

        /**
         * Constructs an archive directory tree with the subpath set
         * to the base directory.
         */
        archive_tree() = default;

        /**
         * Constructs an archive directory tree with a particular
         * subpath.
         *
         * @param subpath The subpath
         */
        archive_tree(paths::string subpath) : m_subpath(std::move(subpath)) {}
        
        /* Method overrides */
        
        virtual dir_entry* add_entry(dir_entry ent);

        virtual paths::string subpath() const {
            return m_subpath;
        }
        virtual void subpath(paths::string path) {
            m_subpath = std::move(path);
        }

        virtual dir_map const * subpath_dir(const paths::string &path) const;
        
        virtual bool is_subdir(const dir_entry &ent) const;

        virtual bool at_basedir() const {
            return m_subpath.empty();
        }

        virtual dir_entry *get_entry(const paths::string &name);
        virtual entry_range get_entries(const paths::string &name);
	};
}

#endif // NUC_ARCHIVE_TREE_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
