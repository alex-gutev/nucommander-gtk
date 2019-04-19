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

#ifndef NUC_LISTER_SUB_ARCHIVE_LISTER_H
#define NUC_LISTER_SUB_ARCHIVE_LISTER_H

#include "lister/archive_lister.h"

namespace nuc {
    /**
     * Lister for an archive located within another archive.
     */
    class sub_archive_lister : public archive_lister {
    public:
        /**
         * Constructor.
         *
         * @param parent Lister object for the archive containing the
         *   archive to list.
         *
         * @param plugin Plugin for reading the archive.
         *
         * @param subpath Subpath, within the containing archive, to
         *   the archive.
         */
        sub_archive_lister(lister *parent, archive_plugin *plugin, const pathname &subpath);

        virtual ~sub_archive_lister();

    private:
        /**
         * Lister object for the archive containing this archive.
         */
        std::unique_ptr<lister> parent_lister;
        /**
         * Input stream for the archive file.
         */
        instream *arch_stream = nullptr;


        /**
         * Finds the archive file within the containing archive, and
         * creates a input stream for the the archive, stored in
         * arch_stream.
         *
         * If the file is not found in the archive an error is
         * signalled.
         *
         * @param subpath Subpath to the archive file.
         */
        void find_archive_file(const pathname &subpath);

        /**
         * Reads a block of data from the archive file.
         *
         * @param buffer Pointer to a pointer which is set to point to
         *   the block just read.
         *
         * @return The size of the block just read, 0 if the end of
         *   file has been reached.
         */
        ssize_t read_block(const void **buffer);

        /**
         * Read callback function.
         *
         * @param ctx Context set to the this pointer of the object.
         *
         * @param buffer Pointer to a pointer which is set to point to
         *   the block just read.
         *
         * @return The size of the block just read, 0 if the end of
         *   file has been reached, -1 if an error occurred.
         */
        static ssize_t read_fn(void *ctx, const void **buffer);
    };

}  // nuc

#endif /* NUC_LISTER_SUB_ARCHIVE_LISTER_H */

// Local Variables:
// mode: c++
// End:
