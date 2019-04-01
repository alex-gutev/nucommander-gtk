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

#include "prefs_window.h"

#include "settings/app_settings.h"

#include <map>

#include <glib/gi18n.h>

using namespace nuc;

prefs_window *prefs_window::instance() {
    static prefs_window *inst = prefs_window::create();
    return inst;
}

prefs_window *prefs_window::create() {
    auto builder = Gtk::Builder::create_from_resource("/org/agware/nucommander/prefs_window.glade");

    prefs_window *window = nullptr;

    builder->get_widget_derived("prefs_window", window);

    if (!window)
        throw std::runtime_error("No \"key_prefs_dialog\" object in key_pref_dialog.glade");

    return window;
}

prefs_window::prefs_window(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder) : Gtk::Window(cobject) {
    // Get Widgets

    builder->get_widget("ok_button", ok_button);
    builder->get_widget("apply_button", apply_button);
    builder->get_widget("cancel_button", cancel_button);

    // Initialize Buttons

    apply_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::apply_clicked));
    ok_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::ok_clicked));
    cancel_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::cancel_clicked));

    init_general(builder);
    init_keybindings(builder);
    init_plugins(builder);
}


void prefs_window::show() {
    if (!is_visible()) {
        get_general_settings();
        get_bindings();
        get_plugins();

        Gtk::Window::show();
    }
}


//// General Settings

void prefs_window::init_general(const Glib::RefPtr<Gtk::Builder> &builder) {
    builder->get_widget("refresh_timeout_entry", refresh_timeout_entry);

    refresh_timeout_entry->set_range(100, 10000);
    refresh_timeout_entry->set_increments(100, 1000);
}

void prefs_window::get_general_settings() {
    refresh_timeout_entry->set_value(app_settings::instance().dir_refresh_timeout());
}

void prefs_window::store_general_settings() {
    app_settings::instance().dir_refresh_timeout(refresh_timeout_entry->get_value_as_int());
}


//// Keybinding Settings

void prefs_window::init_keybindings(const Glib::RefPtr<Gtk::Builder> &builder) {
    // Get Widgets

    builder->get_widget("bindings_tree_view", bindings_view);
    builder->get_widget("kb_add_button", kb_add_button);
    builder->get_widget("kb_remove_button", kb_remove_button);

    // Create List Model

    bindings_list = Gtk::ListStore::create(kb_model);
    bindings_list->set_sort_column(kb_model.command, Gtk::SortType::SORT_ASCENDING);

    // Initialize Tree View

    bindings_view->set_model(bindings_list);

    bindings_view->append_column_editable("Command", kb_model.command);
    bindings_view->append_column_editable("Shortcut", kb_model.key);

    bindings_view->get_column(0)->set_sort_column(kb_model.command);
    bindings_view->get_column(1)->set_sort_column(kb_model.key);

    bindings_view->get_column(0)->set_resizable(true);
    bindings_view->get_column(1)->set_resizable(true);

    // Initialize Buttons

    kb_add_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::add_binding));
    kb_remove_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::remove_binding));
}


prefs_window::kb_model_columns::kb_model_columns() {
    add(command);
    add(key);
}

void prefs_window::get_bindings() {
    Glib::Variant<std::map<Glib::ustring, Glib::ustring>> key_map;

    app_settings::instance().settings()->get_value("keybindings", key_map);

    bindings_list->clear();

    for (auto binding : key_map.get()) {
        Gtk::TreeRow row = *bindings_list->append();

        row[kb_model.key] = binding.first;
        row[kb_model.command] = binding.second;
    }
}

void prefs_window::store_bindings() {
    std::map<Glib::ustring, Glib::ustring> key_map;

    for (auto row : bindings_list->children()) {
        key_map[row[kb_model.key]] = row[kb_model.command];
    }

    app_settings::instance().settings()->set_value("keybindings", Glib::Variant<std::map<Glib::ustring, Glib::ustring>>::create(key_map));
}


void prefs_window::add_binding() {
    auto row = bindings_list->append();

    bindings_view->set_cursor(bindings_list->get_path(row), *bindings_view->get_column(0), true);
}

void prefs_window::remove_binding() {
    if (auto row = bindings_view->get_selection()->get_selected())
        bindings_list->erase(row);
}


//// Plugin Settings

prefs_window::plugin_model_columns::plugin_model_columns() {
    add(path);
    add(regex);
}

void prefs_window::init_plugins(const Glib::RefPtr<Gtk::Builder> &builder) {
    // Get Widgets

    builder->get_widget("plugins_treeview", plugins_view);
    builder->get_widget("plugin_add_button", plugin_add_button);
    builder->get_widget("plugin_remove_button", plugin_remove_button);

    // Create List Store Model

    plugins_list = Gtk::ListStore::create(plugin_model);
    plugins_list->set_sort_column(plugin_model.path, Gtk::SortType::SORT_ASCENDING);

    // Initialize Tree View

    plugins_view->set_model(plugins_list);

    plugins_view->append_column_editable(_("Plugin Path"), plugin_model.path);
    plugins_view->append_column_editable(_("Regex"), plugin_model.regex);

    plugins_view->get_column(0)->set_sort_column(plugin_model.path);
    plugins_view->get_column(1)->set_sort_column(plugin_model.regex);

    plugins_view->get_column(0)->set_resizable(true);
    plugins_view->get_column(1)->set_resizable(true);

    // Initialize Buttons

    plugin_add_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::add_plugin));
    plugin_remove_button->signal_clicked().connect(sigc::mem_fun(this, &prefs_window::remove_plugin));
}

void prefs_window::get_plugins() {
    Glib::Variant<std::vector<std::pair<Glib::ustring, Glib::ustring>>> plugins;
    app_settings::instance().settings()->get_value("plugins", plugins);

    plugins_list->clear();

    for (auto plugin : plugins.get()) {
        Gtk::TreeRow row = *plugins_list->append();

        row[plugin_model.path] = plugin.first;
        row[plugin_model.regex] = plugin.second;
    }
}

void prefs_window::store_plugins() {
    std::vector<std::tuple<Glib::ustring, Glib::ustring>> plugins;

    for (auto row : plugins_list->children()) {
        plugins.emplace_back(row[plugin_model.path], row[plugin_model.regex]);
    }

    app_settings::instance().settings()->set_value("plugins", Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>::create(plugins));
}

void prefs_window::add_plugin() {
    auto row = plugins_list->append();

    plugins_view->set_cursor(plugins_list->get_path(row), *plugins_view->get_column(0), true);
}

void prefs_window::remove_plugin() {
    if (auto row = plugins_view->get_selection()->get_selected())
        plugins_list->erase(row);
}


//// Window Signal Handlers

void prefs_window::apply_clicked() {
    store_general_settings();
    store_bindings();
    store_plugins();
}

void prefs_window::ok_clicked() {
    apply_clicked();
    hide();
}

void prefs_window::cancel_clicked() {
    hide();
}


bool prefs_window::on_delete(const GdkEventAny *e) {
    gtk_widget_hide_on_delete((GtkWidget*)this->gobj());
    return true;
}

bool prefs_window::on_key_press_event(GdkEventKey *e) {
    if (!Gtk::Window::on_key_press_event(e)) {
        switch (e->keyval) {
        case GDK_KEY_Escape:
            cancel_clicked();
            return true;
        }
    }

    return false;
}
