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

#include "tree_lister.h"

using namespace nuc;


void tree_lister::add_list_callback(const list_callback &fn) {
    if (list_fn) {
        auto old_fn = list_fn;

        list_fn = [=] (const lister::entry &ent, const struct stat *st, visit_info info) {
            return fn(ent, st, info) && old_fn(ent, st, info);
        };
    }
    else {
        list_fn = fn;
    }
}

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
