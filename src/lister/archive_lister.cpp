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

#include "archive_lister.h"

#include "stream/archive_instream.h"

#include <cassert>

using namespace nuc;

archive_lister::~archive_lister() {
    close();
}

archive_lister::archive_lister(archive_plugin *plugin, const paths::string &path) : plugin(plugin) {
    int error = 0;

    if (!(handle = plugin->open(path.c_str(), NUC_AP_MODE_UNPACK, &error))) {
        raise_error(error);
    }
}

archive_lister::archive_lister(archive_plugin *plugin) : plugin(plugin), handle(nullptr) {}


void archive_lister::close() {
    if (handle) plugin->close(handle);
    handle = nullptr;
}


bool archive_lister::read_entry(lister::entry &ent) {
    int err = plugin->next_entry(handle, &ent.name);

    if (err == NUC_AP_EOF)
        return false;

    if (err != NUC_AP_OK) {
        // Refine error handling
        raise_error(errno);
    }

    const struct stat *st = plugin->entry_stat(handle);

    ent.type = IFTODT(st->st_mode & S_IFMT);

    return true;
}

bool archive_lister::entry_stat(struct stat& st) {
    const struct stat *s = plugin->entry_stat(handle);

    if (s) st = *s;

    return s;
}

instream * archive_lister::open_entry() {
    return new archive_instream(plugin, handle);
}

const char *archive_lister::symlink_path() const {
    assert(S_ISLNK(plugin->entry_stat(handle)->st_mode));

    return plugin->entry_link_path(handle);
}

// Local Variables:
// indent-tabs-mode: nil
// End:
