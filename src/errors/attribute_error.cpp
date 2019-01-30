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

#include "attribute_error.h"

using namespace nuc;

Glib::ustring attribute_error::type_explanation() const noexcept {
    switch (error_type()) {
    case error::type_set_mode:
        return Glib::ustring::compose("Error setting permissions of '%1'.", file);

    case error::type_set_owner:
        return Glib::ustring::compose("Error setting owner of '%1'.", file);

    case error::type_set_times:
        return Glib::ustring::compose("Error setting access/modification time of '%1'.", file);
    }

    return "";
}
