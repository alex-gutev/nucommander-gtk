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

dir_entry::dir_entry(const lister::entry &ent, const struct stat *st)
    : m_orig_name(ent.name) {
    set_attr(ent, st);
}

void dir_entry::display_name(const path_str &name) {
    m_display_name = name;
    m_ext = file_extension(name);
}


void dir_entry::set_attr(const lister::entry &ent, const struct stat *st) {
    m_type = ent.type;
    
    if (st) {
        m_attr = *st;
    }
    else {
        m_attr.st_mode = DTTOIF(ent.type);
    }
}
