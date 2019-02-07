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

#include "directory_buffers.h"

using namespace nuc;

directory_buffers &directory_buffers::instance() {
    static directory_buffers buffers;
    return buffers;
}

directory_buffers::buffer_set &directory_buffers::buffers() {
    return bufs;
}

std::shared_ptr<file_list_controller> directory_buffers::new_buffer() {
    return *bufs.emplace(file_list_controller::create()).first;
}

void directory_buffers::close_buffer(std::shared_ptr<file_list_controller> flist) {
    bufs.erase(flist);
}
