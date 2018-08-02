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
#include <exception>
#include <tuple>

namespace nuc {
    /**
     * Abstract interface for an input stream.
     */
    class instream {
    public:
        /**
         * Error exception
         */
        class error : public std::exception {
            /**
             * Error code
             */
            int m_code;

        public:

            error(int code) : m_code(code) {}

            int code() const {
                return m_code;
            }
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
         */
        void raise_error(int code) {
            throw error(code);
        };
    };
}

#endif // NUC_INSTREAM_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
