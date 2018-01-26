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
#include "lister.h"

#include "dir_entry.h"

namespace nuc {
    /**
     * Creates a virtual directory tree abstraction out of
     * a flat list of files, such as the file list obtained
     * from an archive.
     * 
     * The directory structure is parsed from the file paths.
     */    
    class dir_tree {
        /**
         * Map of all entries in the directory tree.
         * 
         * A multi-map is used since certain virtual
         * file systems, such as archives, may contain
         * multiple files with the same name.
         */
        file_map<dir_entry> map;
        
        /**
         * If true, the directory structure is parsed from
         * the file paths. If false, the flat list of files
         * is simply stored without parsing the directory
         * structure.
         */        
        bool m_parse_dirs = false;
        
        /**
         * Map of the entries in the base directory. Used only when
         * 'm_parse_dirs' is true.
         */        
        file_map<dir_entry *> m_base_dir;
        
        /**
         * Returns a canonicalized version of the original entry name
         * in order for it to be used as the map key. All non-leading
         * '.' and '..' path components are removed.
         */
        path_str ent_name_key(const path_str &orig_name);

        /**
         * Returns only the file name component of the entry's
         * path.
         * 
         * key: The canonicalized path, as returned by 'ent_name_key'.
         */
        path_str file_name(path_str key);
        
        /**
         * Extracts and creates the intermediate directory components
         * of an entry.
         *
         * path:    The path to the entry, as returned by ent_name_key.
         * ent:     Pointer to the entry.
         * update:  If true, each component is added to the updated
         *            child map of its parent entry.
         */
        void add_components(const path_str &path, dir_entry &ent);
        
        /**
         * Searches for a directory entry with sub-path 'path', in
         * the map. If the entry is found and it is a directory entry
         * it is returned otherwise, a new directory entry is created
         * with display name 'name'.
         * 
         * path: The sub-path of the entry, within the directory tree.
         * name: The display name of the entry to create.
         */
        dir_entry &make_dir_ent(const path_str &path, const path_str &name);
       
        /**
         * Adds an entry to a multi-map if the map does not already contain
         * that entry, this is determined by checking whether any of the
         * existing entries in the map, with the same key, are equal to 
         * the entry (currently by pointer comparison).
         */
        static void add_to_map(file_map<dir_entry *> &map, const path_str &name, dir_entry *ent);
        
    public:
        /**
         * Removes all entries in the tree.
         */
        void clear() {
            map.clear();
            m_base_dir.clear();
        }
        
        /**
         * Swaps two directory trees.
         */
        void swap(dir_tree &fs) {
            map.swap(fs.map);
            m_base_dir.swap(fs.m_base_dir);
        }
        
        /**
         * Returns true if the directory structure
         * is parsed from the file paths, false
         * if only the flat list of files is stored.
         */
        bool parse_dirs() const {
            return m_parse_dirs;
        }

        /**
         * Changes to either a directory tree or
         * flat list.
         * 
         * If 'parse_dirs' is true the directory
         * structure is parsed from the file paths.
         * 
         * If 'parse_dirs' is false, no directory structure
         * is stored, only the flat list of files is stored.
         */
        void parse_dirs(bool parse_dirs) {
            m_parse_dirs = parse_dirs;
        }
        
        /**
         * Creates and adds a new entry to the tree, from the
         * information returned by a lister::entry object.
         *
         * update:  If true, the updated entry attributes are set.
         * ent:     The entry object returned by the lister object.
         * st:      The stat information returned by the lister object.
         */
        dir_entry *add_entry(const lister::entry &ent, const struct stat &st);
        
        /**
         * Returns the pointer to the first entry with name 'name',
         * 'nullptr' if the entry was not found in the tree.
         */
        dir_entry *get_entry(const path_str &name) {
            auto it = map.find(name);

            if (it != map.end()) {
                return &it->second;
            }

            return nullptr;
        }
        
        auto get_entries(const path_str &name) -> decltype(map.equal_range(name)) {
            return map.equal_range(name);
        }

        /**
         * Returns a reference to the map of the entries in the base
         * directory.  This should only be used to iterate over the
         * base directory using begin() and end().
         */
        auto base_dir() -> decltype(m_base_dir) & {
            return m_base_dir;
        }

        /** Iterator to the first entry in the directory tree. */
        auto begin() -> decltype(map.begin()) {
            return map.begin();
        }

        /** Past the end iterator. */
        auto end() -> decltype(map.end()) {
            return map.end();
        }

    };
}

#endif // NUC_DIR_TREE_H
