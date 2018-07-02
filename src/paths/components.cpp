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

#include "components.h"


using namespace nuc::paths;

path_components::iter::iter(const string &path, size_t pos) : path(path), pos(pos) {
    if (pos != string::npos) {
        next_pos = next_slash();

        if (next_pos == pos)
            next_pos++;
    }
}

std::vector<string> path_components::all(const string &path) {
    path_components comps(path);
    std::vector<string> all;

    for (string comp : comps) {
        all.push_back(comp);
    }

    return all;
}


size_t path_components::iter::next_slash() {
    return path.find('/', pos);
}

size_t path_components::iter::next_non_slash() const{
    return path.find_first_not_of('/', next_pos);
}

void path_components::iter::next() {
    pos = next_non_slash();

    if (pos == string::npos) {
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
    return iter(path, string::npos);
}

string path_components::iter::operator*() const {
    return path.substr(pos, next_pos - pos);
}
