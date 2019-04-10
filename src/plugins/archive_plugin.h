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
#include <mutex>

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

        typedef void*(*open_unpack_fn)(nuc_arch_read_callback, nuc_arch_skip_callback, void *, int *);
        typedef void*(*open_pack_fn)(nuc_arch_write_callback, void *, int *);

        typedef int(*error_code_fn)(void *);
        typedef const char *(*error_string_fn)(void *);

        typedef int(*next_entry_fn)(void *, const char **);
        typedef const struct stat *(*entry_stat_fn)(void *);
        typedef const char *(*entry_link_path_fn)(void *);
        typedef const char *(*entry_symlink_path_fn)(void *);

        typedef int(*unpack_fn)(void *, const char **, size_t *, off_t *);

        typedef int(*copy_archive_type_fn)(void *, const void *);
        typedef int(*copy_last_entry_header_fn)(void *, const void *);
        typedef int(*copy_last_entry_data_fn)(void *, const void *);

        typedef int(*create_entry_fn)(void *, const char *, const struct stat *);
        typedef void(*entry_set_path_fn)(void *, const char *);
        typedef void(*entry_set_stat_fn)(void *, const struct stat *);
        typedef void(*entry_set_link_path_fn)(void *, const char *);
        typedef void(*entry_set_symlink_path_fn)(void *, const char *);
        typedef int(*write_entry_header_fn)(void *);

        typedef int(*pack_fn)(void *, const char *, size_t, off_t);
        typedef int(*pack_finish_fn)(void *);

        typedef void(*set_callback_fn)(void *, nuc_arch_progress_fn, void *);

        /**
         * Path to the plugin shared library file.
         */
        const std::string path;

        /**
         * Mutex for synchronizing loading and unloading.
         */
        std::mutex mutex;


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

        /**
         * Constructs an archive_plugin object, for a plugin located
         * at a particular path.
         *
         * The plugin is not loaded, call the load method before
         * calling any of the plugin functions.
         *
         * @param path Path to the plugin's shared library file.
         */
        archive_plugin(std::string path) : path(path) {}

        /**
         * Copy constructor and assignment operator are deleted as
         * there should only be a single instance per plugin.
         */
        archive_plugin(const archive_plugin &) = delete;
        archive_plugin& operator=(const archive_plugin &) = delete;

        /**
         * Destructor: Unloads the plugin.
         */
        ~archive_plugin();

        /**
         * Loads the plugin's shared library, if not already
         * loaded. If the shared library has already been loaded, this
         * method does nothing. Throws an archive_plugin::error
         * exception if loading fails.
         *
         * Should be called prior to calling any of the plugin
         * functions.
         */
        void load();


        /**
         * For a documentation of the archive plugin api visit
         * archive_plugin_api.h
         */

        open_fn open;
        close_fn close;

        open_unpack_fn open_unpack;
        open_pack_fn open_pack;

        error_code_fn error_code;
        error_string_fn error_string;

        next_entry_fn next_entry;
        entry_stat_fn entry_stat;
        entry_link_path_fn entry_link_path;
        entry_symlink_path_fn entry_symlink_path;

        unpack_fn unpack;

        copy_archive_type_fn copy_archive_type;
        copy_last_entry_header_fn copy_last_entry_header;
        copy_last_entry_data_fn copy_last_entry_data;

        create_entry_fn create_entry;
        entry_set_path_fn entry_set_path;
        entry_set_stat_fn entry_set_stat;
        entry_set_link_path_fn entry_set_link_path;
        entry_set_symlink_path_fn entry_set_symlink_path;
        write_entry_header_fn write_entry_header;
        pack_fn pack;
        pack_finish_fn pack_finish;

        set_callback_fn set_callback;
    };
}

#endif // NUC_ARCHIVE_PLUGIN_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
