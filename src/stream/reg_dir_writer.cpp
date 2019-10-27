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

#include "fsutil.h"
#include "file_outstream.h"

#include "error_macros.h"
#include "errors/restarts.h"

using namespace nuc;

/**
 * Creates the "overwrite" restart.
 *
 * @param flags Reference to a variable which holds the flags passed
 *   to open. When the restart is invoked, the O_EXCL flag is cleared.
 *
 * @return The restart.
 */
static restart overwrite_restart(int &flags);

reg_dir_writer::reg_dir_writer(const char *path) {
    TRY_OP((fd = open(path, O_DIRECTORY)) < 0)
}

void reg_dir_writer::close() {
    ::close(fd);
}


//// Directory Operations

outstream * reg_dir_writer::create(const pathname &path, const struct stat *st, int flags) {
    int fflags = flags & stream_flag_exclusive ? O_EXCL : 0;

    global_restart overwrite(overwrite_restart(fflags));

    file_outstream *stream;

    try_op([&] {
        stream = new file_outstream(fd, path.path().c_str(), fflags);
    });

    stream->times(st);

    set_file_attributes(stream->get_fd(), path.path().c_str(), st);

    return stream;
}

restart overwrite_restart(int &flags) {
    return restart("overwrite", [&flags] (const nuc::error &e, boost::any) {
        flags &= ~O_EXCL;
    },
    [] (const nuc::error &e) {
        return e.code() == EEXIST;
    });
}


void reg_dir_writer::mkdir(const pathname &path, bool) {
    TRY_OP_(mkdirat(fd, path.path().c_str(), S_IRWXU),
            throw file_error(errno, error::type_create_dir, true, path))
}

void reg_dir_writer::symlink(const pathname &path, const pathname &target, const struct stat *st) {
    TRY_OP(symlinkat(target.path().c_str(), fd, path.path().c_str()))

    set_attributes(path, st);
}

void reg_dir_writer::rename(const pathname &src, const pathname &dest) {
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

void reg_dir_writer::remove(const pathname &path, bool relative) {
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


//// Attributes

void reg_dir_writer::set_file_attributes(int fd, const char *path, const struct stat *st) {
    if (st) {
        with_skip_attrib([=] {
            TRY_OP_(fchmod(fd, st->st_mode & ~S_IFMT),
                    throw attribute_error(errno, error::type_set_mode, true, path));
        });

        with_skip_attrib([=] {
            TRY_OP_(fchown(fd, st->st_uid, st->st_gid),
                    throw attribute_error(errno, error::type_set_owner, true, path));
        });
    }
}

void reg_dir_writer::set_attributes(const pathname &path, const struct stat *st) {
    if (st) {
        if (!S_ISLNK(st->st_mode))
            with_skip_attrib([=] {
                TRY_OP_(fchmodat(fd, path.path().c_str(), st->st_mode & ~S_IFMT, 0),
                        throw attribute_error(errno, error::type_set_mode, true, path));
            });

        fs::time_type times[2];

        fs::stat_times(st, times);

        with_skip_attrib([&] {
            TRY_OP_(fs::set_ftimeat(fd, path.path().c_str(), times),
                    throw attribute_error(errno, error::type_set_times, true, path));
        });

        with_skip_attrib([=] {
            TRY_OP_(fchownat(fd, path.path().c_str(), st->st_uid, st->st_gid, AT_SYMLINK_NOFOLLOW),
                    throw attribute_error(errno, error::type_set_owner, true, path));
        });
    }
}

nuc::file_id reg_dir_writer::get_file_id(const pathname &path) {
    struct stat st;

    if (!fstatat(fd, path.c_str(), &st, AT_SYMLINK_NOFOLLOW)) {
        return file_id(st);
    }

    return file_id();
}
