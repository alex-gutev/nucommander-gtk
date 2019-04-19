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

#include "archive_tree_lister.h"

#include "sub_archive_lister.h"


using namespace nuc;

archive_tree_lister::archive_tree_lister(archive_plugin *plugin, const pathname &base, const std::vector<pathname> &paths)
    : archive_tree_lister(new archive_lister(plugin, base), paths) {
}

archive_tree_lister::archive_tree_lister(archive_lister *listr, const std::vector<pathname> &paths)
    : listr(listr) {
    for (auto path : paths) {
        visit_paths.emplace(path.canonicalize());
    }
}


void archive_tree_lister::list_entries(const list_callback &fn) {
    lister::entry ent;
    struct stat st = {};

    add_list_callback(fn);

    while (listr->read_entry(ent)) {
        pathname ent_path = pathname(ent.name).canonicalize();

        if (ent.type == DT_DIR)
            ent_path = pathname(ent_path, true);

        size_t offset = path_offset(ent_path);
        if (offset != pathname::string::npos) {
            bool got_stat = listr->entry_stat(st);

            ent.name = ent_path.path().c_str() + offset;

            if (ent.type == DT_DIR) {
                if (!got_stat) {
                    st.st_mode = S_IFDIR | S_IRWXU;
                }

                pathname::string name(ent.name);

                if (!add_dir_stat(name, &st)) {
                    continue;
                }
            }

            list_fn(ent, got_stat ? &st : nullptr, visit_preorder);
        }
    }

    for (auto it = visited_dirs.rbegin(), end = visited_dirs.rend(); it != end; ++it) {
        ent.name = it->first.path().c_str();
        ent.type = DT_DIR;

        if (it->second.second)
            list_fn(ent, &it->second.first, visit_postorder);
    }
}

size_t archive_tree_lister::path_offset(const pathname &path) {
    size_t offset = pathname::subpath_offset(visit_paths, path);

    if (offset != pathname::string::npos) {
        return add_visited_dirs(offset, path) ? offset : pathname::string::npos;
    }

    return pathname::string::npos;
}

bool archive_tree_lister::add_visited_dirs(size_t base_offset, const pathname &path) {
    struct stat st = {};
    st.st_mode = S_IFDIR | S_IRWXU;

    pathname subpath = path.path().substr(base_offset);
    std::vector<pathname::string> comps = subpath.components();

    pathname dir_path;

    for (auto it = comps.begin(), end = comps.end()-1; it != end; ++it) {
        dir_path = pathname(dir_path.append(*it), true);

        auto visited_it = visited_dirs.find(dir_path);

        if (visited_it == visited_dirs.end()) {
            auto it = visited_dirs.emplace(std::make_pair(dir_path, std::make_pair(st, true)));

            lister::entry ent;
            ent.name = dir_path.path().c_str();
            ent.type = DT_DIR;

            if (!list_fn(ent, nullptr, visit_preorder)) {
                it.first->second.second = false;
                return false;
            }
        }
        else if (!visited_it->second.second)
            return false;
    }

    return true;
}

bool archive_tree_lister::add_dir_stat(const pathname &name, const struct stat *st) {
    auto dir_it = visited_dirs.find(name);

    if (dir_it != visited_dirs.end()) {
        dir_it->second.first = *st;
        return false;
    }
    else {
        visited_dirs.emplace(std::make_pair(name, std::make_pair(*st, true)));
        return true;
    }
}

std::string archive_tree_lister::symlink_path() {
    return listr->symlink_path();
}

// Local Variables:
// indent-tabs-mode: nil
// End:
