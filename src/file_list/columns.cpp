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

#include "columns.h"

#include "sort_func.h"

#include <glib/gi18n.h>

using namespace nuc;

/**
 * Creates a new tree view column.
 *
 * @param title Column heading title.
 *
 * @return The column.
 */
static Gtk::TreeView::Column *create_column(const Glib::ustring &title);

/**
 * Adds a text cell to a tree view column, binds its
 * foreground_rgba attribute to the color model column, and
 * binds its text property to the tree model column @a data.
 *
 * @param col The Column.
 *
 * @param data The model column to bind to the text property
 *   of the cell.
 *
 * @return The text cell.
 */
static Gtk::CellRendererText *add_text_cell(Gtk::TreeView::Column *col, Gtk::TreeModelColumn<Glib::ustring> data);

/**
 * Adds a text cell to a tree view column, and binds its
 * foreground_rgba attribute to the color model column.
 *
 * @param col The Column.
 *
 * @return The text cell.
 */
static Gtk::CellRendererText *add_text_cell(Gtk::TreeView::Column *col);


/* Column Descriptors for built-in columns */

/**
 * Name and Icon Column.
 */
struct name_column : public column_descriptor {
    using column_descriptor::column_descriptor;

    virtual Gtk::TreeView::Column * create();
    virtual Gtk::TreeSortable::SlotCompare sort_func(Gtk::SortType order);
};

/**
 * File Size Column.
 */
struct size_column : public column_descriptor {
    using column_descriptor::column_descriptor;

    virtual Gtk::TreeView::Column * create();
    virtual Gtk::TreeSortable::SlotCompare sort_func(Gtk::SortType order);

    static void on_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter);
};

/**
 * Last Modified Date Column
 */
struct date_column : public column_descriptor {
    using column_descriptor::column_descriptor;

    virtual Gtk::TreeView::Column * create();
    virtual Gtk::TreeSortable::SlotCompare sort_func(Gtk::SortType order);

    static void on_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter);
};


/* Column Descriptor Instances */

/**
 * Returns the map, mapping column names to their integer identifiers.
 *
 * @return An unordered_map.
 */
static std::unordered_map<std::string, int> & column_name_map();


/**
 * Creates the column descriptors of the builtin Columns.
 *
 * @return Vector of column descriptors.
 */
static std::vector<std::unique_ptr<column_descriptor>> make_builtin_columns();



/* Implementations */

std::unordered_map<std::string, int> & column_name_map() {
    static std::unordered_map<std::string, int> map({
            std::make_pair("name", 0),
            std::make_pair("size", 1),
            std::make_pair("date-modified", 2)
    });

    return map;
}

std::vector<std::unique_ptr<column_descriptor>> & nuc::column_descriptors() {
    static std::vector<std::unique_ptr<column_descriptor>> array = make_builtin_columns();
    return array;
}

std::vector<std::unique_ptr<column_descriptor>> make_builtin_columns() {
    std::vector<std::unique_ptr<column_descriptor>> columns;

    columns.emplace_back(new name_column(0));
    columns.emplace_back(new size_column(1));
    columns.emplace_back(new date_column(2));

    return columns;
}


column_descriptor * nuc::get_column(const std::string &name) {
    auto &names = column_name_map();
    auto it = names.find(name);

    if (it != names.end()) {
        return get_column(it->second);
    }

    return nullptr;
}

column_descriptor * nuc::get_column(int id) {
    return column_descriptors()[id].get();
}


/* Column and Cell Creation Functions */

static Gtk::TreeView::Column *create_column(const Glib::ustring &title) {
    Gtk::TreeView::Column *col = Gtk::manage(new Gtk::TreeView::Column(title));
    col->set_resizable();

    return col;
}

static Gtk::CellRendererText *add_text_cell(Gtk::TreeView::Column *col, Gtk::TreeModelColumn<Glib::ustring> data) {
    auto cell = add_text_cell(col);

    col->add_attribute(cell->property_text(), data);

    return cell;
}

static Gtk::CellRendererText *add_text_cell(Gtk::TreeView::Column *col) {
    auto cell = Gtk::manage(new Gtk::CellRendererText());

    col->pack_start(*cell);
    col->add_attribute(cell->property_foreground_rgba(), file_model_columns::instance().color);

    return cell;
}


/* Column Descriptors Implementations */

