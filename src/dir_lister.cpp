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

#include "dir_lister.h"

#include <unistd.h>

using namespace nuc;

int dir_lister::init(const path_str &path) {
    dp = opendir(path.c_str());
    
    return dp ? 0 : errno;
}

dir_lister::~dir_lister() {
    if (dp) closedir(dp);
}

int dir_lister::init(int fd, bool dupfd) {
    int dfd = fd;
    
    if (dupfd) {
        if ((dfd = dup(fd)) < 0) {
            return errno;
        }
    }
    
    if ((dp = fdopendir(dfd))) {
        // Reset pointer to start of directory
        if (!lseek(dfd, 0, SEEK_SET)) {
            return 0;
        }
    }
    
    int err = errno;
    
    if (dupfd) {
        close(dfd);
    }
    
    return err;
}

void dir_lister::read_async() {
    if (!call_callback(BEGIN, NULL, NULL)) {
        read_dir();
    }
    
    call_finish(errno);
}

void dir_lister::read_dir() {
    struct dirent *dir_ent;
    struct stat st;
    
    entry ent;
    
    int stat_err = 0;
    
    while ((errno = 0, dir_ent = readdir(dp))) {
        stat_err = get_stat(dir_ent, &st);
        
        ent.name = dir_ent->d_name;
        ent.type = dir_ent->d_type;
        
        // In the case of a stat error a NULL pointer is passed to the
        // callback and errno is set.  It is up to the callback to
        // deal with the error, since this is not necessarily a
        // critical error.
        
        if (call_callback(ENTRY, &ent, !stat_err ? &st : NULL)) {
            errno = ECANCELED;
            return;
        }
    }
}

int dir_lister::get_stat(const struct dirent *ent, struct stat *st) {
    int fd = dirfd(dp);
    
    if (fstatat(fd, ent->d_name, st, 0)) {
        return fstatat(fd, ent->d_name, st, AT_SYMLINK_NOFOLLOW);
    }
    
    return 0;
}
