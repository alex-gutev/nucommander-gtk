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

#include <giomm/appinfo.h>

#include "tasks/async_task.h"
#include "commands/commands.h"
#include "operations/copy.h"

#include "operations/dir_size.h"
#include "file_list/directory_buffers.h"

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

    add_events(Gdk::FOCUS_CHANGE_MASK);

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


    // Create file_list_controllers for both panes

    left_view->file_list(directory_buffers::instance().new_buffer());
    right_view->file_list(directory_buffers::instance().new_buffer());

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

    ptr->add_events(Gdk::KEY_PRESS_MASK);
    ptr->signal_key_press().connect(sigc::bind<file_view*>(sigc::mem_fun(this, &app_window::on_keypress), ptr), false);

    ptr->signal_activate_entry().connect(sigc::mem_fun(this, &app_window::on_entry_activate));
}


Glib::RefPtr<Gtk::Builder> app_window::file_view_builder() {
    return Gtk::Builder::create_from_resource("/org/agware/nucommander/fileview.glade");
}

bool app_window::on_keypress(const GdkEventKey *e, file_view *src) {
    return command_keymap::instance().exec_command(this, src, e);
}

void app_window::on_entry_activate(nuc::file_view *src, nuc::file_list_controller *flist, nuc::dir_entry *ent) {
    using namespace std::placeholders;

    if (!flist->descend(*ent)) {
        dir_type type = flist->dir_vfs()->directory_type();

        if (!type.is_dir()) {
            add_operation(make_unpack_task(type, ent->orig_subpath(), std::bind(&app_window::open_file, this, _1)));
        }
        else {
            paths::pathname full_path = flist->path().append(ent->orig_subpath());
            add_operation([=] (cancel_state &) {
                open_file(full_path.c_str());
            });
        }
    }
}

void app_window::open_file(const std::string &path) {
    try {
        Gio::AppInfo::launch_default_for_uri(Gio::File::create_for_path(path)->get_uri());
    }
    catch (Gio::Error) {
        // For now do nothing as there isn't much that can be done
        // other than letting the user manually retry.
    }
}

void app_window::add_operation(task_queue::task_type op) {
    using namespace std::placeholders;

    operations->add(with_error_handler(std::move(op), error_handler(this)),
                    std::bind(&app_window::on_operation_finish, this, _1));
}

void app_window::add_operation(const task_queue::task_type &op, const progress_event::callback &progress) {
    add_operation([=] (cancel_state &state) {
        state.no_cancel([&] {
            state.progress = progress;
        });

        op(state);
    });
}

/// Error Handlers

void app_window::error_handler::operator()(cancel_state &state, const nuc::error &e) {
    auto it = chosen_actions.find(e);
    auto &actions = restarts();

    if (it != chosen_actions.end()) {
        auto r_it = actions.find(it->second);

        if (r_it != actions.end()) {
            r_it->second(e);
            return;
        }
    }

    const restart *r;
    bool all;

    state.no_cancel([&] {
        std::tie(r, all) = window->choose_action(e);
    });

    if (all) {
        chosen_actions[e] = r->name;
    }

    (*r)(e);
}

std::pair<const restart *, bool> app_window::choose_action(const error &e) {
    auto &actions = restarts();

    error_dialog::action_promise promise;

    dispatch_main([&] {
        show_error(promise, e, actions);
    });

    return promise.get_future().get();
}


/// Error Dialog

void app_window::create_error_dialog() {
    if (!err_dialog) {
        err_dialog = error_dialog::create();

        err_dialog->set_transient_for(*this);
    }
}

void app_window::show_error(error_dialog::action_promise &promise, const nuc::error &err, const restart_map &restarts) {
    create_error_dialog();

    err_dialog->show(promise, err, restarts);
    err_dialog->present();
}


//// Destination Dialog

nuc::dest_dialog *app_window::dest_dialog() {
    if (!m_dest_dialog) {
        m_dest_dialog = dest_dialog::create();
        m_dest_dialog->set_transient_for(*this);
    }

    return m_dest_dialog;
}


//// Progress

nuc::progress_dialog *app_window::progress_dialog() {
    if (!m_progress_dialog) {
        m_progress_dialog = progress_dialog::create();
        m_progress_dialog->set_transient_for(*this);
        m_progress_dialog->signal_response().connect(sigc::mem_fun(this, &app_window::on_prog_dialog_response));
    }

    return m_progress_dialog;
}
void app_window::on_prog_dialog_response(int id) {
    if (id == Gtk::RESPONSE_CANCEL) {
        operations->cancel();
    }
}

void app_window::on_operation_finish(bool cancelled) {
    dispatch_main([this] {
        progress_dialog()->hide();
    });
}


void app_window::progress_fn::operator()(const nuc::progress_event &e) {
    switch (e.type) {
    case progress_event::type_begin:
        dialog->show();
        dialog->present();
        break;

    case progress_event::type_finish:
        dialog->hide();
        break;

    case progress_event::type_enter_file:
        if (!depth)
            dialog->hide_dir();

        dialog->set_file_label(e.file.path());
        dialog->set_file_size(e.bytes);
        dialog->file_progress(0);
        break;

    case progress_event::type_process_data:
        dialog->file_progress(dialog->file_progress() + e.bytes);
        break;

    case progress_event::type_exit_file:
        dialog->dir_progress(dialog->dir_progress() + 1);
        break;

    case progress_event::type_enter_dir:
        if (!depth) {
            dialog->show_dir();
            dialog->dir_progress(0);
            dialog->set_dir_size(0);
            dialog->set_dir_label(e.file.path());

            get_dir_size(e.file);
        }
        depth++;
        break;

    case progress_event::type_exit_dir:
        if (!--depth) {
            dir_size_state->cancel();
        }
        break;
    }
}

void app_window::progress_fn::get_dir_size(const paths::pathname &dir) {
    using namespace std::placeholders;

    dir_size_state = std::make_shared<cancel_state>();

    dir_size(dir_size_state, type, dir, std::bind(&app_window::progress_fn::got_dir_size, this, _1));
}

void app_window::progress_fn::got_dir_size(size_t size) {
    nfiles = size;

    dialog->set_dir_size(size);
}


progress_event::callback app_window::get_progress_fn(const dir_type &type) {
    std::shared_ptr<progress_fn> fn = std::make_shared<progress_fn>(progress_dialog(), type);

    return [=] (const progress_event &e) {
        dispatch_main([=] {
            (*fn)(e);
        });
    };
}

//// Open Directories Popup

nuc::open_dirs_popup *app_window::open_dirs_popup() {
    if (!m_open_dirs_popup) {
        m_open_dirs_popup = open_dirs_popup::create();
        m_open_dirs_popup->set_transient_for(*this);
    }

    m_open_dirs_popup->set_dirs(directory_buffers::instance().buffers());

    return m_open_dirs_popup;
}
