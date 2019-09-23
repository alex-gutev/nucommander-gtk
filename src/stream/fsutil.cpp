/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#include "fsutil.h"

#include <string>

#ifdef __APPLE__
#include <fcntl.h>
#include <sys/param.h>
#endif

void nuc::fs::stat_times(const struct stat *st, time_type times[]) {
#ifdef __APPLE__
    TIMESPEC_TO_TIMEVAL(times, &st->st_atimespec);
    TIMESPEC_TO_TIMEVAL(times+1, &st->st_mtimespec);
#else
    times[0] = st->st_atim;
    times[1] = st->st_mtim;
#endif
}

int nuc::fs::set_ftime(int fd, const time_type *times) {
#ifdef __APPLE__
    return futimes(fd, times);
#else
    return futimens(fd, times);
#endif
}


int nuc::fs::set_ftimeat(int fd, const char *path, const time_type *times) {
#ifdef __APPLE__

    char dir_path[MAXPATHLEN+1] = {0};

    if (!fcntl(fd, F_GETPATH, dir_path)) {
        std::string full_path(dir_path);
        full_path.append("/");
        full_path.append(path);

        return lutimes(full_path.c_str(), times);
    }

    return -1;

#else
    return utimensat(fd, path, times, AT_SYMLINK_NOFOLLOW);
#endif
}
