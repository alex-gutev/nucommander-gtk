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

#ifndef NUC_DIR_TREE_H
#define NUC_DIR_TREE_H

#include "types.h"
#include "lister/lister.h"
#include "dir_entry.h"

namespace nuc {
    /**
     * Interface for creating a directory tree abstraction out of a
     * flat list of files.
     *
     * This class (the base class) provides the generic interface
     * however does not provide any directory tree abstraction, only a
     * file index is stored to be able to quickly query whether a
     * certain file exists in the tree. Actual directory abstractions
     * should be provided by subclasses, this class is intended to be
     * used only for regular directories where the entire list of
     * files belongs to the base directory.
     */
    class dir_tree {
    protected:
        /**
         * File index stored as an unordered multi-map where each key
         * is the file subpath and the corresponding value is the
         * 'dir_entry' object.
         *
         * A multi-map is used since certain virtual file systems,
         * such as archives, may contain multiple files with the same
         * name.
         */
        file_map<dir_entry> map;
        
    public:

        /**
         * Iterator range type. A pair of iterators is returned by
         * get_entries, which corresponds to all entries with a
         * particular name.
         */
        typedef std::pair<file_map<dir_entry>::iterator, file_map<dir_entry>::iterator> entry_range;
        
        /**
         * Directory map type.
         *
         * A directory map is an unordered multi-map map of the
         * entries in a sub-directory of the directory tree. The key
         * is the entry file name (not the sub-path) and the value is
         * a pointer to the 'dir_entry' object.
         */
        typedef file_map<dir_entry*> dir_map;

        /**
         * Destructor.
         *
         * Virtual as the class is intended to be inherited from and
         * used polymorphically.
         */
        virtual ~dir_tree() = default;

        /**
         * Adds an entry to the tree. The 'dir_entry' object is
         * created from the information in the 'lister::entry' and
         * 'stat' objects returned by the lister object.
         *
         * @param ent The 'lister::entry' object returned by the lister.
         * @param st  The 'stat' attributes of the entry
         *
         * @return A pointer to the 'dir_entry' object is returned if
         *    the entry is a direct child of the tree's current
         *    subdirectory, otherwise nullptr is returned.
         */
        virtual dir_entry* add_entry(const lister::entry &ent, const struct stat &st);

        /**
         * Adds an entry to the tree, which is a copy of an existing
         * entry.
         *
         * @param ent The entry to add to the tree.
         *
         * @return A pointer to the 'dir_entry' object is returned if
         *    the entry is a direct child of the tree's current
         *    subdirectory, otherwise nullptr is returned.
         */
        virtual dir_entry* add_entry(dir_entry ent);

        /**
         * Returns the current subdirectory of the tree. The empty
         * string indicates the base directory.
         *
         * @return The current subdirectory.
         */
        virtual paths::string subpath() const {
            // The empty string cannot be a valid directory name.
            return "";
        }
        
        /**
         * Sets the tree's subdirectory, without checking whether it
         * exists.
         *
         * @param path The new subpath.
         */
        virtual void subpath(paths::string path) {}

        /**
         * Returns the contents of a subdirectory, if it exists.
         *
         * @param path The subpath of the directory.
         *
         * @return If the subdirectory @a path exists in the tree, a
         *    pointer to the directory map of its contents is
         *    returned, otherwise nullptr is returned, if there is no
         *    such subdirectory.
         */
        virtual dir_map const * subpath_dir(const paths::string &path) const {
            return nullptr;
        }

        /**
         * @return True if the entry @a ent is a subdirectory of the
         *    tree.
         */
        virtual bool is_subdir(const dir_entry &ent) const {
            return false;
        }

        /**
         * @return True if the directory tree is at the base
         *    directory.
         */
        virtual bool at_basedir() const {
            return true;
        }
        
        /**
         * Retrieves the first entry with name @name in the current
         * subdirectory.
         *
         * @param name The name to the entry.
         *
         * @return Pointer to the entry, 'nullptr' if the entry was
         *    not found.
         */
        virtual dir_entry *get_entry(const paths::string &name) {
            auto it = map.find(name);

            if (it != map.end()) {
                return &it->second;
            }

            return nullptr;
        }

        /**
         * Returns all entries with name @name in the current
         * subdirectory.
         *
         * @param name The name of the entries.
         *
         * @return A pair of iterators where the first iterator is the
         *   iterator to the first entry and the second iterator is
         *   the past the end iterator.
         */
        virtual entry_range get_entries(const paths::string &name) {
            return map.equal_range(name);
        }

        /**
         * @return A reference to the map (index) containing all
         *    entries in the directory tree.
         */
        auto index() -> decltype(map) & {
            return map;
        }

        /** 
         * @return Iterator to the first entry in the directory tree.
         */
        auto begin() -> decltype(map.begin()) {
            return map.begin();
        }

        /**
         * @return Past-the-end iterator. 
         */
        auto end() -> decltype(map.end()) {
            return map.end();
        }
    };
}

#endif // NUC_DIR_TREE_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
