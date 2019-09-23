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

#include "fsutil.h"
#include "outstream.h"

namespace nuc {
    /**
     * Regular File Output Stream.
     */
    class file_outstream : public outstream {
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
         * Create a file output stream with an open file
         * descriptor. Data is written directly to the file descriptor
         * and the file descriptor is closed when the object is
         * destroyed.
         *
         * @param fd The file descriptor.
         */
        file_outstream(int fd) : fd(fd) {};

        /**
         * Closes the output stream.
         */
        ~file_outstream() {
            close_fd();
        }

        /* Method overrides */

        virtual void close();

        virtual void write(const byte *buf, size_t n, off_t offset = -1);

        /**
         * Returns the file descriptor of the file.
         *
         * @return The file descriptor.
         */
        int get_fd() const {
            return fd;
        }


        /* Modification and Access Times */

        /**
         * Sets the access and modification times which will be set
         * when the stream is closed.
         *
         * @param st Stat structure containing access and modification
         *    time.
         */
        void times(const struct stat *st);

        /**
         * Sets the access and modification times which will be set
         * when the stream is closed.
         *
         * @param atime Access time.
         * @param mtime Last modified time.
         */
        void times(fs::time_type atime, fs::time_type mtime);

    private:
        /**
         * Path to the file being written to.
         */
        std::string path;

        /**
         * File descriptor.
         */
        int fd;

        /**
         * Flag: true if the access and modification times should be
         * set when the stream is closed.
         */
        bool set_times = false;

        /**
         * Last modified time to set when stream is closed.
         */
        fs::time_type mtime;
        /**
         * Last accessed time to set when stream is closed.
         */
        fs::time_type atime;

        /**
         * Seeks to the position @a offset bytes from the current
         * position in the file.
         *
         * @param offset The offset, to seek to, from the current file
         *   position.
         */
        void seek(off_t offset);

        /**
         * Closes the file descriptor.
         *
         * @return 0 if successful, non-zero on error.
         */
        int close_fd();

        /**
         * Sets the access and last modified times, to 'atime' and
         * 'mtime'.
         */
        void update_times();

    protected:
        void raise_error(int code, error::type_code type, bool can_retry = true) {
            throw file_error(code, type, can_retry, path);
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
