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

using namespace nuc;

archive_lister::~archive_lister() {
    close();
}

void archive_lister::open(const paths::string &path) {
    int error = 0;

    if (!(handle = plugin->open(path.c_str(), NUC_AP_MODE_UNPACK, &error))) {
        raise_error(error);
    }
}

void archive_lister::close() {
    if (handle) plugin->close(handle);
    handle = nullptr;
}


bool archive_lister::read_entry(lister::entry &ent) {
    int err = plugin->next_entry(handle, &arch_entry);

    if (err && err != NUC_AP_EOF) {
        // Refine error handling
        raise_error(errno);
    }

    ent.name = arch_entry.path;
    ent.type = IFTODT(arch_entry.stat->st_mode & S_IFMT);

    return err == NUC_AP_OK;
}

bool archive_lister::entry_stat(struct stat& st) {
    st = *arch_entry.stat;
    return true;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
