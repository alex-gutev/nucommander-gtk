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

#include <exception>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "file_outstream.h"

#include "error_macros.h"

using namespace nuc;

class skip_attribute : public std::exception {};

template <typename F>
void with_skip_attrib(F op);


reg_dir_writer::reg_dir_writer(const char *path) {
    TRY_OP((fd = open(path, O_DIRECTORY)) < 0)
}

void reg_dir_writer::close() {
    ::close(fd);
}

outstream * reg_dir_writer::create(const char *path, const struct stat *st, int flags) {
    file_outstream *stream = new file_outstream(fd, path, flags);

    set_file_attributes(stream->get_fd(), path, st);

    return stream;
}

void reg_dir_writer::mkdir(const char *path) {
    TRY_OP_(mkdirat(fd, path, S_IRWXU),
            throw file_error(errno, error::type_create_dir, path))
}

void reg_dir_writer::symlink(const char *path, const char *target, const struct stat *st) {
    TRY_OP(symlinkat(target, fd, path))

    set_attributes(path, st);
}


/// Setting Attributes

void reg_dir_writer::set_file_attributes(int fd, const char *path, const struct stat *st) {
    if (st) {
        with_skip_attrib([=] {
            TRY_OP_(fchmod(fd, st->st_mode & ~S_IFMT),
                    throw attribute_error(errno, error::type_set_mode, true, path));
        });

        struct timespec times[] = { st->st_atim, st->st_mtim };

        with_skip_attrib([&] {
            TRY_OP_(futimens(fd, times),
                    throw attribute_error(errno, error::type_set_times, true, path));
        });

        with_skip_attrib([=] {
            TRY_OP_(fchown(fd, st->st_uid, st->st_gid),
                    throw attribute_error(errno, error::type_set_owner, true, path));
        });
    }
}

void reg_dir_writer::set_attributes(const char *path, const struct stat *st) {
    if (st) {
        if (!S_ISLNK(st->st_mode))
            with_skip_attrib([=] {
                TRY_OP_(fchmodat(fd, path, st->st_mode & ~S_IFMT, 0),
                        throw attribute_error(errno, error::type_set_mode, true, path));
            });

        struct timespec times[] = { st->st_atim, st->st_mtim };

        with_skip_attrib([&] {
            TRY_OP_(utimensat(fd, path, times, AT_SYMLINK_NOFOLLOW),
                    throw attribute_error(errno, error::type_set_times, true, path));
        });

        with_skip_attrib([=] {
            TRY_OP_(fchownat(fd, path, st->st_uid, st->st_gid, AT_SYMLINK_NOFOLLOW),
                    throw attribute_error(errno, error::type_set_owner, true, path));
        });
    }
}

template <typename F>
void with_skip_attrib(F op) {
    global_restart skip(restart("skip attribute", [] (const error &, boost::any) {
        throw skip_attribute();
    }));

    try {
        op();
    }
    catch (const skip_attribute &) {
    }
}
