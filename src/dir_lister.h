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

#ifndef NUC_DIR_LISTER_H
#define NUC_DIR_LISTER_H

#include "lister.h"

namespace nuc {
    
    /**
     * filelister subclass for reading directories which are accessible
     * using the OS's file system API.
     */
    class dir_lister : public lister {
        /** Directory handle. */
        DIR *dp = nullptr;
        
        /** Last entry read. */
        struct dirent *last_ent;
        
        /**
         * Gets the stat attributes of an entry.
         *
         * First the stat system call is attempted, if it fails lstat is
         * attempted. If both calls fail an error value is returned.
         *
         * ent: The entry as returned by readdir.
         * st:  Pointer to the struct stat where the attributes will be stored.
         *
         * Returns 0 if successful, non-zero on error.
         */
        int get_stat(const struct dirent *ent, struct stat *st);
        
    public:
        virtual ~dir_lister();
        
        virtual void open(const path_str &path);
        virtual void open(int fd);
        
        virtual void close();
        
        virtual bool read_entry(entry &ent);
        virtual bool entry_stat(struct stat &st);
        
        /**
         * Returns the file descriptor of the directory.
         */
        virtual int fd() const {
            return dirfd(dp);
        }
        
        static bool reads_fd() {
            return true;
        }
    };
}

#endif // NUC_DIR_LISTER_H
