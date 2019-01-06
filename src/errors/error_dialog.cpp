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

#include "error_dialog.h"

#include <exception>

#include "tasks/async_task.h"

using namespace nuc;


error_dialog *error_dialog::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/error_dialog.glade");

    error_dialog *dialog = nullptr;

    builder->get_widget_derived("error_dialog", dialog);

    if (!dialog)
        throw std::runtime_error("No \"error_dialog\" object in error_dialog.glade");

    return dialog;
}

error_dialog::error_dialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) : Gtk::Dialog(cobject) {
    builder->get_widget("actions", actions_view);
    builder->get_widget("error-message", error_label);
    builder->get_widget("exec_button", exec_button);
    builder->get_widget("all_button", all_button);

    exec_button->signal_clicked().connect(sigc::mem_fun(this, &error_dialog::exec_clicked));
    all_button->signal_clicked().connect(sigc::mem_fun(this, &error_dialog::all_clicked));

    actions_view->signal_row_activated().connect(sigc::mem_fun(this, &error_dialog::row_clicked));

    signal_delete_event().connect(sigc::mem_fun(this, &error_dialog::on_delete));

    add_events(Gdk::KEY_PRESS_MASK);

    signal_key_press_event().connect(sigc::mem_fun(this, &error_dialog::key_pressed));

    init_model();
}

void error_dialog::init_model() {
    actions = create_model();
    actions_view->set_model(actions);

    actions_view->append_column("Recovery Action", columns.name);
}

Glib::RefPtr<Gtk::ListStore> error_dialog::create_model() {
    auto list_store = Gtk::ListStore::create(columns);

    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);

    return list_store;
}


void error_dialog::show(const error &e, const restart_map &restarts, chose_action_fn chose_fn) {
    actions->clear();

    for (auto &restart : restarts) {
        if (restart.second.applicable(e)) {
            Gtk::TreeRow row = *actions->append();

            row[columns.name] = restart.first;
            row[columns.action] = &restart.second;
        }
    }

    action_chosen = chose_fn;

    set_error_label(e);

    Gtk::Dialog::show();
    present();

    actions_view->grab_focus();
}

void error_dialog::show(action_promise &promise, const error &e, const restart_map &restarts) {
    show(e, restarts, [this, &promise] (const restart *r, bool all) {
        promise.set_value(std::make_pair(r, all));
    });
}

void error_dialog::set_error_label(const nuc::error &e) {
    auto type = e.type_explanation();

    if (type.empty()) {
        error_label->set_label(e.explanation());
    }
    else {
        error_label->set_label(Glib::ustring::compose("%1\n\n%2", e.type_explanation(), e.explanation()));
    }
}


void error_dialog::exec_clicked() {
    choose_action(false);
}

void error_dialog::all_clicked() {
    choose_action(true);
}

void error_dialog::choose_action(bool all) {
    auto row = *actions_view->get_selection()->get_selected();

    if (row) {
        action_chosen(row[columns.action], all);
        hide();
    }
}

void error_dialog::row_clicked(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn *column) {
    exec_clicked();
}

bool error_dialog::on_delete(const GdkEventAny *e) {
    action_chosen(&restart_abort, false);

    gtk_widget_hide_on_delete((GtkWidget*)this->gobj());

    return true;
}

bool error_dialog::key_pressed(const GdkEventKey *e) {
    if (e->keyval == GDK_KEY_Return && e->state & GDK_SHIFT_MASK) {
        all_clicked();
        return true;
    }

    return false;
}
