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

#ifndef NUC_STREAM_REG_DIR_WRITER_H
#define NUC_STREAM_REG_DIR_WRITER_H

#include "dir_writer.h"


namespace nuc {
    /**
     * Regular Directory Writer.
     */
    class reg_dir_writer : public dir_writer {
    public:
        /**
         * Creates a directory writer for the directory at @a path.
         *
         * @param path Path to the directory.
         */
        reg_dir_writer(const char *path);

        /**
         * Closes the directory.
         */
        virtual ~reg_dir_writer() {
            close();
        }


        /* Method Overrides */

        virtual void close();

        virtual outstream *create(const pathname &path, const struct stat *st, int flags);

        virtual void mkdir(const pathname &path, bool defer);

        virtual void symlink(const pathname &path, const pathname &target, const struct stat *st);

        virtual void set_attributes(const pathname &path, const struct stat *st);

        virtual void rename(const pathname &src, const pathname &dest);

        virtual void remove(const pathname &path, bool relative);

        virtual file_id get_file_id(const pathname &path);

    private:
        /**
         * File descriptor of the directory.
         */
        int fd;

        /**
         * Sets the attributes of an open file.
         *
         * @param fd File descriptor of the file.
         *
         * @param path Path to the file whose attributes are being
         *   set. Only used for error reporting.
         *
         * @param st Stat attributes to set.
         */
        void set_file_attributes(int fd, const char *path, const struct stat *st);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
