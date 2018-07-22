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

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <unordered_map>

#include <stdint.h>

#include <gtkmm/liststore.h>

#include "paths/utils.h"

/**
 * Contains type alias used throughout the project.
 */

namespace nuc {
    /**
     * File map type.
     */
    template <typename T>
    using file_map = std::unordered_multimap<paths::string, T>;

    /**
     * Context data type for the dir_entry class.
     */
    struct dir_entry_context {
        Gtk::TreeRow row;
    };
}

#endif // TYPES_H

// Local Variables:
// mode: c++
// End:
