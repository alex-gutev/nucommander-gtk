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

#include "archive_outstream.h"


using namespace nuc;

void archive_outstream::write(const byte *buf, size_t n, off_t off) {
    try_op([=] {
        if (int err = plugin->pack(handle, (const char *)buf, n, off)) {
            throw error(plugin->error_code(handle), error::type_write_file, err == NUC_AP_RETRY, plugin->error_string(handle));
        }
    });
}
