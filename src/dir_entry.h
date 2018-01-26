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

#include <functional>

#include "types.h"
#include "lister.h"

namespace nuc {
    /**
     * Directory Entry Class.
     * 
     * Stores information about entries in a directory tree,
     * such as the file name, the sub-path (within the tree),
     * and stat attributes.
     * 
     * When the directory tree is set to parse the directory
     * structure from the file paths, the class also stores
     * a map of the child entries of the directory, if the
     * entry is of type directory.
     */
    class dir_entry {
        /**
         * The file name which is displayed.
         */
        path_str m_display_name;
        /**
         * The original non-canonicalized name of the file.
         */
        path_str m_orig_name;
        /**
         * File extension.
         */
        path_str m_ext;
        
        /**
         * The type of the entry itself, not the underlying
         * file, as a POSIX 'dirent' constant.
         * 
         * This is useful to distinguish symbolic links
         * from regular files whilst still storing the
         * file type of their targets.
         */
        file_type m_type;
        
        /**
         * Stat attributes of the underlying file.
         */
        struct stat m_attr;
        
        /**
         * Map of child entries, used when the directory tree
         * is set to store the directory structure, and the file
         * is of type directory. Otherwise it is not used.
         */
        file_map<dir_entry *> child_map;
        
    public:
        /**
         * Constructs a 'dir_entry' object with a given name and
         * type, which is stored in the entry type and stat attributes.
         */
        dir_entry(const path_str orig_name, uint8_t type);
        
        /**
         * Constructs a 'dir_entry' object from a 'lister' object.
         */
        dir_entry(const lister::entry &ent);
        
        /**
         * Constructs a 'dir_entry' object from the details stored int
         * the 'lister::entry' object (returned by the 'lister' object)
         * and stat attributes
         */
        dir_entry(const lister::entry &ent, const struct stat &st);
        
        /**
         * Returns the original name.
         */
        const path_str &orig_name() const {
            return m_orig_name;
        }
        /**
         * Returns the extension.
         */
        const path_str &extension() const {
            return m_ext;
        }
        
        /**
         * Returns the display name.
         */
        const path_str &display_name() const {
            return m_display_name;
        }
        /**
         * Sets the display name and the extension, which
         * is extracted from the display name.
         */
        void display_name(const path_str &name);
        
        /**
         * Returns the entry type.
         */
        file_type ent_type() const {
            return m_type;
        }
        /**
         * Sets the entry type, the stat attributes
         * are not changed.
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
