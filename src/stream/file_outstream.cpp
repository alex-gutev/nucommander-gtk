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

#include "file_outstream.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace nuc;

/**
 * Short-hand for calling the try_op method with a function in which
 * if the expression op returns non-zero, raise_error is called with
 * the value of errno.
 */
#define TRY_OP(type, op) try_op([&] {  \
        if ((op)) raise_error(errno, (type));   \
    });


file_outstream::file_outstream(const char *path, int flags, int perms) : path(path) {
    int excl = O_EXCL;

    global_restart overwrite(overwrite_restart(excl));

    TRY_OP(error::type_create_file, (fd = open(path, flags | O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC | excl, perms)) < 0)
}
file_outstream::file_outstream(int dirfd, const char *path, int flags, int perms) : path(path) {
    int excl = O_EXCL;

    global_restart overwrite(overwrite_restart(excl));

    TRY_OP(error::type_create_file, (fd = openat(dirfd, path, flags | O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC | excl, perms)) < 0)
}

restart file_outstream::overwrite_restart(int &excl) {
    return restart("overwrite", [&excl] (const nuc::error &e, boost::any) {
        excl = 0;
    },
    [] (const nuc::error &e) {
        return e.code() == EEXIST;
    });
}

void file_outstream::close() {
    if (close_fd())
        raise_error(errno, error::type_write_file, false);
}

int file_outstream::close_fd() {
    if (fd >= 0) {
        int err = ::close(fd);
        fd = -1;
        return err;
    }

    return 0;
}

void file_outstream::write(const byte *buf, size_t n, off_t offset) {
    seek(offset);

    while(n) {
        try_op([&] {
            ssize_t bytes_written = ::write(fd, (const char *)buf, n);

            if (bytes_written < 0) raise_error(errno, error::type_write_file);

            n -= bytes_written;
            buf += bytes_written;
        });
    };
}

void file_outstream::seek(off_t offset) {
    // Seeking past EOF results in "gaps" in the file which are filled
    // with zeroes, provided that data is written past the
    // gap. Currently it is assumed that if offset is non-zero then
    // the size of the block is non-zero.

    if (offset) {
        TRY_OP(error::type_write_file, lseek(fd, offset, SEEK_CUR))
    }
}
