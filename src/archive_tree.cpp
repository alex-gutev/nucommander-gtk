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

#include "path_utils.h"


using namespace nuc;

dir_entry *archive_tree::add_entry(dir_entry ent) {
    dir_entry *dir_ent = dir_tree::add_entry(std::move(ent));

    return add_components(dir_ent->subpath(), *dir_ent);
}

bool archive_tree::in_subpath(const path_str& path) {
    return is_child_of(m_subpath, path);
}

dir_entry *archive_tree::add_components(const path_str &path, dir_entry &ent) {
    path_components comps(path);

    file_map<dir_entry *> *parent_map = &dirs[""];
    std::string sub_path;

    dir_entry *child_ent = nullptr;
    
    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        std::string comp = *it;
        
        append_component(sub_path, comp);

        dir_entry *dent;
        
        if (!it.last()) {
            dent = &make_dir_ent(sub_path);
            
            if (!add_to_map(*parent_map, comp, dent))
                dent = nullptr;
            
            parent_map = &dirs[sub_path];
        }
        else {
            dent = add_to_map(*parent_map, comp, &ent) ? &ent : nullptr;
        }

        if (dent && in_subpath(sub_path)) {
            child_ent = dent;
        }
    }

    return child_ent;
}

bool archive_tree::add_to_map(file_map<dir_entry *> &map, const path_str &name, dir_entry *ent) {
    auto range = map.equal_range(name);
    
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == ent) {
            return false;
        }
    }

    map.emplace(name, ent);
    return true;
}

dir_entry &archive_tree::make_dir_ent(const path_str &path) {
    auto range = map.equal_range(path);
    
    for (auto it = range.first; it != range.second; ++it) {
        dir_entry &ent = it->second;
        
        if (ent.ent_type() == DT_DIR) {
            return ent;
        }
    }
    
    dir_entry &ent = map.emplace(path, dir_entry(path, DT_DIR))->second;
    
    return ent;
}

dir_tree::dir_map const * archive_tree::subpath(const path_str &path) {
    auto it = dirs.find(path);

    if (it != dirs.end()) {
        m_subpath = path;
        
        return &(it->second);
    }

    return nullptr;
}

bool archive_tree::is_subdir(const dir_entry& ent) const {
    return ent.type() == DT_DIR && dirs.count(ent.subpath());
}

// Local Variables:
// indent-tabs-mode: nil
// End:
