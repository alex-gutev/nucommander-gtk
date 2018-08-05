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

#include "lister/dir_lister.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstring>

#include "stream/file_instream.h"

using namespace nuc;

dir_lister::dir_lister(const paths::string &path) {
    if (!(dp = opendir(path.c_str()))) {
        raise_error(errno);
    }
}


dir_lister::~dir_lister() {
    close();
}

void dir_lister::close() {
    if (dp) closedir(dp);
    dp = nullptr;
}


bool dir_lister::read_entry(lister::entry &ent) {
    last_ent = next_ent();

    if (!last_ent) {
        return false;
    }

    ent.name = last_ent->d_name;
    ent.type = last_ent->d_type;

    return true;
}

struct dirent* dir_lister::next_ent() {
    struct dirent *ent;

    do {
        errno = 0;
        ent = readdir(dp);
    } while (ent && (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")));

    if (!ent) {
        int err = errno;

        if (err) raise_error(err);
    }

    return ent;
}


bool dir_lister::entry_stat(struct stat &st) {
    int fd = dirfd(dp);

    if (fstatat(fd, last_ent->d_name, &st, 0)) {
        return !fstatat(fd, last_ent->d_name, &st, AT_SYMLINK_NOFOLLOW);
    }

    return true;
}


instream * dir_lister::open_entry() {
    return new file_instream(dirfd(dp), last_ent->d_name);
}
