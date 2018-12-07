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


using namespace nuc;

archive_tree_lister::archive_tree_lister(archive_plugin *plugin, const paths::string &base, const std::vector<paths::string> &paths)
    : visit_paths(paths.begin(), paths.end()), listr(plugin, base) {}


void archive_tree_lister::list_entries(const list_callback &fn) {
    lister::entry ent;
    struct stat st = {};

    add_list_callback(fn);

    while (listr.read_entry(ent)) {
        paths::string ent_path = paths::canonicalized_path(ent.name);

        if (ent.type == DT_DIR)
            ent_path.push_back('/');

        if (should_visit(ent_path, ent)) {
            bool got_stat = listr.entry_stat(st);

            if (ent.type == DT_DIR) {
                if (!got_stat) {
                    st.st_mode = S_IFDIR | S_IRWXU;
                }

                paths::string name(ent.name);

                if (!add_dir_stat(name, &st)) {
                    continue;
                }
            }

            list_fn(ent, got_stat ? &st : nullptr, visit_preorder);
        }
    }

    for (auto it = visited_dirs.rbegin(), end = visited_dirs.rend(); it != end; ++it) {
        ent.name = it->first.c_str();
        ent.type = DT_DIR;

        if (it->second.second)
            list_fn(ent, &it->second.first, visit_postorder);
    }
}

bool archive_tree_lister::should_visit(const paths::string &path, lister::entry &ent) {
    auto it = visit_paths.lower_bound(path);

    if (visit_paths.begin() != visit_paths.end()) {
        if (it == visit_paths.end() || *it != path)
            --it;

        if (*it == path || paths::is_subpath(*it, path)) {
            size_t offset = it->rfind('/', it->size() - 2);
            offset = offset == paths::string::npos ? 0 : offset + 1;

            ent.name = path.c_str() + offset;

            return add_visited_dirs(offset, path);
        }
    }

    return false;
}

bool archive_tree_lister::add_visited_dirs(size_t base_offset, const paths::string &path) {
    struct stat st = {};
    st.st_mode = S_IFDIR | S_IRWXU;

    paths::string subpath = path.substr(base_offset);
    paths::path_components comps(subpath);

    paths::string dir_path;

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        if (!it.last()) {
            paths::append_component(dir_path, *it);
            dir_path += '/';

            auto visited_it = visited_dirs.find(dir_path);

            if (visited_it == visited_dirs.end()) {
                auto it = visited_dirs.emplace(std::make_pair(dir_path, std::make_pair(st, true)));

                lister::entry ent;
                ent.name = dir_path.c_str();
                ent.type = DT_DIR;

                if (!list_fn(ent, nullptr, visit_preorder)) {
                    it.first->second.second = false;
                    return false;
                }
            }
            else if (!visited_it->second.second)
                return false;
        }
    }

    return true;
}

bool archive_tree_lister::add_dir_stat(const paths::string &name, const struct stat *st) {
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
    return listr.symlink_path();
}

// Local Variables:
// indent-tabs-mode: nil
// End:
