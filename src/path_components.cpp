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

#include "path_components.h"


using namespace nuc;

path_components::iter::iter(const std::string &path, size_t pos) : path(path), pos(pos) {
    if (pos != path.length()) {
        next_pos = next_slash();
        
        if (next_pos == pos)
            next_pos++;
    }
}

std::vector<std::string> path_components::all(const std::string &path) {
    path_components comps(path);
    std::vector<std::string> all;
    
    for (std::string comp : comps) {
        all.push_back(comp);
    }
    
    return all;
}




size_t path_components::iter::next_slash() {
    size_t next_pos = path.find('/', pos);
    
    if (next_pos == std::string::npos) {
        next_pos = path.length();
    }
    
    return next_pos;
}


void path_components::iter::next() {
    // Strip all leading slashes at next_pos
    // Search for next slash
    // Set as next_pos
    
    pos = path.find_first_not_of('/', next_pos);
    
    if (pos == std::string::npos) {
        pos = next_pos = path.length();
        return;
    }
    
    next_pos = next_slash();
    
    if (next_pos == pos) {
        next();
    }
}


path_components::iter path_components::begin() {
    return iter(path, 0);
}

path_components::iter path_components::end() {
    return iter(path, path.length());
}

std::string path_components::iter::operator*() const {
    return path.substr(pos, next_pos - pos);
}
