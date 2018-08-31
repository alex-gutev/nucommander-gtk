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

#include "reg_dir_writer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "file_outstream.h"


using namespace nuc;

reg_dir_writer::reg_dir_writer(const char *path) {
    if ((fd = open(path, O_DIRECTORY)) < 0)
        throw error(errno);
}

void reg_dir_writer::close() {
    ::close(fd);
}

outstream * reg_dir_writer::create(const char *path, const struct stat *st, int flags) {
    file_outstream *stream = new file_outstream(fd, path, flags);

    set_file_attributes(stream->get_fd(), st);

    return stream;
}

void reg_dir_writer::mkdir(const char *path) {
    if (mkdirat(fd, path, S_IRWXU))
        throw error(errno);
}

void reg_dir_writer::symlink(const char *path, const char *target, const struct stat *st) {
    if (symlinkat(path, fd, target))
        throw error(errno);

    set_attributes(path, st);
}


/// Setting Attributes

void reg_dir_writer::set_file_attributes(int fd, const struct stat *st) {
    // Issue: No error reporting yet

    if (st) {
        fchmod(fd, st->st_mode & ~S_IFMT);
        fchown(fd, st->st_uid, st->st_gid);

        struct timespec times[] = { st->st_atim, st->st_mtim };
        futimens(fd, times);
    }
}

void reg_dir_writer::set_attributes(const char *path, const struct stat *st) {
    // Issue: No error reporting yet

    if (st) {
        if (!S_ISLNK(st->st_mode))
            fchmodat(fd, path, st->st_mode & ~S_IFMT, 0);

        fchownat(fd, path, st->st_uid, st->st_gid, AT_SYMLINK_NOFOLLOW);

        struct timespec times[] = { st->st_atim, st->st_mtim };
        utimensat(fd, path, times, AT_SYMLINK_NOFOLLOW);
    }
}