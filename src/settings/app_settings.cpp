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

#include "app_settings.h"

using namespace nuc;

app_settings &app_settings::instance() {
    static app_settings inst;
    return inst;
}

app_settings::app_settings() : m_settings(Gio::Settings::create(settings_id)) {
    m_dir_refresh_timeout = m_settings->get_int("dir-refresh-timeout");
}


int app_settings::dir_refresh_timeout() const {
    return m_dir_refresh_timeout;
}

void app_settings::dir_refresh_timeout(int timeout) {
    m_settings->set_int("dir-refresh-timeout", timeout);
    m_dir_refresh_timeout = timeout;
}


std::vector<std::string> app_settings::columns() const {
    return m_settings->get_string_array("columns");
}

void app_settings::columns(const std::vector<std::string> &columns) {
    m_settings->set_string_array("columns", columns);
}


std::string app_settings::default_sort_column() const {
    return m_settings->get_string("default-sort-column");
}

void app_settings::default_sort_column(const std::string &column) {
    m_settings->set_string("default-sort-column", column);
}


std::map<Glib::ustring, Glib::ustring> app_settings::keybindings() const {
    Glib::Variant<std::map<Glib::ustring, Glib::ustring>> gv_map;

    m_settings->get_value("keybindings", gv_map);

    return gv_map.get();
}
