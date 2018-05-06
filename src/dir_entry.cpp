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

#include "dir_entry.h"

#include "path_utils.h"

using namespace nuc;

// The default constructor call m_attr() is required to value
// initialize all members of the stat struct to zero

dir_entry::dir_entry(const path_str orig_name, uint8_t type)
    : m_orig_subpath(orig_name), m_subpath(canonicalized_path(orig_name)),
      m_file_name(nuc::file_name(m_subpath)), m_type(type), m_attr() {}

dir_entry::dir_entry(const lister::entry &ent) : dir_entry(ent.name, ent.type) {}

dir_entry::dir_entry(const lister::entry &ent, const struct stat &st) : dir_entry(ent.name, ent.type) {
    m_attr = st;
}

dir_entry::dir_entry(path_str path, const struct stat &st) : dir_entry(std::move(path), IFTODT(st.st_mode & S_IFMT)) {
    m_attr = st;
}


file_type dir_entry::type() const {
    file_type type = IFTODT(m_attr.st_mode);
    
    return type != DT_UNKNOWN ? type : m_type;
}
