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

#ifndef NUC_DIR_WRITER_H
#define NUC_DIR_WRITER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "outstream.h"

namespace nuc {
    /**
     * Generic Directory Writer Interface.
     *
     * Provides an interface for creating files/directories, deleting
     * files/directories and changing file attributes.
     */
    class dir_writer {
    public:
        /**
         * Error exception.
         */
        class error : public std::exception {
            int m_code;

        public:
            error(int code) : m_code(code) {}

            /**
             * Returns the error code.
             */
            int code() const {
                return m_code;
            }
        };

        virtual ~dir_writer() = default;


        /**
         * Closes the directory.
         */
        virtual void close() = 0;

        /**
         * Creates a regular file and returns an output stream for
         * writing data to the file.
         *
         * @param path Subpath of the file, relative to the directory
         *    of the dir_writer object.
         *
         * @param st Stat Attributes of the file
         *
         * @param flags Additional file creation flags.
         *
         * @return An outstream object for writing data to the file.
         */
        virtual outstream *create(const char *path, const struct stat *st = nullptr, int flags = 0) = 0;

        /**
         * Creates a directory, with read, write and search
         * permissions to allow files to be created within the
         * directory.
         *
         * @param path Subpath of the directory to create, relative to
         *    the directory of the dir_writer object.
         */
        virtual void mkdir(const char *path) = 0;


        /**
         * Creates a symbolic link.
         *
         * @param path Subpath where the link is to be created.
         * @param target The target of the link.
         * @param st Stat attributes of the link
         */
        virtual void symlink(const char *path, const char *target, const struct stat *st) = 0;

        /**
         * Sets the stat attributes of an existing file, in the
         * directory.
         *
         * @param path Path to the file, relative to the directory.
         * @param st   The stat attributes to set.
         */
        virtual void set_attributes(const char *path, const struct stat *st) = 0;
    };
}

#endif

// Local Variables:
// mode: c++
// End:
