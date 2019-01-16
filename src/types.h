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

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <unordered_map>
#include <functional>

#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtkmm/liststore.h>

#include "paths/pathname.h"

/**
 * Contains type aliases used throughout the project.
 */

namespace nuc {
    /**
     * File map type.
     */
    template <typename T>
    using file_map = std::unordered_multimap<paths::string, T>;

    /**
     * Context data type for the dir_entry class.
     */
    struct dir_entry_context {
        Gtk::TreeRow row;
    };

    /**
     * File Identifier.
     *
     * Uniquely identifies a file by inode and device id.
     */
    struct file_id {
        /**
         * Device id.
         */
        const dev_t dev = 0;
        /**
         * Inode.
         */
        const ino_t ino = 0;

        /** Constructors */

        file_id() {}
        file_id(dev_t dev, ino_t ino) : dev(dev), ino(ino) {}

        file_id(const struct stat &st) : dev(st.st_dev), ino(st.st_ino) {}

        /**
         * Checks whether the file_id is a valid file identifier.
         *
         * @return True if the file_id is a valid file identifier.
         */
        operator bool() const {
            return dev != 0 && ino != 0;
        }
    };

    /* file_id equality comparison operators. */

    inline bool operator==(const file_id &id1, const file_id &id2) {
        return id1.dev == id2.dev && id1.ino == id2.ino;
    }
    inline bool operator!=(const file_id &id1, const file_id &id2) {
        return !(id1 == id2);
    }
}

/* file_id hash function */
namespace std {
    template <> struct hash<nuc::file_id> {
        size_t operator()(const nuc::file_id &fid) const noexcept {
            hash<size_t> h{};
            return h((fid.dev * 2654435761U) ^ fid.ino);
        }
    };
}

#endif // TYPES_H

// Local Variables:
// mode: c++
// End:
