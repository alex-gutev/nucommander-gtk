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

#ifndef NUC_STREAM_SUB_ARCHIVE_DIR_WRITER_H
#define NUC_STREAM_SUB_ARCHIVE_DIR_WRITER_H

#include "archive_dir_writer.h"
#include "directory/dir_type.h"

namespace nuc {
    /**
     * Directory writer for archives nested in other archives.
     */
    class sub_archive_dir_writer : public archive_dir_writer {
    public:
        /**
         * Constructor.
         *
         * @param plugin for writing to the nested archive.
         *
         * @param dtype Directory type of the nested archive. This
         *   object takes ownership of the directory type object, thus
         *   a copy should be passed to it.
         *
         * @param path Path, within the parent archive, to the nested
         *   archive file.
         *
         * @param subpath Subpath within the nested archive, at which
         *   new entries are created.
         */
        sub_archive_dir_writer(archive_plugin *plugin, dir_type *dtype, dir_writer *parent_writer, const paths::pathname &path, const paths::pathname &subpath);

        virtual void close();

    private:
        /**
         * Archive directory type.
         */
        std::unique_ptr<dir_type> dtype;
        /**
         * Directory writer for the parent archive.
         */
        std::unique_ptr<dir_writer> parent_writer;

        /**
         * Create a lister for the old archive.
         */
        void open_old();

        /**
         * Write the new archive file, stored at a temporary location,
         * to the parent archive.
         */
        void pack_to_parent();
    };

}  // nuc

#endif /* NUC_STREAM_SUB_ARCHIVE_DIR_WRITER_H */

// Local Variables:
// mode: c++
// End:
