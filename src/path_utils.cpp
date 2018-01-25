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

#include "path_utils.h"

nuc::path_str nuc::file_name(const path_str &path) {
    size_t slash_pos = path.rfind('/');
    
    if (slash_pos != std::string::npos) {
        return path.substr(slash_pos + 1);
    }
    
    return path;
}

nuc::path_str nuc::file_extension(const path_str &path) {
    // Search for first '.' or '/' from the end of string
    
    size_t pos = path.find_last_of("./");
    
    // If one of the characters was found and it is not
    // '/' return substring beginning from next character.
    if (pos != std::string::npos && path[pos] != '/') {
        return path.substr(pos + 1);
    }
    
    // No characters found, return empty string
    return std::string();
}

void nuc::append_component(path_str &path, const path_str &comp) {
    if (path.size() && path.back() != '/')
        path.append("/");
    
    path.append(comp);
}

nuc::path_str nuc::path_from_components(const std::vector<path_str> &comps) {
    path_str path;
    
    for (const path_str &comp : comps) {
        append_component(path, comp);
    }
    
    return path;
}

nuc::path_str nuc::canonicalized_path(const path_str &path) {
    path_components comps(path);
    std::vector<path_str> new_comps;
    
    bool first = true;
    
    for (path_str comp : comps) {
        if (comp == "..") {
            // If more than one component and last component is not '..'
            if (new_comps.size() && new_comps.back() != "..") {
                new_comps.pop_back();
            }
            else {
                // If first component, simply add to array
                new_comps.push_back(std::move(comp));
            }
        }
        else if (comp != "." && comp != "") {
            new_comps.push_back(std::move(comp));
        }
    }
    
    return path_from_components(new_comps);
}
