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

#include "stream/file_instream.h"

using namespace nuc;

dir_tree_lister::dir_tree_lister(const paths::string &base, const std::vector<paths::string> &paths) {
    std::vector<std::string> full_paths;
    std::vector<const char *> cpaths;

    for (const paths::string &path : paths) {
        full_paths.emplace_back(paths::appended_component(base, path));
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

void dir_tree_lister::close() {
    if (handle) {
        fts_close(handle);
        handle = nullptr;
    }
}


bool dir_tree_lister::read_entry(nuc::lister::entry &ent) {
    if ((last_ent = fts_read(handle))) {
        if (last_ent->fts_info == FTS_ERR || last_ent->fts_info == FTS_DNR)
            raise_error(last_ent->fts_errno);

        last_path = paths::appended_component(current_dir, last_ent->fts_name);

        // Update current directory
        set_dir();

        ent.type = get_type();
        ent.name = last_path.c_str();

        return true;
    }
    else if (int error = errno) {
        raise_error(error);
    }

    return false;
}

void dir_tree_lister::set_dir() {
    switch (last_ent->fts_info) {
    case FTS_D:
        paths::append_component(current_dir, last_ent->fts_name);
        break;

    case FTS_DP:
        paths::remove_last_component(current_dir);
        break;
    }
}

int dir_tree_lister::get_type() const {
    switch (last_ent->fts_info) {
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
        return IFTODT(last_ent->fts_statp->st_mode);
    }
}

bool dir_tree_lister::entry_stat(struct stat &st) {
    if (last_ent && !stat_err() && last_ent->fts_statp) {
        st = *last_ent->fts_statp;
        return true;
    }

    return false;
}

bool dir_tree_lister::stat_err() const {
    switch (last_ent->fts_info) {
    case FTS_ERR:
    case FTS_DNR:
    case FTS_NS:
    case FTS_NSOK:
        return true;
    }

    return false;
}


dir_tree_lister::visit_info dir_tree_lister::entry_visit_info() const {
    switch (last_ent->fts_info) {
    case FTS_DP:
        return visit_info::visit_postorder;

    case FTS_DC:
        return visit_info::visit_cycle;

    default:
        return visit_info::visit_preorder;
    }
}


instream * dir_tree_lister::open_entry() {
    return new file_instream(last_ent->fts_path);
}

// Local Variables:
// indent-tabs-mode: nil
// End:
