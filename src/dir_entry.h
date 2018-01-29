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

#ifndef NUC_DIR_ENTRY_H
#define NUC_DIR_ENTRY_H

#include "types.h"
#include "lister.h"

#include "path_utils.h"

namespace nuc {
    /**
     * Directory Entry.
     * 
     * Stores information about entries in a directory tree, such as
     * the file name, the subpath (within the tree), and stat
     * attributes.
     * 
     * When the directory tree is set to parse the directory structure
     * from a flat list of file paths, a map of the child entries of
     * the directory, if the entry is of type directory, is stored.
     */
    class dir_entry {
        /**
         * The file name of the entry, i.e. the basename of the
         * canonicalized subpath of the entry.
         */
        path_str m_file_name;
        /**
         * Original non-canonicalized subpath of the entry.
         */
        path_str m_orig_subpath;
        /**
         * Canonicalized subpath.
         */
        path_str m_subpath;
        
        /**
         * The type of the entry itself, not the underlying file, as a
         * POSIX 'dirent' constant.
         * 
         * This is useful to distinguish symbolic links from regular
         * files whilst still storing the file type of their targets.
         */
        file_type m_type;
        
        /**
         * Stat attributes of the underlying file.
         */
        struct stat m_attr;
        
        /**
         * Map of child entries, used when the directory tree stores
         * the directory structure, and the entry is of type
         * directory.
         */
        file_map<dir_entry *> child_map;
        
    public:
        /**
         * Constructs a 'dir_entry' object with a given name and type,
         * which is stored in the entry type and 'mode' stat
         * attribute.
         *
         * orig_subpath: The non-canonicalized subpath.
         * type:         The type of the entry, as a dirent constant.
         */
        dir_entry(const path_str orig_subpath, uint8_t type);
        
        /**
         * Constructs a 'dir_entry' object from a 'lister' object.
         */
        dir_entry(const lister::entry &ent);
        
        /**
         * Constructs a 'dir_entry' object from 'lister::entry' object
         * with given stat attributes.
         */
        dir_entry(const lister::entry &ent, const struct stat &st);
        
        /**
         * Returns the original non-canonicalized subpath.
         */
        const path_str &orig_subpath() const {
            return m_orig_subpath;
        }

        /**
         * Returns the canonicalized subpath.
         */
        const path_str &subpath() const {
            return m_subpath;
        }
        /**
         * Sets the canonicalized subpath, and file name.
         */
        void subpath(const path_str &str) {
            m_subpath = str;
            m_file_name = nuc::file_name(str);
        }
        
        /**
         * Returns the file name component of the canonicalized
         * subpath.
         */
        const path_str &file_name() const {
            return m_file_name;
        }
        
        /**
         * Returns the entry type.
         */
        file_type ent_type() const {
            return m_type;
        }
        /**
         * Sets the entry type. The stat mode attribute is not
         * changed.
         */
        void ent_type(uint8_t type) {
            m_type = type;
        }
        
        /**
         * Returns the stat attributes.
         */
        const struct stat &attr() const {
            return m_attr;
        }
        
        /**
         * Sets the stat attributes.
         */
        void attr(const struct stat &st) {
            m_attr = st;
        }
        
        /**
         * Returns a reference to the child map.
         */
        file_map<dir_entry *> & child_ents() {
            return child_map;
        }
        const file_map<dir_entry *> &child_ents() const {
            return child_map;
        }
    };
}

#endif // NUC_DIR_ENTRY_H