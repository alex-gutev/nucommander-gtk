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

using namespace nuc;

dir_entry* dir_tree::add_entry(const lister::entry &ent, const struct stat &st) {
    return add_entry(dir_entry(ent, st));
}

dir_entry* dir_tree::add_entry(dir_entry ent) {
    paths::string key = ent.subpath();

    dir_entry &dir_ent = map.emplace(key, std::move(ent))->second;
    
    return &dir_ent;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
