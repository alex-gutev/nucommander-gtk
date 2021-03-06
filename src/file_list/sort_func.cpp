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

    dir_entry *ent1 = (*a)[columns.ent];
    dir_entry *ent2 = (*b)[columns.ent];

    Glib::ustring name1 = ent1->file_name();
    Glib::ustring name2 = ent2->file_name();

    return name1.uppercase().compare(name2.uppercase());
}

int nuc::sort_size(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    dir_entry *ent1 = (*a)[columns.ent];
    dir_entry *ent2 = (*b)[columns.ent];

    size_t sz1 = ent1->attr().st_size;
    size_t sz2 = ent2->attr().st_size;

    return sz1 > sz2 ? 1 : (sz1 < sz2 ? -1 : 0);
}

int nuc::sort_mtime(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    dir_entry *ent1 = (*a)[columns.ent];
    dir_entry *ent2 = (*b)[columns.ent];

    auto tm1 = ent1->attr().st_mtime;
    auto tm2 = ent2->attr().st_mtime;

    return tm1 > tm2 ? 1 : (tm1 < tm2 ? -1 : 0);
}

int nuc::sort_extension(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
    file_model_columns &columns = file_model_columns::instance();

    dir_entry *ent1 = (*a)[columns.ent];
    dir_entry *ent2 = (*b)[columns.ent];

    Glib::ustring ext1 = ent1->subpath().extension();
    Glib::ustring ext2 = ent2->subpath().extension();

    return ext1.uppercase().compare(ext2.uppercase());
}
