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

#ifndef NUC_ARCHIVE_OUTSTREAM_H
#define NUC_ARCHIVE_OUTSTREAM_H

#include "outstream.h"
#include "plugins/archive_plugin.h"

namespace nuc {
    /**
     * Archive Output Stream.
     *
     * Writes to files contained in archives.
     */
    class archive_outstream : public outstream {
        /**
         * Plugin for writing to the archive.
         */
        archive_plugin *plugin;

        /**
         * Handle of the open archive.
         */
        void *handle;

    public:
        /**
         * Creates an archive outstream.
         *
         * No operations should be performed on the archive, and the
         * handle should not be closed, while this stream is in use.
         *
         * @param plugin Plugin for writing to the archive.
         * @param handle Handle of the open archive.
         */
        archive_outstream(archive_plugin *plugin, void *handle) : plugin(plugin), handle(handle) {}

        ~archive_outstream() {
            close();
        }


        /* Method Overrides */

        virtual void close() {
            // Does nothing as it is the responsibility of the creator
            // of this object to close the handle.
        }

        virtual void write(const byte *buf, size_t n, off_t offset);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
