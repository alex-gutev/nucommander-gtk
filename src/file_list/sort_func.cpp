/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#include "sort_func.h"

using namespace nuc;

int nuc::sort_entry_type(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    dir_entry *ent_a = (*a)[columns.ent];
    dir_entry *ent_b = (*b)[columns.ent];

    dir_entry::entry_type type_a = ent_a->type();
    dir_entry::entry_type type_b = ent_b->type();

    if (type_a == dir_entry::type_parent)
        return -1;
    else if (type_b == dir_entry::type_parent)
        return 1;
    else if (type_a == dir_entry::type_dir && type_b != dir_entry::type_dir) {
        return -1;
    }
    else if (type_b == dir_entry::type_dir && type_a != dir_entry::type_dir) {
        return 1;
    }

    return 0;
}

int nuc::sort_name(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    Glib::ustring name_a = (*a)[columns.name];
    Glib::ustring name_b = (*b)[columns.name];

    return name_a.compare(name_b);
}

int nuc::sort_size(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    dir_entry *ent1 = (*a)[columns.ent];
    dir_entry *ent2 = (*b)[columns.ent];

    size_t sz1 = ent1->attr().st_size;
    size_t sz2 = ent2->attr().st_size;

    return sz1 > sz2 ? 1 : (sz1 < sz2 ? -1 : 0);
}
