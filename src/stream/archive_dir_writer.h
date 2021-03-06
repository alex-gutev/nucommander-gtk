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

#ifndef NUC_STREAM_ARCHIVE_DIR_WRITER_H
#define NUC_STREAM_ARCHIVE_DIR_WRITER_H

#include <map>
#include <memory>

#include "dir_writer.h"

#include "paths/pathname.h"
#include "plugins/archive_plugin.h"

#include "lister/archive_lister.h"

namespace nuc {
    /**
     * Archive directory writer.
     *
     * Writes to archives.
     */
    class archive_dir_writer : public dir_writer {
    public:
        /**
         * Creates an archive directory writer for a particular
         * subpath within a particular archive.
         *
         * @param path Path to the archive file.
         * @param plugin Plugin for writing to the archive.
         * @param subpath Subpath within the archive.
         */
        archive_dir_writer(pathname path, archive_plugin *plugin, pathname subpath = pathname());

        /**
         * Closes any open archive handles, and deletes the new file
         * created if it has not been renamed over the old file.
         *
         * Does not call close() as close() performs additional tasks:
         * such as renaming the new archive file over the old archive
         * file. In order for those tasks to be performed close() has
         * to be called explicitly.
         */
        virtual ~archive_dir_writer();


        /* Method Overrides */

        /**
         * Closes both archive handles and renames the new archive
         * file over the old archive file, if its handle was closed
         * successfully.
         *
         * This method should be called explicitly as it is not called
         * from the destructor.
         */
        virtual void close();


        virtual outstream *create(const pathname &path, const struct stat *st, int flags);

        virtual void mkdir(const pathname &path, bool defer);

        virtual void symlink(const pathname &path, const pathname &target, const struct stat *st);

        virtual void set_attributes(const pathname &path, const struct stat *st);

        virtual void rename(const pathname &src, const pathname &dest);

        virtual void remove(const pathname &path, bool relative);

    protected:
        /**
         * Plugin for writing to the archive.
         */
        archive_plugin *plugin;

        /**
         * Handle of the existing archive (open for reading).
         */
        std::unique_ptr<archive_lister> in_lister;

        /**
         * Handle of the new archive file being created (open for
         * writing).
         */
        void *out_handle = nullptr;


        /**
         * Path to the new archive file that is being created.
         */
        pathname::string tmp_path;

        /**
         * Flag: True if the temporary file, into which the new
         * archive is written has been created and has not been
         * renamed over the old archive file.
         */
        bool tmp_exists = false;

        /**
         * Old entry information.
         */
        struct old_entry {
            /**
             * Type of the entry as a DT_ constant.
             */
            int type;
            /**
             * The path under which the entry should be recreated in
             * the new archive. If this is an empty string, the entry
             * is recreated under the same path.
             */
            pathname new_path;

            old_entry(int type) : type(type) {}
        };

        /**
         * Path to the archive file being written to.
         */
        pathname path;

        /**
         * Subpath within the archive, where all new entries are
         * created.
         */
        pathname subpath;

        /**
         * Map containing the entries already in the archive. Each key
         * is the canonicalized subpath to the entry and the
         * corresponding value is an old_entry struct.
         */
        std::map<pathname, old_entry> old_entries;


        /**
         * Constructs an archive_dir_writer without opening an archive
         * for writing, out_handle is set to NULL.
         *
         * @param plugin Plugin for writing to the archive.
         * @param path Path to the archive file.
         * @param subpath Subpath within the archive.
         */
        archive_dir_writer(archive_plugin *plugin, const pathname &path, const pathname &subpath);

        /**
         * Obtain the subpaths of all entries in the old archive and
         * store them in old_entries.
         */
        void get_old_entries();


        /**
         * Creates a temporary at the location @a path, with the file
         * name as the prefix, and opens a new archive handle for
         * writing.
         *
         * @param path Location where the temporary file should be
         * created.
         */
        void open_temp(const pathname &path);

        /**
         * Copies all entries in the existing archive to the new
         * archive.
         */
        void copy_old_entries();

        /**
         * Closes the two archive handles and deletes the new archive
         * file created, if it was not renamed over the old archive
         * file.
         */
        void close_handles();


        using dir_writer::raise_error;

        /**
         * Throws an error exception for the error with code @a
         * code. If @a type is NUC_AP_RETRY, the retry restart is
         * enabled.
         *
         * @param code The error code.
         * @param type Error type constant returned by the plugin.
         */
        void raise_error(int code, int type) {
            dir_writer::raise_error(code, type == NUC_AP_RETRY);
        }

        /**
         * Throws an error exception with the code and error
         * description obtained using archive_plugin::error_code and
         * archive::plugin_error_string.
         *
         * @param handle Handle of the archive in which the error was
         *   triggered.
         *
         * @param type Error type constant returned by the plugin.
         */
        void raise_plugin_error(void *handle, int type) {
            throw error(plugin->error_code(handle), error::type_general, type == NUC_AP_RETRY, plugin->error_string(handle));
        }

    private:
        /**
         * Opens the old archive for reading.
         */
        void open_old();

        /**
         * Creates a temporary file and opens a new archive handle at
         * the file's location.
         */
        void open_temp();

        /**
         * Adds the parent directory entries of the entry with subpath
         * @path to the old_entries map.
         *
         * @param path Subpath to the entry.
         */
        void add_parent_entries(pathname path);

        /**
         * Adds an old entry with subpath @a path to the old_entries
         * map.
         *
         * If there is already an entry with the same subpath in the
         * map, its type is not DT_DIR and @type is DT_DIR, it is
         * replaced.
         *
         * @param path Subpath to the entry.
         * @param type Type of the entry as a dirent constant.
         *
         * @return True if the entry was inserted in the map. False if
         *   there was an entry with the same subpath in the map, and
         *   it was not replaced.
         */
        bool add_old_entry(const pathname &path, int type);

        /**
         * Sets the type of the new archive to the same type as the
         * old archive.
         */
        void copy_archive_type();

        /**
         * Retrieve the metadata of the the next entry.
         *
         * @param ent Pointer to the nuc_arch_entry struct into which
         *    the metadata is read.
         */
        bool next_entry(lister::entry &ent);

        /**
         * Adds an entry header to the archive.
         *
         * @param path Path to the entry relative to 'subpath'.
         *
         * @param st Stat attributes of the entry
         *
         * @param symlink_dest Path to the symlink target if the entry
         *   is a symbolic link.
         */
        void create_entry(bool check, const char *path, const struct stat *st, const char *symlink_dest = nullptr);


        /**
         * Checks whether the old archive contains an entry at the
         * subpath @a path. If so an error is raised with two restarts
         * which allow the entry to be replaced or a new duplicate
         * entry to be created.
         *
         * @param path Path to the entry.
         */
        void check_exists(pathname path);

        /**
         * Removes the entry at subpath @a path. If it is a directory
         * entry all child entries are removed as well.
         *
         * @param path Subpath to the entry.
         */
        void remove_old_entry(pathname path);
    };
}


#endif

// Local Variables:
// mode: c++
// End:
