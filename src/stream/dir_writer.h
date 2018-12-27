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

#include "paths/pathname.h"

#include "errors/errors.h"
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
        virtual outstream *create(const paths::pathname &path, const struct stat *st = nullptr, int flags = 0) = 0;

        /**
         * Creates a directory, with read, write and search
         * permissions to allow files to be created within the
         * directory.
         *
         * @param path Subpath of the directory to create, relative to
         *    the directory of the dir_writer object.
         *
         * @param defer If true the creation of the directory may be
         *   deferred until a file is created in it or its attributes
         *   are set. 
         */
        virtual void mkdir(const paths::pathname &path, bool defer = true) = 0;


        /**
         * Creates a symbolic link.
         *
         * @param path Subpath where the link is to be created.
         * @param target The target of the link.
         * @param st Stat attributes of the link
         */
        virtual void symlink(const paths::pathname &path, const paths::pathname &target, const struct stat *st) = 0;

        /**
         * Sets the stat attributes of an existing file, in the
         * directory.
         *
         * @param path Path to the file, relative to the directory.
         * @param st   The stat attributes to set.
         */
        virtual void set_attributes(const paths::pathname &path, const struct stat *st) = 0;

        /**
         * Rename the file at path @a src to @a dest.
         *
         * @param src Path to the file to rename.
         * @param dest Destination path to rename the file to.
         */
        virtual void rename(const paths::pathname &src, const paths::pathname &dest) = 0;

        /**
         * Delete the file at path @a path.
         *
         * @param path to the file to delete.
         *
         * @param relative If true @a path is interpreted relative to
         *   the subpath of the writer otherwise it is interpreted
         *   relative to the base path of the writer.
         */
        virtual void remove(const paths::pathname &path, bool relative = true) = 0;

    protected:
        /**
         * Throws an error exception.
         *
         * @param code Error code.
         *
         * @param can_retry True if the operation can be retried,
         *    false otherwise.
         */
        void raise_error(int code, bool can_retry = true) {
            throw error(code, can_retry);
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
