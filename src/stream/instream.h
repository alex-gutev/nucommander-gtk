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

#ifndef NUC_INSTREAM_H
#define NUC_INSTREAM_H

#include <stdint.h>
#include <stdlib.h>

#include <vector>
#include <tuple>

#include "error.h"

namespace nuc {
    /**
     * Abstract interface for an input stream.
     */
    class instream : public restartable {
        error_handler_fn err_handler;

    public:
        /**
         * Error exception
         */
        class error : public nuc::error {
        public:
            using nuc::error::error;
        };

        typedef uint8_t byte;

        virtual ~instream() = default;

        /**
         * Closes the stream
         */
        virtual void close() = 0;

        /**
         * Reads a block of data from the input stream, the size of
         * the block is determined by the stream.
         *
         * @param size Reference to the variable where the size of the
         *    block in bytes is stored.
         *
         * @param offset Number of bytes (zeroes) from the end of the
         *    previous block to the start of the current block.
         *
         * @return Pointer to the byte array containing the data in
         *    the block. If this is NULL then the end of file has been
         *    reached.
         */
        virtual const byte *read_block(size_t &size, off_t &offset) = 0;

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
            throw error(code, can_retry);
        };
    };
}

#endif // NUC_INSTREAM_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
