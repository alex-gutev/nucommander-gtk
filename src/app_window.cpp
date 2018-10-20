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

#include "app_window.h"

#include <exception>
#include <functional>
#include <iostream>

#include "tasks/async_task.h"

#include "commands/commands.h"

#include "copy/copy.h"

using namespace nuc;

app_window* app_window::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/window.glade");

    app_window *window = nullptr;

    builder->get_widget_derived("commander_window", window);

    if (!window)
        throw std::runtime_error("No \"commander_window\" object in window.glade");

    return window;
}

app_window::app_window(Gtk::ApplicationWindow::BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder> &builder)
    : Gtk::ApplicationWindow(cobject), builder(builder) {

    // TODO: Add error checking

    set_default_size(800, 600);

    builder->get_widget("pane_view", pane_view);

    init_pane_view();
}

app_window::~app_window() {
    if (err_dialog)
        delete err_dialog;
}

void app_window::init_pane_view() {
    set_focus_chain({pane_view});

    add_file_view(left_view, 1);
    add_file_view(right_view, 2);

    left_view->next_file_view = right_view;
    right_view->next_file_view = left_view;

    left_view->path("/");
    right_view->path("/");

    pane_view->set_focus_chain({left_view, right_view});
    pane_view->show_all();
}


void app_window::add_file_view(nuc::file_view* & ptr, int pane) {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/fileview.glade");

    // TODO: Add error checking

    builder->get_widget_derived("file_view", ptr);

    if (pane == 1) {
        pane_view->pack1(*ptr, true, true);
    }
    else {
        pane_view->pack2(*ptr, true, true);
    }

    ptr->add_events(Gdk::KEY_RELEASE_MASK);
    ptr->signal_key_release_event().connect(sigc::bind<file_view*>(sigc::mem_fun(this, &app_window::on_keypress), ptr), false);
}


Glib::RefPtr<Gtk::Builder> app_window::file_view_builder() {
    return Gtk::Builder::create_from_resource("/org/agware/nucommander/fileview.glade");
}

bool app_window::on_keypress(const GdkEventKey *e, file_view *src) {
    using namespace std::placeholders;

    std::string command_name = command_keymap::instance().command_name(e);

    auto command = commands.find(command_name);

    if (command != commands.end()) {
        command->second(this, src);
        return true;
    }

    return false;
};

void app_window::add_operation(task_queue::task_type op) {
    using namespace std::placeholders;

    operations->add(with_error_handler(std::move(op), std::bind(&app_window::handle_error, this, _1)));
}

void app_window::handle_error(const error &e) {
    auto &actions = restarts();

    std::promise<const restart *> promise;

    dispatch_main([&] {
        show_error(promise, e, actions);
    });

    const restart &r = *promise.get_future().get();
    r(e);
}


/// Error Dialog

void app_window::create_error_dialog() {
    if (!err_dialog) {
        err_dialog = error_dialog::create();

        err_dialog->set_transient_for(*this);
    }
}

void app_window::show_error(std::promise<const restart *> &promise, const nuc::error &err, const restart_map &restarts) {
    create_error_dialog();

    err_dialog->show(promise, err, restarts);
    err_dialog->present();
}
