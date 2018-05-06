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

#include "dir_tree.h"

#include "path_utils.h"


using namespace nuc;


dir_entry &dir_tree::add_entry(const lister::entry &ent, const struct stat &st) {
/*    dir_entry dent(ent, st);
    path_str key = dent.subpath();
    
    dir_entry &dir_ent = map.emplace(key, std::move(dent))->second;
    
    if (m_parse_dirs) {
        add_components(key, dir_ent);
    }
    
    return dir_ent;*/

    return add_entry(dir_entry(ent, st));
}

dir_entry &dir_tree::add_entry(dir_entry ent) {
    path_str key = ent.subpath();

    dir_entry &dir_ent = map.emplace(key, std::move(ent))->second;

    if (m_parse_dirs) {
        add_components(key, dir_ent);
    }

    return dir_ent;
}

void dir_tree::add_components(const path_str &path, dir_entry &ent) {
    path_components comps(path);
    
    file_map<dir_entry *> *parent_map = &m_base_dir;
    std::string sub_path;
    
    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        std::string comp = *it;
        
        dir_entry &comp_ent = it.last() ? ent : make_dir_ent(sub_path);
        add_to_map(*parent_map, comp, &comp_ent);
        
        parent_map = &comp_ent.child_ents();
    }
}

void dir_tree::add_to_map(file_map<dir_entry *> &map, const path_str &name, dir_entry *ent) {
    auto range = map.equal_range(name);
    
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == ent) {
            return;
        }
    }
    
    map.emplace(name, ent);
}

dir_entry &dir_tree::make_dir_ent(const path_str &path) {
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