/**
 * Returns either a cached formatted display string of an attribute
 * or, if there is no cached value, calls @a fn to generate the
 * formatted display string and saves into in the entry's formatted
 * value cache.
 *
 * @param ent The entry.
 *
 * @param key The attribute's key within the cache.
 *
 * @param fn A function of 0 arguments which should return a
 *   formatted display string of the attribute.
 *
 * @return The display string of the attribute.
 */
template<typename F>
Glib::ustring cached_value(dir_entry *ent, const std::string &key, F fn) {
    auto &cache = ent->context.format_cache;
    auto cit = cache.find(key);

    return cit == cache.end() ? (cache[key] = fn()) : cit->second;
}


/* Name and Icon Column */

Gtk::TreeView::Column *name_column::create() {
    auto &columns = file_model_columns::instance();
    auto *column = create_column(_("Name"));

    column->pack_start(columns.icon, false);
    auto *cell = add_text_cell(column, columns.name);

    cell->property_ellipsize().set_value(Pango::ELLIPSIZE_END);

    column->set_expand(true);
    column->set_sort_column(id);

    return column;
}

Gtk::TreeSortable::SlotCompare name_column::sort_func(Gtk::SortType order) {
    return combine_sort(make_invariant_sort(sort_entry_type, order), sort_name);
}


/* File Size Column */

Gtk::TreeView::Column *size_column::create() {
    auto *column = create_column(_("Size"));
    auto *cell = add_text_cell(column);

    column->set_cell_data_func(*cell, sigc::ptr_fun(on_data));
    column->set_expand(false);
    column->set_sort_column(id);

    return column;
}

Gtk::TreeSortable::SlotCompare size_column::sort_func(Gtk::SortType order) {
    return combine_sort(make_invariant_sort(sort_entry_type, order), sort_size, make_invariant_sort(sort_name, order));
}

void size_column::on_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
    Gtk::CellRendererText &text_cell = dynamic_cast<Gtk::CellRendererText&>(*cell);

    auto row = *iter;
    dir_entry *ent = row[file_model_columns::instance().ent];

    text_cell.property_text().set_value(cached_value(ent, "size", [=] () -> Glib::ustring {
        switch (ent->type()) {
        case dir_entry::type_reg: {
            const char *unit = "";

            size_t size = ent->attr().st_size;
            float frac = 0;

            if (size >= 1073741824) {
                unit = "GB";

                frac = (size % 1073741824) / 1073741824.0;
                size /= 1073741824;
            }
            else if (size >= 1048576) {
                unit = "MB";

                frac = (size % 1048576) / 1048576.0;
                size /= 1048576;
            }
            else if (size >= 1024) {
                unit = "KB";

                frac = (size % 1024) / 1024.0;
                size /= 1024;
            }

            if (int rem = (int)floorf(frac * 10)) {
                return Glib::ustring::compose("%1.%2 %3", size, rem, unit);
            }
            else {
                return Glib::ustring::compose("%1 %2", size, unit);
            }
        } break;

        case dir_entry::type_dir:
            return "<DIR>";
            break;

        default:
            return "";
        }
    }));
}


/* Last Modified Date Column */

Gtk::TreeView::Column *date_column::create() {
    auto *column = create_column(_("Date Modified"));
    auto *cell = add_text_cell(column);

    column->set_cell_data_func(*cell, sigc::ptr_fun(on_data));
    column->set_expand(false);
    column->set_sort_column(id);

    return column;
}

Gtk::TreeSortable::SlotCompare date_column::sort_func(Gtk::SortType order) {
    return combine_sort(make_invariant_sort(sort_entry_type, order), sort_mtime, make_invariant_sort(sort_name, order));
}

void date_column::on_data(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
    Gtk::CellRendererText &text_cell = dynamic_cast<Gtk::CellRendererText&>(*cell);

    auto row = *iter;
    dir_entry *ent = row[file_model_columns::instance().ent];

    text_cell.property_text().set_value(cached_value(ent, "mtime", [=] () -> Glib::ustring {
        if (ent->ent_type() != dir_entry::type_parent) {
            // localtime is NOT THREAD SAFE
            auto tm = localtime(&ent->attr().st_mtime);

            const size_t buf_size = 17;
            char buf[buf_size]  = {0};

            strftime(buf, buf_size, "%d/%m/%Y %H:%M", tm);
            return buf;
        }

        return "";
    }));
}
