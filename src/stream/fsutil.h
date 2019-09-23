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

#ifndef NUC_STREAM_FSUTIL_H
#define NUC_STREAM_FSUTIL_H

#include <sys/stat.h>

#ifdef __APPLE__
#include <sys/time.h>
#endif

namespace nuc {
    namespace fs {
        /**
         * Modification and Access time type.
         */
#ifdef __APPLE__
        typedef struct timeval time_type;
#else
        typedef struct timespec time_type;
#endif

        /**
         * Retrieves the access and modification time from the stat
         * attributes of a file.
         *
         * @param st The stat attributes struct.
         *
         * @param times Pointer to array of two elements. The access
         *   time is stored in the first element and the modification
         *   time in the second element.
         */
        void stat_times(const struct stat *st, time_type times[]);

        /**
         * Sets the access and modification time of the file with file
         * descriptor @a fd.
         *
         * @param fd File descriptor.
         *
         * @param times Array storing the access time followed by the
         *   modification time.
         *
         * @return Zero if successful, non-zero on failure.
         */
        int set_ftime(int fd, const time_type *times);


        /**
         * Sets the access and modification time of the file at path
         * @a path relative to the directory with file descriptor @a
         * fd.
         *
         * If the file is a symbolic link, sets the times of the link
         * itself and not the target file.
         *
         * @param fd Directory File descriptor.
         *
         * @param path Path to the file relative to the directory with
         *   file descriptor @a fd.
         *
         * @param times Array storing the access time followed by the
         *   modification time.
         *
         * @return Zero if successful, non-zero on failure.
         */
        int set_ftimeat(int fd, const char *path, const time_type *times);
    }
}  // nuc

#endif /* NUC_STREAM_FSUTIL_H */

// Local Variables:
// mode: c++
// End:
