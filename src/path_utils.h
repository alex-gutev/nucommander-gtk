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

#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>
#include <vector>

#include "types.h"
#include "path_components.h"

namespace nuc {
    path_str file_name(const path_str &path);
    path_str file_extension(const path_str &path);
    
    void append_component(path_str &path, const path_str &comp);
    
    path_str path_from_components(const std::vector<path_str> &comps);
    
    path_str canonicalized_path(const path_str &path);
}

#endif // PATH_UTILS_H
