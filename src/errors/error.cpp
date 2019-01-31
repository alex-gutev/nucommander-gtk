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


/**
 * Retrieves the map of default automatic error handlers from
 * settings.
 */
std::map<nuc::error, std::string> get_auto_error_handlers() {
    auto settings = app_settings::instance().settings();
    Glib::VariantContainerBase handlers; // Array of handlers

    settings->get_value("auto-error-handlers", handlers);

    std::map<error, std::string> handler_map;

    for (size_t i = 0; i < handlers.get_n_children(); ++i) {
        Glib::VariantContainerBase handler; // Current handler tuple

        handlers.get_child(handler, i);

        Glib::Variant<int> type;
        Glib::Variant<int> code;
        Glib::Variant<std::string> restart;

        handler.get_child(type, 0);
        handler.get_child(code, 1);
        handler.get_child(restart, 2);

        handler_map[error(type.get(), code.get(), false)] = restart.get();
    }

    return handler_map;
}

std::map<nuc::error, std::string> nuc::auto_error_handlers() {
    static std::map<nuc::error, std::string> handlers(get_auto_error_handlers());
    return handlers;
}
