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


thread_local std::unordered_map<std::string, nuc::restart> nuc::restarts;

nuc::global_restart::global_restart(restart r) : name(r.name) {
    bool inserted;

    std::tie(std::ignore, inserted) = restarts.emplace(name, r);
    assert(inserted);
}

nuc::global_restart::~global_restart() {
    restarts.erase(name);
}
