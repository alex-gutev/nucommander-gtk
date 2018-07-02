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
#include "lister/lister.h"

#include "paths/utils.h"

namespace nuc {
    enum {
        /**
         * Parent directory pseudo-entry type.
         * 
         * DT_WHT has the maximum value, 1 is added to it to produce
         * a custom type constant
         */
        DT_PARENT = DT_WHT + 1
    };
    
    /**
     * Directory Entry.
     * 
     * Stores information about entries in a directory tree, such as
     * the file name, the subpath (within the tree), and stat
     * attributes.
     */
    class dir_entry {
        /**
         * Original non-canonicalized subpath of the entry.
         */
        paths::string m_orig_subpath;
        /**
         * Canonicalized subpath.
         */
        paths::string m_subpath;
        /**
         * The file name of the entry, i.e. the basename of the
         * canonicalized subpath of the entry.
         */
        paths::string m_file_name;
        
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
        
    public:

        /**
         * Stores context data. This field is not used by the vfs
         * class.
         */
        dir_entry_context context;


        /**
         * Constructs a 'dir_entry' object with a given name and type,
         * which is stored in the entry type and 'mode' stat
         * attribute.
         *
         * orig_subpath: The non-canonicalized subpath.
         * type:         The type of the entry, as a dirent constant.
         */
        dir_entry(const paths::string orig_subpath, uint8_t type);
        
        /**
         * Constructs a 'dir_entry' object from a 'lister' object.
         */
        dir_entry(const lister::entry &ent);
        
        /**
         * Constructs a 'dir_entry' object from 'lister::entry' object
         * with given stat attributes.
         */
        dir_entry(const lister::entry &ent, const struct stat &st);

        dir_entry(paths::string path, const struct stat &st);
        
        /**
         * Returns the original non-canonicalized subpath.
         */
        const paths::string &orig_subpath() const {
            return m_orig_subpath;
        }

        /**
         * Changes the original sub-path of the entry and updates the
         * canonicalized subpath and file name.
         */
        void orig_subpath(paths::string path) {
            m_orig_subpath = path;
            subpath(paths::canonicalized_path(std::move(path)));
        }

        /**
         * Returns the canonicalized subpath.
         */
        const paths::string &subpath() const {
            return m_subpath;
        }
        /**
         * Sets the canonicalized subpath, and file name.
         */
        void subpath(const paths::string &str) {
            m_subpath = str;
            m_file_name = paths::file_name(str);
        }
        
        /**
         * Returns the file name component of the canonicalized
         * subpath.
         */
        const paths::string &file_name() const {
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
         * Returns the type of the underlying file as a dirent (DT_ )
         * constant. If the type of the underlying file is unknown
         * (DT_UNKNOWN), the entry type is returned instead.
         */
        file_type type() const;
        
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
    };
}

#endif // NUC_DIR_ENTRY_H

// Local Variables:
// mode: c++
// End:
