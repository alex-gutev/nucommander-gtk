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

#include "archive_instream.h"

using namespace nuc;


const instream::byte *archive_instream::read_block(size_t &size, off_t &offset) {
    const instream::byte * buf;

    off_t new_offset = 0;
    int err = NUC_AP_OK;

    try_op([&] {
        err = plugin->unpack(handle, (const char **)&buf, &size, &new_offset);

        if (err < NUC_AP_OK)
            raise_error(errno, err == NUC_AP_RETRY);
    });

    offset = new_offset - last_offset;
    last_offset = new_offset + size;

    return err == NUC_AP_OK ? buf : nullptr;
}
