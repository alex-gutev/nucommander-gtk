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

#include "create_error.h"

#include <error.h>

using namespace nuc;


Glib::ustring create_error::explanation() const noexcept {
    switch (code()) {
    case EACCES:
        return Glib::ustring::compose("Write access to '%1' denied.", file);

    case EEXIST:
        return Glib::ustring::compose("Destination file '%1' exists.", file);

    case EISDIR:
        return Glib::ustring::compose("Destination file '%1' is a directory.", file);

    case ENOENT:
        return Glib::ustring::compose("Directory '%1' does not exist.", paths::removed_last_component(file));

    case ENOTDIR:
        return Glib::ustring::compose("'%1' is not a directory.", paths::removed_last_component(file));

    case ETXTBSY:
        return Glib::ustring::compose("Can't write to '%1' as it is a currently running executable.", file);

    default:
        return nuc::error::explanation();
    }
}
