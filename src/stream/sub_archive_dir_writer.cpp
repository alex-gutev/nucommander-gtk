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

#include "sub_archive_dir_writer.h"

#include "file_instream.h"

using namespace nuc;


sub_archive_dir_writer::sub_archive_dir_writer(archive_plugin *plugin, dir_type *dtype, dir_writer *parent_writer, const paths::pathname &path, const paths::pathname &subpath)
    : archive_dir_writer(plugin, path, subpath), dtype(dtype), parent_writer(parent_writer) {
    open_old();

    try {
        open_temp(dtype->path());
    }
    catch (...) {
        close_handles();
        throw;
    }
}

void sub_archive_dir_writer::open_old() {
    try_op([this] {
        in_lister = std::unique_ptr<archive_lister>(dynamic_cast<archive_lister*>(dtype->create_lister()));
    });
}

void sub_archive_dir_writer::close() {
    open_old();
    copy_old_entries();

    if (out_handle) {
        int err = plugin->close(out_handle);
        out_handle = nullptr;

        if (err)
            raise_error(errno, false);
    }

    pack_to_parent();
    parent_writer->close();
}

void sub_archive_dir_writer::pack_to_parent() {
    std::unique_ptr<instream> in{new file_instream(tmp_path.c_str())};

    struct stat st;

    if (stat(tmp_path.c_str(), &st))
        throw error(errno);

    // TODO: Get stat attributes from file in archive.

    std::unique_ptr<outstream> out{parent_writer->create(path, &st, 0)};

    size_t size;
    off_t offset;

    while (const instream::byte *block = in->read_block(size, offset)) {
        out->write(block, size, offset);
    }
}
