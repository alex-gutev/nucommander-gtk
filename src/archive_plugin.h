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

#ifndef NUC_ARCHIVE_PLUGIN_H
#define NUC_ARCHIVE_PLUGIN_H

#include <string>
#include <exception>

#include "archive_plugin_types.h"

namespace nuc {
    /**
     * Archive plugin class.
     *
     * Provides an interface to archive plugins. Archive plugins are
     * dynamically loaded shared libraries which provide functions for
     * reading and optionally creating archives.
     *
     * This class stores pointers to the shared library functions. A
     * single archive_plugin object should be created per plugin and
     * retained in memory.
     */
    class archive_plugin {
        /* Function Pointer types */

        typedef void*(*open_fn)(const char *, int, int *);
        typedef int(*close_fn)(void *);

        typedef int(*read_entry_fn)(void *, nuc_arch_entry *);
        typedef int(*unpack_entry_fn)(void *, int);

        typedef void(*set_callback_fn)(void *, nuc_arch_progress_fn, void *); 

        /**
         * Handle to the dynamically loaded shared library.
         */
        void *dl_handle = nullptr;

        /**
         * Checks whether the last dl operation resulted in an error,
         * by checking whether dlerror is NULL. If dlerror returns
         * non-NULL an error exception with error code @a code is
         * thrown.
         *
         * @param code The error code, as a load_error constant, of
         *    the exception to throw if the last operation resulted in
         *    an error.
         */
        void check_error(int code);
        
    public:
        /**
         * Exception thrown when there is an error loading the plugin.
         */
        class error : public std::exception {
            /** Error type code. */
            int code;
            
        public:
            /**
             * Error type code constants.
             */
            enum load_error {
                /**
                 * DLOPEN failed.
                 */
                dlopen = 1,
                /**
                 * One or more of the required API functions was not
                 * found in the shared library.
                 */
                api_incomplete
            };

            error(int code) : code(code) {}

            /** Returns the error code. */
            int type() const {
                return code;
            }
        };

        ~archive_plugin();

        /**
         * Loads the plugin a @a path. Throws an exception if loading
         * fails.
         *
         * @param Path to the plugin shared library
         */
        void load(const std::string &path);

        /**
         * For a documentation of the archive plugin api visit
         * archive_plugin_api.h
         */
        
        open_fn open;
        close_fn close;

        read_entry_fn read_entry;
        unpack_entry_fn unpack_entry;

        set_callback_fn set_callback;
    };
}

#endif // NUC_ARCHIVE_PLUGIN_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
