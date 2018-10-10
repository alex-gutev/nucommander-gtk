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

/**
 * Map of established restarts (error handler functions)
 * where the keys are the restart identifiers and the
 * corresponding values are the restarts.
 */
static thread_local nuc::restart_map global_restart_map = nuc::restart_map({ std::make_pair("abort", nuc::restart_abort) });

thread_local nuc::error_handler_fn nuc::global_error_handler = nuc::error_handler_fn();


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

std::string nuc::error::explanation() const noexcept {
    std::string msg("Error with code: ");
    msg += std::to_string(m_code);

    return msg;
}
