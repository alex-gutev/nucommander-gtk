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

#ifndef NUC_STREAM_ARCHIVE_INSTREAM_H
#define NUC_STREAM_ARCHIVE_INSTREAM_H

#include "instream.h"

#include "plugins/archive_plugin.h"

namespace nuc {
    /**
     * Input stream for files in archives.
     */
    class archive_instream : public instream {
    public:

        /**
         * Creates an input stream for a file located in an
         * archive.
         *
         * No operations should be performed, using the archive handle
         * @a handle, while the stream object is in use. The stream
         * object does not release the archive handle.
         *
         * @param plugin Plugin for reading the archive.
         * @param handle Archive handle
         */
        archive_instream(archive_plugin *plugin, void *handle) : plugin(plugin), handle(handle) {}


        /* Method Overrides */

        /**
         * Does nothing, the stream object does not release the
         * archive handle. It is the responsibility of the creator of
         * this object to release it.
         */
        virtual void close() {}

        virtual const byte *read_block(size_t &size, off_t &offset);

    private:
        /**
         * Offset of the first byte after the last block of data read.
         */
        off_t last_offset = 0;

        /**
         * Plugin for reading the archive.
         */
        archive_plugin *plugin;

        /**
         * Archive handle.
         */
        void *handle;
    };
}


#endif // ARCHIVE

// Local Variables:
// mode: c++
// End:
