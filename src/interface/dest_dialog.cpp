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

#include "dest_dialog.h"

#include <exception>

#include "tasks/async_task.h"

using namespace nuc;


dest_dialog *dest_dialog::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/dest_dialog.ui");

    dest_dialog *dialog = nullptr;

    builder->get_widget_derived("dest_dialog", dialog);

    if (!dialog)
        throw std::runtime_error("No \"dest_dialog\" object in dest_dialog.ui");

    return dialog;
}

dest_dialog::dest_dialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) : Gtk::Dialog(cobject) {
    builder->get_widget("query_label", query_label);
    builder->get_widget("exec_button", exec_button);
    builder->get_widget("cancel_button", cancel_button);
    builder->get_widget("dest_entry", dest_entry);

    exec_button->signal_clicked().connect(sigc::mem_fun(this, &dest_dialog::exec_clicked));
    cancel_button->signal_clicked().connect(sigc::mem_fun(this, &dest_dialog::cancel_clicked));

    dest_entry->signal_activate().connect(sigc::mem_fun(this, &dest_dialog::exec_clicked));

    signal_delete_event().connect(sigc::mem_fun(this, &dest_dialog::on_delete));
}

int dest_dialog::run() {
    int response = Gtk::Dialog::run();
    hide();

    return response;
}

void dest_dialog::show(chose_dest_fn chose_fn) {
    dest_chosen = std::move(chose_fn);

    Gtk::Dialog::show();
    present();
}

void dest_dialog::exec_clicked() {
    if (dest_chosen)
        dest_chosen(dest_entry->get_text());

    response(Gtk::RESPONSE_OK);

    hide();
}

void dest_dialog::cancel_clicked() {
    response(Gtk::RESPONSE_CANCEL);
    hide();
}

void dest_dialog::on_show() {
    Gtk::Dialog::on_show();

    Gdk::Geometry geom{};

    geom.min_height = geom.max_height = get_height();
    geom.min_width = 0;
    geom.max_width = get_screen()->get_width();

    set_geometry_hints(*this, geom, Gdk::HINT_MAX_SIZE);

    dest_entry->grab_focus();
}

bool dest_dialog::on_delete(const GdkEventAny *e) {
    gtk_widget_hide_on_delete((GtkWidget*)this->gobj());
    return true;
}
