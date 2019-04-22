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

#ifndef NUC_DIRECTORY_DIR_ENTRY_H
#define NUC_DIRECTORY_DIR_ENTRY_H

#include "types.h"
#include "lister/lister.h"

#include "paths/pathname.h"
#include "paths/pathname.h"

namespace nuc {
    /**
     * Directory Entry.
     *
     * Stores information about entries in a directory tree, such as
     * the file name, the subpath (within the tree), and stat
     * attributes.
     */
    class dir_entry {
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
         * @param orig_subpath The non-canonicalized subpath.
         * @param type The type of the entry, as a dirent (DT_) constant.
         */
        dir_entry(const pathname orig_subpath, uint8_t type);
        /**
         * Constructs a 'dir_entry' object with a given name and type.
         *
         * @param orig_subpath The non-canonicalized subpath.
         * @param type The type of the entry, as an entry_type constant.
         */
        dir_entry(const pathname orig_subpath, entry_type type);

        /**
         * Constructs a 'dir_entry' object from a 'lister::entry'
         * object.
         *
         * @param ent The lister::entry object.
         */
        dir_entry(const lister::entry &ent);

        /**
         * Constructs a 'dir_entry' object from a 'lister::entry' object
         * with given stat attributes.
         *
         * @param ent The lister::entry object.
         * @param st The stat attributes
         */
        dir_entry(const lister::entry &ent, const struct stat &st);

        /**
         * Constructs a 'dir_entry' object with a given path and stat
         * attributes.
         *
         * @param path The original pathname of the entry.
         * @param st The stat attributes
         */
        dir_entry(pathname path, const struct stat &st);


        /**
         * Returns the original non-canonicalized subpath.
         *
         * @return pathname
         */
        const pathname &orig_subpath() const;

        /**
         * Changes the original sub-path of the entry and updates the
         * canonicalized subpath and file name.
         *
         * @param path The new path.
         */
        void orig_subpath(pathname path);


        /**
         * Returns the canonicalized subpath.
         *
         * @return pathname
         */
        const pathname &subpath() const;

        /**
         * Returns the file name component of the canonicalized
         * subpath.
         *
         * @return string
         */
        const pathname::string &file_name() const;


        /**
         * Returns the entry type.
         *
         * @return entry_type constant
         */
        entry_type ent_type() const noexcept;

        /**
         * Sets the entry type. The stat mode attribute is not
         * changed.
         *
         * @param type The new entry type.
         */
        void ent_type(entry_type type) noexcept;
        void ent_type(uint8_t type) noexcept;

        /**
         * Returns the type of the underlying file as an entry_type
         * constant. If the type of the underlying file is unknown,
         * the entry type is returned instead.
         *
         * @return entry_type constant.
         */
        entry_type type() const noexcept;


        /**
         * Returns the stat attributes.
         *
         * @return Reference to stat struct
         */
        const struct stat &attr() const;

        /**
         * Sets the stat attributes.
         *
         * @param st The new stat attribute struct.
         */
        void attr(const struct stat &st);

    private:
        /**
         * Original non-canonicalized subpath of the entry.
         */
        pathname m_orig_subpath;
        /**
         * Canonicalized subpath.
         */
        pathname m_subpath;
        /**
         * The file name of the entry, i.e. the basename of the
         * canonicalized subpath of the entry.
         */
        pathname::string m_file_name;

        /**
         * Stat attributes of the underlying file.
         */
        struct stat m_attr;

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
        static entry_type dt_to_entry_type(uint8_t type) noexcept;

        /**
         * Sets the canonicalized subpath, and file name.
         *
         * @param path The canonicalized path.
         */
        void subpath(const pathname &path);
    };
}

#endif // NUC_DIRECTORY_DIR_ENTRY_H

// Local Variables:
// mode: c++
// End:
