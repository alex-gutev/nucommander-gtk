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

#include "nucommander.h"

#include <functional>

#include <sigc++/sigc++.h>
#include <gtkmm/styleprovider.h>
#include <gtkmm/cssprovider.h>

#include "app_window.h"

#include "tasks/async_task.h"
#include "commands/commands.h"

#include "interface/prefs_window.h"

Glib::RefPtr<nuc::NuCommander> nuc::NuCommander::instance() {
    static Glib::RefPtr<nuc::NuCommander> app = create();
    return app;
}

Glib::RefPtr<nuc::NuCommander> nuc::NuCommander::create() {
    auto app = Glib::RefPtr<NuCommander>(new NuCommander());

    // Add stylesheet

    auto provider = Gtk::CssProvider::create();
    provider->load_from_resource("/org/agware/nucommander/styles.css");

    Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), provider, 600);

    return app;
}


nuc::NuCommander::NuCommander() : Gtk::Application("org.agware.nucommander") {}


void nuc::NuCommander::on_startup() {
    Gtk::Application::on_startup();

    add_actions();
    set_menu();
}

void nuc::NuCommander::on_activate() {
    // Initialize Threading
    init_threads();

    // Initialize Commands
    command_keymap::instance();

    auto window = create_app_window();
    window->present();
}


void nuc::NuCommander::add_actions() {
    add_action("quit", sigc::mem_fun(this, &NuCommander::quit));
    add_action("preferences", sigc::ptr_fun(preferences));
    add_action("about", sigc::mem_fun(this, &NuCommander::show_about));
}

void nuc::NuCommander::set_menu() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/main_menu.ui");

    auto object = builder->get_object("appmenu");
    auto menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

    set_app_menu(menu);
}


nuc::app_window *nuc::NuCommander::create_app_window() {
    auto window = app_window::create();

    add_window(*window);

    window->signal_hide().connect(sigc::bind<app_window*>(sigc::mem_fun(*this, &NuCommander::on_hide_window), window));

    return window;
}

void nuc::NuCommander::on_hide_window(app_window *window) {
    window->cleanup([=] {
        delete window;
    });
}


void nuc::NuCommander::quit() {
    auto windows = get_windows();

    for (auto window : windows)
        window->hide();

    Gtk::Application::quit();
}


static void on_about_response(Gtk::Dialog *dialog, int response_id) {
    switch (response_id) {
    case Gtk::RESPONSE_OK:
    case Gtk::RESPONSE_CANCEL:
    case Gtk::RESPONSE_DELETE_EVENT:
        dialog->hide();
        break;
    }
}

void nuc::NuCommander::show_about() {
    using namespace std::placeholders;

    if (!about) {
        about.reset(new Gtk::AboutDialog());

        about->set_program_name("NuCommander");
        about->set_version("0.1");
        about->set_copyright("Alexander Gutev");
        about->set_comments("A fast small orthodox file manager.");

        auto license = Gio::Resource::lookup_data_global("/org/agware/nucommander/license.txt");
        gsize size;

        about->set_license((const char *)license->get_data(size));

        about->set_website("https://alex-gutev.github.io/nucommander-gtk/");
        about->set_website_label("NuCommander Website");
        about->set_authors({"Alexander Gutev"});

        about->signal_response().connect(std::bind(on_about_response, about.get(), _1));
    }

    about->show();
    about->present();
}

void nuc::NuCommander::preferences() {
    auto *window = prefs_window::instance();

    window->show();
    window->present();
}
