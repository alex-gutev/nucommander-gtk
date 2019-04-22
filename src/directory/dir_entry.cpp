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

#include "paths/pathname.h"

using namespace nuc;


dir_entry::dir_entry(const pathname orig_name, uint8_t type) : dir_entry(orig_name, dt_to_entry_type(type)) {}

// The default constructor call m_attr() is required to value
// initialize all members of the stat struct to zero

dir_entry::dir_entry(const pathname orig_name, entry_type type)
    : m_orig_subpath(orig_name), m_subpath(orig_name.canonicalize()),
      m_file_name(m_subpath.basename()), m_attr(), m_type(type) {}


dir_entry::dir_entry(const lister::entry &ent) : dir_entry(ent.name, ent.type) {}

dir_entry::dir_entry(const lister::entry &ent, const struct stat &st) : dir_entry(ent.name, ent.type) {
    m_attr = st;
}

dir_entry::dir_entry(pathname path, const struct stat &st) : dir_entry(std::move(path), IFTODT(st.st_mode & S_IFMT)) {
    m_attr = st;
}


const pathname &dir_entry::orig_subpath() const {
    return m_orig_subpath;
}
void dir_entry::orig_subpath(nuc::pathname path) {
    m_orig_subpath = path;
    subpath(std::move(path).canonicalize());
}

const pathname &dir_entry::subpath() const {
    return m_subpath;
}
void dir_entry::subpath(const pathname &path) {
    m_subpath = path;
    m_file_name = path.basename();
}

const pathname::string &dir_entry::file_name() const {
    return m_file_name;
}


dir_entry::entry_type dir_entry::ent_type() const noexcept {
    return m_type;
}
void dir_entry::ent_type(entry_type type) noexcept {
    m_type = type;
}
void dir_entry::ent_type(uint8_t type) noexcept {
    m_type = dt_to_entry_type(type);
}


dir_entry::entry_type dir_entry::type() const noexcept {
    uint8_t type = IFTODT(m_attr.st_mode);

    return type != DT_UNKNOWN ? dt_to_entry_type(type) : m_type;
}

dir_entry::entry_type dir_entry::dt_to_entry_type(uint8_t type) noexcept {
    switch (type) {
    case DT_UNKNOWN:
        return type_unknown;

    case DT_FIFO:
        return type_fifo;

    case DT_CHR:
        return type_chr;

    case DT_DIR:
        return type_dir;

    case DT_BLK:
        return type_blk;

    case DT_REG:
        return type_reg;

    case DT_LNK:
        return type_lnk;

    case DT_WHT:
        return type_wht;
    }

    return type_unknown;
}


const struct stat &dir_entry::attr() const {
    return m_attr;
}
void dir_entry::attr(const struct stat &st) {
    m_attr = st;
}
