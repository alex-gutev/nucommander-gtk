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

#include "open_dirs_popup.h"

using namespace nuc;

open_dirs_popup::model_columns::model_columns() {
    add(path);
    add(file_list);
}

open_dirs_popup *open_dirs_popup::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/open_dirs_popup.glade");

    open_dirs_popup *window = nullptr;

    builder->get_widget_derived("dirs_popup", window);

    if (!window)
        throw std::runtime_error("No \"dirs_popup\" object in open_dirs_popup.glade");

    return window;
}

open_dirs_popup::open_dirs_popup(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) : Gtk::Window(cobject) {
    // Get TreeView Widget

    builder->get_widget("dir_list", dirs_view);

    // Create ListStore Model

    dirs_list = Gtk::ListStore::create(model);
    dirs_list->set_sort_column(model.path, Gtk::SortType::SORT_ASCENDING);

    // Initialize Tree View

    dirs_view->set_model(dirs_list);
    dirs_view->append_column("Directory", model.path);
    dirs_view->get_column(0)->set_sort_column(model.path);

    // Connect Signal Handlers

    dirs_view->signal_row_activated().connect(sigc::mem_fun(this, &open_dirs_popup::dir_activated));
}

void open_dirs_popup::set_dirs(const std::vector<std::shared_ptr<file_list_controller> > &dirs) {
    dirs_list->clear();

    for (const auto &flist : dirs) {
        if (!flist->attached()) {
            auto row = *dirs_list->append();

            row[model.path] = flist->path().path();
            row[model.file_list] = flist;
        }
    }
}

void open_dirs_popup::dir_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column) {
    auto row = *dirs_list->get_iter(path);

    if (row) {
        hide();
        m_dir_chosen(row[model.file_list]);
    }
}

bool open_dirs_popup::on_delete(const GdkEventAny *e) {
    gtk_widget_hide_on_delete((GtkWidget*)this->gobj());
    return true;
}

bool open_dirs_popup::on_key_press_event(GdkEventKey *e) {
    if (!Gtk::Window::on_key_press_event(e)) {
        if (e->keyval == GDK_KEY_Escape) {
            hide();
            return true;
        }
    }

    return false;
}
