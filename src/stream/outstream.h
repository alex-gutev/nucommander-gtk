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

#ifndef NUC_OUTSTREAM_H
#define NUC_OUTSTREAM_H

#include <sys/types.h>
#include <stdint.h>

#include "errors/errors.h"
#include "paths/utils.h"

namespace nuc {
    /**
     * Abstract Output Stream Interface.
     */
    class outstream {
    public:
        typedef uint8_t byte;

        virtual ~outstream() = default;

        /**
         * Closes the output stream.
         *
         * @return True if the stream was closed successfully
         *    indicating that the data was successfully written to the
         *    storage medium
         */
        virtual void close() = 0;

        /**
         * Writes a block of data to the stream.
         *
         * @param buf The byte array to write to the stream.
         *
         * @param n Number of bytes in the array.
         *
         * @param offset Number of zero bytes to write between the end
         *    of the previous block and the start of the current
         *    block.
         */
        virtual void write(const byte *buf, size_t n, off_t offset = 0) = 0;

    protected:
        /**
         * Throws an error exception.
         *
         * @param code Error code.
         *
         * @param can_retry True if the operation can be retried,
         *    false otherwise.
         */
        void raise_error(int code, bool can_retry = true) {
            throw create_error(code, can_retry);
        };
    };
}

#endif

// Local Variables:
// mode: c++
// End:
