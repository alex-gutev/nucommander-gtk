
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

#ifndef NUC_LISTER_H
#define NUC_LISTER_H

#include <sys/stat.h>
#include <dirent.h>

#include <atomic>
#include <exception>

#include "types.h"

namespace nuc {
    /**
     * Abstract lister interface.
     *
     * Provides a generic interface for listing the contents of a
     * directory. The class is subclassed to provide implementations
     * for listing: regular directories, archives, remote directories,
     * etc.
     */
    class lister {
    public:
        /**
         * A simple entry structure containing two fields:
         *
         * name: The name of the entry as a C string.
         * type: The type of the entry as a 'dirent' constant.
         */
        struct entry {
            const char *name;
            uint8_t type;
        };

        /**
         * Error exception.
         */
        class error : public std::exception {
            /** Error code */
            const int m_code;
            
        public:
            /**
             * Constructs an error exception object with an error
             * code.
             */
            error(int code) : m_code(code) {}

            /**
             * Returns the error code.
             */
            int code() const {
                return m_code;
            }
        };
        
        virtual ~lister() = default;
        
        /**
         * Opens the directory at 'path' for reading.
         */
        virtual void open(const path_str &path) = 0;
        
        /**
         * Opens the directory with file descriptor fd. This operation
         * might not be supported by all subclasses. The method
         * 'opens_fd' can be used to determine whetherthe lister
         * object supports opening from a file descriptor.
         * 
         * fd: The file descriptor.
         */
        virtual void open(int fd) {}
        
        /**
         * Reads the next entry into the entry object 'ent'.
         * 
         * Returns true if an entry was read, false if there are no
         * more entries.
         * 
         * Throws 'error' with a suitable error code.
         */
        virtual bool read_entry(entry &ent) = 0;
        /**
         * Retrieves the stat attributes of the last entry read, and
         * stores them into the stat struct 'st'. Must be called after
         * read_entry.
         * 
         * Returns true if the stat attributes were retrieved, false
         * otherwise. Should not through any exceptions as failure to
         * retrieve stat attributes is not a critical error.
         */
        virtual bool entry_stat(struct stat &st) = 0;
        
        /**
         * Closes all open file handles. Should not through any
         * exceptions as close errors are not handled when reading.
         */
        virtual void close() = 0;
        
        /**
         * Returns the file descriptor of the directory being listed.
         *
         * If this is not supported (opens_fd() returns false) -1 is
         * returned.
         */
        virtual int fd() const {
            return -1;
        }
        
        /**
         * Returns true if the lister supports obtaining a handle to
         * the directory from a file descriptor as opposed to a file
         * path.
         */
        virtual bool opens_fd() {
            return false;
        }
        
    protected:

        /**
         * Throws an 'error' exception with error code 'code'.
         */
        void raise_error(int code) {
            throw error(code);
        }
    };
}

#endif // NUC_LISTER_H
