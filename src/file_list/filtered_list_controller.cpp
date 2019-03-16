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

#include "filtered_list_controller.h"

using namespace nuc;

//// Initialization

std::shared_ptr<filtered_list_controller> filtered_list_controller::create(std::shared_ptr<list_controller> flist, filter_fn filter) {
    return std::make_shared<filtered_list_controller>(flist, filter);
}

filtered_list_controller::filtered_list_controller(std::shared_ptr<list_controller> flist, filter_fn filter) :
    m_filter(filter), flist(flist), m_list(Gtk::ListStore::create(columns)) {

    flist->signal_change_model().connect(sigc::mem_fun(this, &filtered_list_controller::change_model));
    flist->signal_select().connect(sigc::mem_fun(this, &filtered_list_controller::select_row));
}


//// Filtering

void filtered_list_controller::refilter() {
    refilter(flist->selected());
}

void filtered_list_controller::refilter(Gtk::TreeRow selection) {
    m_list->clear();
    m_list->set_sort_column(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, Gtk::SortType::SORT_ASCENDING);

    auto children = flist->list()->children();

    Gtk::TreeRow select_row;

    for (Gtk::TreeRow row : children) {
        bool visible;
        float score;

        std::tie(visible, score) = m_filter(row);

        if (visible) {
            auto new_row = add_row(row, score);

            if (selection && row == selection) {
                select_row = new_row;
            }
        }
    }

    m_list->set_sort_column(columns.score, Gtk::SortType::SORT_DESCENDING);
    selected_row = select_row;
}

Gtk::TreeRow filtered_list_controller::add_row(Gtk::TreeRow row, float score) {
    Gtk::TreeRow new_row = *m_list->append();

    copy_columns(row, new_row);
    new_row[columns.score] = score;

    return new_row;
}

void filtered_list_controller::copy_columns(Gtk::TreeRow src, Gtk::TreeRow dest) {
    for (size_t i = 0, sz = columns.size() - 1; i < sz; i++) {
        Glib::ValueBase value;

        gtk_tree_model_get_value((GtkTreeModel*)flist->list()->gobj(), src.gobj(), i, value.gobj());

        // The const_cast is necessary as for some reason the gobj()
        // returns a const pointer. The object is not actually
        // modified by gtk_list_store_set_value thus the const_cast is
        // safe.
        gtk_list_store_set_value(m_list->gobj(), const_cast<GtkTreeIter*>(dest->gobj()), i, value.gobj());
    }
}


//// Marking and Selecting

std::vector<dir_entry*> filtered_list_controller::selected_entries() const {
    std::vector<dir_entry*> entries;

    for (dir_entry *ent : flist->selected_entries()) {
        if (m_filter(ent->context.row).first) {
            entries.push_back(ent);
        }
    }

    if (entries.empty() && selected_row) {
        dir_entry *ent = selected_row[columns.ent];

        if (ent->ent_type() != dir_entry::type_parent)
            entries.push_back(ent);
    }

    return entries;
}

void filtered_list_controller::mark_row(Gtk::TreeRow row) {
    if (row) {
        dir_entry *ent = row[columns.ent];
        flist->mark_row(ent->context.row);

        copy_columns(ent->context.row, row);
    }
}

void filtered_list_controller::on_selection_changed(Gtk::TreeRow row) {
    if (row) {
        selected_row = row;

        dir_entry *ent = row[columns.ent];
        flist->on_selection_changed(ent->context.row);
    }
}


//// List Controller Signal Handlers

void filtered_list_controller::change_model(Glib::RefPtr<Gtk::ListStore> model) {
    refilter(Gtk::TreeRow());
}
void filtered_list_controller::select_row(Gtk::TreeRow row) {
}
