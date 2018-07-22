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
         * Stat attributes of the underlying file.
         */
        struct stat m_attr;

    public:
        /**
         * Entry type constants.
         */
        enum entry_type {
            /* POSIX file types */
            type_unknown = 0,
            type_fifo,
            type_chr,
            type_dir,
            type_blk,
            type_reg,
            type_lnk,
            type_sock,
            type_wht,

            /* Pseudo entry types */
            type_parent
        };

        /**
         * Stores context data. This field is not used by the vfs
         * class.
         */
        dir_entry_context context;


        /**
         * Constructs a 'dir_entry' object with a given name and type.
         *
         * orig_subpath: The non-canonicalized subpath.
         * type:         The type of the entry, as a dirent (DT_) constant.
         */
        dir_entry(const paths::string orig_subpath, uint8_t type);
        /**
         * Constructs a 'dir_entry' object with a given name and type.
         *
         * orig_subpath: The non-canonicalized subpath.
         * type:         The type of the entry, as an entry_type constant.
         */
        dir_entry(const paths::string orig_subpath, entry_type type);
        
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
        entry_type ent_type() const;
        /**
         * Sets the entry type. The stat mode attribute is not
         * changed.
         */
        void ent_type(entry_type type);
        void ent_type(uint8_t type);

        /**
         * Returns the type of the underlying file as an entry_type
         * constant. If the type of the underlying file is unknown,
         * the entry type is returned instead.
         */
        entry_type type() const;
        
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

    private:

        /**
         * The type of the entry itself, not the underlying file.
         *
         * This is useful to distinguish symbolic links from regular
         * files whilst still storing the file type of their targets.
         */
        entry_type m_type;

        /**
         * Converts a POSIX dirent (DT_) type constant to an entry_type
         * constant.
         *
         * @param type The dirent type constant.
         *
         * @return The entry_type constant.
         */
        static entry_type dt_to_entry_type(uint8_t type);
    };
}

#endif // NUC_DIR_ENTRY_H

// Local Variables:
// mode: c++
// End:
