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

#include "file_instream.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "error_macros.h"

using namespace nuc;

file_instream::file_instream(const char *path, size_t buf_size) : buf_size(buf_size) {
    alloc_buf();

    TRY_OP((fd = ::open(path, O_CLOEXEC | O_RDONLY)) < 0)
}

file_instream::file_instream(int dirfd, const char *file, size_t buf_size) : buf_size(buf_size) {
    alloc_buf();

    TRY_OP((fd = openat(dirfd, file, O_CLOEXEC | O_RDONLY)) < 0)
}

void file_instream::alloc_buf() {
    if (buf_size)
        buf = new byte[buf_size];
}


file_instream::~file_instream() {
    close();

    if (buf) delete [] buf;
}

void file_instream::close() {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

size_t file_instream::read(byte *buf, size_t n) {
    size_t total_read = 0;
    ssize_t n_read = 0;

    do {
        try_op([&] {
            n_read = ::read(fd, buf, n);

            if (n_read < 0)
                raise_error(errno);

            n -= n_read;
            buf += n_read;

            total_read += n_read;
        });

    } while (n_read && n);

    return total_read;
}

const instream::byte *file_instream::read_block(size_t &size, off_t &offset) {
    size = read(buf, buf_size);
    offset = 0;

    return size ? buf : nullptr;
}
