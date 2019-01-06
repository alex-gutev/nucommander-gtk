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

#include "progress_dialog.h"

#include <exception>


using namespace nuc;


progress_dialog *progress_dialog::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/progress_dialog.glade");

    progress_dialog *dialog = nullptr;

    builder->get_widget_derived("progress_dialog", dialog);

    if (!dialog)
        throw std::runtime_error("No \"progress_dialog\" object in progress_dialog.glade");

    return dialog;
}

progress_dialog::progress_dialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) : Gtk::Dialog(cobject) {
    builder->get_widget("file_label", file_label);
    builder->get_widget("file_progressbar", file_progressbar);

    builder->get_widget("dir_label", dir_label);
    builder->get_widget("dir_progressbar", dir_progressbar);

    builder->get_widget("cancel_button", cancel_button);
    builder->get_widget("hide_button", hide_button);

    builder->get_widget("box", box);

    cancel_button->signal_clicked().connect(sigc::mem_fun(this, &progress_dialog::cancel_clicked));
    hide_button->signal_clicked().connect(sigc::mem_fun(this, &progress_dialog::hide_clicked));

    signal_delete_event().connect(sigc::mem_fun(this, &progress_dialog::on_delete));
}

void progress_dialog::cancel_clicked() {
    response(Gtk::RESPONSE_CANCEL);
}

void progress_dialog::hide_clicked() {
    hide();
}

void progress_dialog::on_show() {
    Gtk::Dialog::on_show();

    Gdk::Geometry geom{};

    geom.min_height = geom.max_height = get_height();
    geom.min_width = 0;
    geom.max_width = get_screen()->get_width();

    set_geometry_hints(*this, geom, Gdk::HINT_MAX_SIZE);
}

bool progress_dialog::on_delete(const GdkEventAny *e) {
    gtk_widget_hide_on_delete((GtkWidget*)this->gobj());
    return true;
}

void progress_dialog::hide_dir() {
    dir_label->hide();
    dir_progressbar->hide();

    int min, max;

    box->get_preferred_height(min, max);
    resize(get_width(), min);
}

void progress_dialog::show_dir() {
    dir_label->show();
    dir_progressbar->show();
}
