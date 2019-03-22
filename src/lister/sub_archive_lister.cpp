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

#include "sub_archive_lister.h"

using namespace nuc;

sub_archive_lister::sub_archive_lister(lister *parent, archive_plugin *plugin, const paths::pathname &subpath)
    : archive_lister(plugin), parent_lister(parent) {
    int error = 0;

    find_entry(subpath);

    if (!(handle = plugin->open_unpack(read_fn, nullptr, this, &error))) {
        raise_error(error);
    }
}

sub_archive_lister::~sub_archive_lister() {
    if (arch_stream) delete arch_stream;
    if (parent_lister) delete parent_lister;
}

void sub_archive_lister::find_entry(const paths::pathname &subpath) {
    lister::entry ent;

    while (parent_lister->read_entry(ent)) {
        if (subpath == paths::pathname(ent.name).canonicalize()) {
            arch_stream = parent_lister->open_entry();
            break;
        }
    }
}


ssize_t sub_archive_lister::read_block(const void **buffer) {
    size_t size;

    *buffer = arch_stream->read_block(size);
    return size;
}

ssize_t sub_archive_lister::read_fn(void *ctx, const void **buffer) {
    sub_archive_lister *self = static_cast<sub_archive_lister*>(ctx);

    try {
        return self->read_block(buffer);
    }
    catch (const nuc::error &) {
        return -1;
    }
}
