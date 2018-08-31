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

#ifndef NUC_FILE_OUTSTREAM_H
#define NUC_FILE_OUTSTREAM_H

#include <sys/types.h>
#include <sys/stat.h>

#include "outstream.h"

namespace nuc {
    /**
     * Regular File Output Stream.
     */
    class file_outstream : public outstream {
        /**
         * File descriptor.
         */
        int fd;

        /**
         * Seeks to the position @a offset bytes from the current
         * position in the file.
         *
         * @param offset The offset, to seek to, from the current file
         *   position.
         */
        void seek(off_t offset);

    public:
        /**
         * Constructs a file output stream for the file at @a path.
         *
         * @param path Path to the file.
         * @param flags Additional flags passed to open(2).
         */
        file_outstream(const char *path, int flags, int perms = S_IRWXU);
        /**
         * Constructs a file output stream for the file at path @a
         * path relative to the directory with file descriptor @a
         * dirfd.
         *
         * @param dirfd File descriptor of the directory.
         *
         * @param path Path to the file, if relative interpreted as
         *    relative to the directory with file descriptor dirfd.
         *
         * @param flags Additional flags passed to openat(2).
         */
        file_outstream(int dirfd, const char *path, int flags, int perms = S_IRWXU);

        /**
         * Closes the output stream.
         */
        ~file_outstream() {
            close();
        }

        /* Method overrides */

        virtual bool close();

        virtual void write(const byte *buf, size_t n, off_t offset = -1);

        /**
         * Returns the file descriptor of the file.
         *
         * @return The file descriptor.
         */
        int get_fd() const {
            return fd;
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
