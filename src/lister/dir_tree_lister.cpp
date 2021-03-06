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

#include "dir_tree_lister.h"

#include <cassert>

#include <fcntl.h>
#include <unistd.h>

#include "stream/file_instream.h"

using namespace nuc;

dir_tree_lister::dir_tree_lister(const pathname &base, const std::vector<pathname> &paths) {
    std::vector<std::string> full_paths;
    std::vector<const char *> cpaths;

    for (const pathname &path : paths) {
        full_paths.emplace_back(base.append(path));
        cpaths.push_back(full_paths.back().c_str());
    }

    cpaths.push_back(nullptr);

    // const_cast is required as FTS does not take a "char const *
    // const *" only to allow passing arguments of type char **, such
    // as the argv argument of the main function. It is safe as the
    // fts_ functions do not modify the strings in the array.

    if (!(handle = fts_open(const_cast<char * const *>(cpaths.data()), FTS_PHYSICAL | FTS_NOCHDIR, NULL))) {
        raise_error(errno);
    }
}

dir_tree_lister::~dir_tree_lister() {
    fts_close(handle);
}

void dir_tree_lister::list_entries(const list_callback &fn) {
    pathname current_dir;
    lister::entry ent;

    add_list_callback(fn);

    while ((last_ent = fts_read(handle))) {
        if (last_ent->fts_info == FTS_ERR || last_ent->fts_info == FTS_DNR)
            raise_error(last_ent->fts_errno);

        pathname::string name = last_ent->fts_namelen ? last_ent->fts_name : pathname(last_ent->fts_path).basename();
        pathname::string path = last_ent->fts_info == FTS_DP ? current_dir : current_dir.append(name);

        ent.type = get_type(last_ent);
        ent.name = path.c_str();

        if (!list_fn(ent, !stat_err(last_ent) && last_ent->fts_statp ? last_ent->fts_statp : nullptr, get_visit_info(last_ent))) {
            fts_set(handle, last_ent, FTS_SKIP);

            if (ent.type == DT_DIR) {
                // Skip over next entry: directory in post-order.
                fts_read(handle);
                continue;
            }
        }

        // Update path to current directory
        current_dir = set_dir(last_ent, name, current_dir);
    }

    if (int error = errno) {
        raise_error(error);
    }
}

pathname dir_tree_lister::set_dir(FTSENT *last_ent, const pathname::string &name, pathname current_dir) {
    switch (last_ent->fts_info) {
    case FTS_D:
        return current_dir.append(name);
        break;

    case FTS_DP:
        return current_dir.remove_last_component();
        break;
    }

    return current_dir;
}

int dir_tree_lister::get_type(FTSENT *ent) {
    switch (ent->fts_info) {
    case FTS_D:
    case FTS_DP:
    case FTS_DC:
        return DT_DIR;

    case FTS_F:
        return DT_REG;

    case FTS_NS:
    case FTS_NSOK:
        return DT_UNKNOWN;

    case FTS_SL:
    case FTS_SLNONE:
        return DT_LNK;

    default:
        return IFTODT(ent->fts_statp->st_mode);
    }
}

bool dir_tree_lister::stat_err(FTSENT *ent) {
    switch (ent->fts_info) {
    case FTS_ERR:
    case FTS_DNR:
    case FTS_NS:
    case FTS_NSOK:
        return true;
    }

    return false;
}


dir_tree_lister::visit_info dir_tree_lister::get_visit_info(FTSENT *last_ent) {
    switch (last_ent->fts_info) {
    case FTS_DP:
        return visit_info::visit_postorder;

    case FTS_DC:
        return visit_info::visit_cycle;

    default:
        return visit_info::visit_preorder;
    }
}


std::string dir_tree_lister::symlink_path() {
    assert(last_ent->fts_info == FTS_SL);

    std::string buf(last_ent->fts_statp->st_size+1, 0);

    try_op([&] {
        ssize_t sz;

        while ((sz = readlink(last_ent->fts_path, &buf.front(), buf.size())) == buf.size()) {
            buf.resize(sz * 2);
        }

        if (sz == -1)
            raise_error(errno);

        buf.resize(sz + 1);
        buf[sz] = 0;
    });

    return buf;
}

instream * dir_tree_lister::open_entry() {
    return new file_instream(last_ent->fts_path);
}

// Local Variables:
// indent-tabs-mode: nil
// End:
