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

#ifndef NUC_FILE_INSTREAM_H
#define NUC_FILE_INSTREAM_H

#include "instream.h"

#include "paths/utils.h"

namespace nuc {
    /**
     * Regular File Input Stream.
     */
    class file_instream : public instream {
        /**
         * File Descriptor.
         */
        int fd = -1;

        /**
         * Path to the file being read.
         */
        paths::string path;

        /**
         * Default buffer size (size of the blocks).
         */
        static constexpr size_t default_buf_size = 131072;

        /**
         * Size of allocated buffer;
         */
        size_t buf_size = default_buf_size;

        /**
         * Buffer into which the block is read.
         */
        byte * buf = nullptr;

        /**
         * Allocates the buffer.
         */
        void alloc_buf();

        /**
         * Reads @a n bytes into the buffer.
         *
         * @param buf The buffer into which to read the data.
         * @param n   Number of bytes to read.
         *
         * @return The number of bytes actually read. If this is less
         *    than @a n then the end of file was reached.
         */
        size_t read(byte *buf, size_t n);

    public:

        /**
         * Creates an input stream for the file @a path.
         *
         * Throws an exception if the file could not be opened for
         * reading.
         *
         * @param path Path to the file.
         * @param buf_size Block buffer size to use.
         */
        file_instream(const char *path, size_t buf_size = default_buf_size);
        /**
         * Creates an input stream for the file at @a path, which is
         * relative to the directory with file descriptor @a dirfd.
         *
         * @param dirfd File descriptor of the directory to which @a
         *    path is relative.
         *
         * @param path Path to the file, relative to the directory.
         *
         * @param buf_size Block buffer size to use.
         */
        file_instream(int dirfd, const char *path, size_t buf_size = default_buf_size);

        /**
         * Closes the stream.
         */
        virtual ~file_instream();

        /* Method Overrides */

        virtual void close();

        virtual const byte *read_block(size_t &size, off_t &offset);

    protected:
        void raise_error(int code, bool can_retry = true) {
            throw file_error(code, error::type_read_file, can_retry, path);
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
