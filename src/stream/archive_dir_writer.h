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

#ifndef NUC_ARCHIVE_DIR_WRITER_H
#define NUC_ARCHIVE_DIR_WRITER_H

#include "dir_writer.h"

#include "paths/utils.h"
#include "plugins/archive_plugin.h"

namespace nuc {
    /**
     * Archive directory writer.
     *
     * Writes to archives.
     */
    class archive_dir_writer : public dir_writer {
        /**
         * Path to the archive file being written to.
         */
        paths::string path;
        /**
         * Path to the new archive file that is being created.
         */
        paths::string tmp_path;

        /**
         * Flag: True if the temporary file, into which the new
         * archive is written has been created and has not been
         * renamed over the old archive file.
         */
        bool tmp_exists = false;


        /**
         * Subpath within the archive, where all new entries are
         * created.
         */
        paths::string subpath;

        /**
         * Plugin for writing to the archive.
         */
        archive_plugin *plugin;

        /**
         * Handle of the existing archive (open for reading).
         */
        void *in_handle = nullptr;
        /**
         * Handle of the new archive file being created (open for
         * writing).
         */
        void *out_handle = nullptr;


        /**
         * Creates a temporary file and opens a new archive handle at
         * the file's location.
         */
        void open_temp();

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

    public:
        /**
         * Creates an archive directory writer for a particular
         * subpath within a particular archive.
         *
         * @param path Path to the archive file.
         * @param plugin Plugin for writing to the archive.
         * @param subpath Subpath within the archive.
         */
        archive_dir_writer(paths::string path, archive_plugin *plugin, paths::string subpath = paths::string());

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


        virtual outstream *create(const char *path, const struct stat *st = nullptr, int flags = 0);

        virtual void mkdir(const char *path) {
            // Does nothing as the directories are automatically
            // created with their child entries. An actual directory
            // entry is only created when its attributes are set.
        }

        virtual void symlink(const char *path, const char *target, const struct stat *st);

        virtual void set_attributes(const char *path, const struct stat *st);
    };
}


#endif

// Local Variables:
// mode: c++
// End:
