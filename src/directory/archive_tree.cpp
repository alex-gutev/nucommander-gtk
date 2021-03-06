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

#include "archive_tree.h"

#include "paths/pathname.h"


using namespace nuc;

dir_tree::dir_map const * archive_tree::subpath_dir(const pathname &path) const {
    auto it = dirs.find(path);

    if (it != dirs.end()) {
        return &(it->second);
    }

    return nullptr;
}

bool archive_tree::is_subdir(const dir_entry& ent) const {
    return ent.type() == dir_entry::type_dir && dirs.count(ent.subpath());
}

dir_entry *archive_tree::get_entry(const pathname::string &name) {
    return dir_tree::get_entry(m_subpath.append(name).canonicalize());
}

dir_tree::entry_range archive_tree::get_entries(const pathname::string &name) {
    return dir_tree::get_entries(m_subpath.append(name).canonicalize());
}


dir_entry *archive_tree::add_entry(dir_entry ent) {
    dir_entry * dir_ent = ent.type() == dir_entry::type_dir ?
        add_dir_entry(std::move(ent)) : dir_tree::add_entry(std::move(ent));

    return add_components(dir_ent->subpath(), *dir_ent);
}

bool archive_tree::in_subpath(const pathname& path) {
    return path.is_child_of(m_subpath);
}

dir_entry *archive_tree::add_components(const pathname &path, dir_entry &ent) {
    std::vector<pathname::string> comps = path.components();

    file_map<dir_entry *> *parent_map = &dirs[""];
    pathname sub_path;

    dir_entry *child_ent = nullptr;

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        std::string comp = *it;

        sub_path = sub_path.append(comp);

        dir_entry *dent;

        if (it != (end - 1)) {
            dent = &make_dir_ent(sub_path);

            if (!add_to_map(*parent_map, comp, dent))
                dent = nullptr;

            parent_map = &dirs[sub_path];
        }
        else {
            dent = add_to_map(*parent_map, comp, &ent) ? &ent : nullptr;

            if (ent.type() == dir_entry::type_dir) {
                // Add to directory map, if not already present.
                dirs[sub_path];
            }
        }

        if (dent && in_subpath(sub_path)) {
            child_ent = dent;
        }
    }

    return child_ent;
}


dir_entry * archive_tree::add_dir_entry(nuc::dir_entry ent) {
    auto range = map.equal_range(ent.subpath());

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.type() == dir_entry::type_dir) {
            it->second = std::move(ent);
            return &it->second;
        }
    }

    return dir_tree::add_entry(std::move(ent));
}

bool archive_tree::add_to_map(file_map<dir_entry *> &map, const pathname::string &name, dir_entry *ent) {
    auto range = map.equal_range(name);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == ent) {
            return false;
        }
    }

    map.emplace(name, ent);
    return true;
}

dir_entry &archive_tree::make_dir_ent(const pathname &path) {
    auto range = map.equal_range(path);

    for (auto it = range.first; it != range.second; ++it) {
        dir_entry &ent = it->second;

        if (ent.ent_type() == dir_entry::type_dir) {
            return ent;
        }
    }

    dir_entry &ent = map.emplace(path, dir_entry(path, dir_entry::type_dir))->second;

    return ent;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
