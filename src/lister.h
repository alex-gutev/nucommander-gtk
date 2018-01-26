
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
     * Abstract filelister interface.
     *
     * This class provides a generic interface for listing the contents of
     * directories, regardless of whether the directories are OS
     * directories or virtual directories such as archives.
     *
     * The interface also provides an implementation of a thread-safe
     * callback mechanism.
     */
    class lister {        
    public:        
        /**
         * A simple entry structures containing two fields:
         *
         * name: The name of the entry as a c string.
         * type: The type of the entry as a dirent constant.
         */
        struct entry {
            const char *name;
            uint8_t type;
        };
        
        class error : public std::exception {
            int m_code;
            
        public:
            error(int code) : m_code(code) {}
            
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
         * Opens the directory with file descriptor fd. This may
         * not be supported by all listers.
         * 
         * fd:      The file descriptor
         * 
         * Returns true if opening a file descriptor is supported
         * by the lister, false if not supported.
         */
        virtual void open(int fd) {}
        
        /**
         * Reads the next entry into the entry
         * object 'ent'.
         * 
         * Returns true if an entry was read, false
         * if there are no more entries.
         * 
         * Throws 'error' with the value
         * of 'errno'.
         */
        virtual bool read_entry(entry &ent) = 0;
        /**
         * Retrieves the stat attributes of the last
         * entry read, and stores them into the stat struct
         * 'st'. Must be called after read_entry.
         * 
         * Returns true if the stat attributes were retrieved,
         * false otherwise.
         */
        virtual bool entry_stat(struct stat &st) = 0;
        
        /**
         * Closes open file handles.
         */
        virtual void close() = 0;
        
        /**
         * Returns the file descriptor of the directory being listed.
         *
         * If this is not supported (reads_fd() returns false) -1 is
         * returned.
         */
        virtual int fd() const {
            return -1;
        }
        
        /**
         * Returns true if the lister supports reading directly from a
         * file descriptor i.e. open(int fd, bool dupfd) can be used.  If
         * the return value is false only the open(path_str path) method
         * can be used.
         */
        static bool reads_fd() {
            return false;
        }
        
    protected:
        
        void raise_error(int code) {
            throw error(code);
        }
    };
}

#endif // NUC_LISTER_H
