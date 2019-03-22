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

#include "instream.h"

using namespace nuc;

static constexpr size_t gap_block_size = 131072;

instream::~instream() {
    if (null_block) delete[] null_block;
}

const instream::byte * instream::read_block(size_t &size) {
    // If there is a pending gap which has not been returned yet
    if (gap_size) {
        size = std::min(gap_size, gap_block_size);
        gap_size -= size;

        return null_block;
    }

    // If there is a pending block which has not been returned yet
    if (last_block_size) {
        size = last_block_size;
        last_block_size = 0;

        return last_block;
    }

    // Read new block
    off_t offset;

    last_block = read_block(size, offset);

    // If there is a gap before the block
    if (offset) {
        last_block_size = size;

        size = std::min(offset, (off_t)gap_block_size);
        gap_size = offset - size;

        if (!null_block) null_block = new byte[gap_block_size]();
        return null_block;
    }

    return last_block;
}
