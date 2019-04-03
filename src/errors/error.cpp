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

#include "error.h"

#include <cassert>
#include <cstring>

#include "settings/app_settings.h"

using namespace nuc;


/**
 * Map of established restarts (error handler functions)
 * where the keys are the restart identifiers and the
 * corresponding values are the restarts.
 */
static thread_local nuc::restart_map global_restart_map = nuc::restart_map({ std::make_pair("abort", nuc::restart_abort) });

thread_local nuc::error_handler_fn nuc::global_error_handler = [] (const error &e) {
    restart_abort(e);
};


nuc::restart_map &nuc::restarts() {
    return global_restart_map;
}

nuc::global_restart::global_restart(restart r) : name(r.name) {
    bool inserted;

    std::tie(std::ignore, inserted) = restarts().emplace(name, r);
    assert(inserted);
}

nuc::global_restart::~global_restart() {
    restarts().erase(name);
}

Glib::ustring nuc::error::explanation() const noexcept {
    // strerror is not thread-safe thus renders the entire method not
    // thread-safe.

    return error_string.empty() ? strerror(m_code) : error_string;
}


std::unordered_map<std::string, int> &nuc::error_type_map() {
    static std::unordered_map<std::string, int> map({
            std::make_pair("general", error::type_general),

            std::make_pair("create-file", error::type_create_file),
            std::make_pair("write-file", error::type_write_file),
            std::make_pair("read-file", error::type_read_file),
            std::make_pair("rename-file", error::type_rename_file),
            std::make_pair("delete-file", error::type_delete_file),

            std::make_pair("create-dir", error::type_create_dir),
            std::make_pair("set-mode", error::type_set_mode),
            std::make_pair("set-owner", error::type_set_owner),
            std::make_pair("set-times", error::type_set_times)
    });

    return map;
}

std::unordered_map<std::string, int> &nuc::error_code_map() {
    static std::unordered_map<std::string, int> map;

    return map;
}

/**
 * Retrieves the error type code corresponding to the error type
 * identifier @a type.
 *
 * @return The error type code or -1 if there is no such error type.
 */
static int get_error_type(const std::string &type) {
    auto &map = error_type_map();
    auto it = map.find(type);

    if (it != map.end()) {
        return it->second;
    }

    return -1;
}

/**
 * Retrieves the map of default automatic error handlers from
 * settings.
 */
static std::map<nuc::error, std::string> get_auto_error_handlers() {
    auto settings = app_settings::instance().settings();
    Glib::VariantContainerBase handlers; // Array of handlers

    settings->get_value("auto-error-handlers", handlers);

    std::map<error, std::string> handler_map;

    for (size_t i = 0; i < handlers.get_n_children(); ++i) {
        Glib::VariantContainerBase handler; // Current handler tuple

        handlers.get_child(handler, i);

        Glib::Variant<std::string> vtype;
        Glib::Variant<int> code;
        Glib::Variant<std::string> restart;

        handler.get_child(vtype, 0);
        handler.get_child(code, 1);
        handler.get_child(restart, 2);

        int type = get_error_type(vtype.get());

        if (type >= 0)
            handler_map[error(type, code.get(), false)] = restart.get();
    }

    return handler_map;
}

std::map<nuc::error, std::string> nuc::auto_error_handlers() {
    static std::map<nuc::error, std::string> handlers(get_auto_error_handlers());
    return handlers;
}
