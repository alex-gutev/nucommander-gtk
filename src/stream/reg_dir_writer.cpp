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
#include <stdio.h>

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

outstream * reg_dir_writer::create(const paths::pathname &path, const struct stat *st, int flags) {
    file_outstream *stream = new file_outstream(fd, path.path().c_str(), flags);

    set_file_attributes(stream->get_fd(), path.path().c_str(), st);

    return stream;
}

void reg_dir_writer::mkdir(const paths::pathname &path, bool) {
    TRY_OP_(mkdirat(fd, path.path().c_str(), S_IRWXU),
            throw file_error(errno, error::type_create_dir, true, path))
}

void reg_dir_writer::symlink(const paths::pathname &path, const paths::pathname &target, const struct stat *st) {
    TRY_OP(symlinkat(target.path().c_str(), fd, path.path().c_str()))

    set_attributes(path, st);
}


/// Attributes

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

void reg_dir_writer::set_attributes(const paths::pathname &path, const struct stat *st) {
    if (st) {
        if (!S_ISLNK(st->st_mode))
            with_skip_attrib([=] {
                TRY_OP_(fchmodat(fd, path.path().c_str(), st->st_mode & ~S_IFMT, 0),
                        throw attribute_error(errno, error::type_set_mode, true, path));
            });

        struct timespec times[] = { st->st_atim, st->st_mtim };

        with_skip_attrib([&] {
            TRY_OP_(utimensat(fd, path.path().c_str(), times, AT_SYMLINK_NOFOLLOW),
                    throw attribute_error(errno, error::type_set_times, true, path));
        });

        with_skip_attrib([=] {
            TRY_OP_(fchownat(fd, path.path().c_str(), st->st_uid, st->st_gid, AT_SYMLINK_NOFOLLOW),
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

nuc::file_id reg_dir_writer::get_file_id(const paths::pathname &path) {
    struct stat st;

    if (!fstatat(fd, path.c_str(), &st, AT_SYMLINK_NOFOLLOW)) {
        return file_id(st);
    }

    return file_id();
}

/// Renaming Files

void reg_dir_writer::rename(const paths::pathname &src, const paths::pathname &dest) {
    bool replace = false;

    global_restart(restart("replace", [&] (const error &e, boost::any) {
        replace = true;
    }, [&] (const error &e) {
        return e.code() == EEXIST;
    }));

    try_op([&] {
        // Check if file exists
        if (!replace && !faccessat(fd, dest.path().c_str(), F_OK, AT_SYMLINK_NOFOLLOW)) {
            throw file_error(EEXIST, error::type_rename_file, true, dest);
        }

        if (renameat(fd, src.path().c_str(), fd, dest.path().c_str())) {
            throw file_error(errno, error::type_rename_file, true, dest);
        }
    });
}


/// Deleting Files

void reg_dir_writer::remove(const paths::pathname &path, bool relative) {
    try_op([=] {
        if (unlinkat(fd, path.path().c_str(), AT_REMOVEDIR)) {
            int err = errno;

            if (err == ENOTDIR) {
                if (unlinkat(fd, path.path().c_str(), 0))
                    throw file_error(errno, error::type_delete_file, true, path);
            }
            else {
                throw file_error(err, error::type_delete_file, true, path);
            }
        }
    });
}
