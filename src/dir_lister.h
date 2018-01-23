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
        DIR *dp;
        
        /** Reads the directory. */
        void read_dir();
        
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
        
        /** Constructor */
        using lister::lister;
        
    public:
        
        /**
         * The static create method. The type parameter is ignored.
         */
        static lister *create(callback_fn callback, void *ctx, void *) {
            return new dir_lister(callback, ctx);
        }
        
        /**
         * Returns the creator functor for this object.
         */
        static creator get_creator() {
            return creator(create);
        }
        
        /**
         * Returns the file descriptor of the directory.
         */
        virtual int fd() const {
            return dirfd(dp);
        }
        
    protected:
        
        virtual ~dir_lister();
        
        /** Both init methods are supported */
        virtual int init(const path_str &path);
        virtual int init(int fd, bool dupfd);
        
        virtual void read_async();        
    };
}

#endif // NUC_DIR_LISTER_H
