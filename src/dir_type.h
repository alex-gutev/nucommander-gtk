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

#ifndef NUC_DIR_TYPE_H
#define NUC_DIR_TYPE_H

#include <tuple>
#include <functional>

#include "types.h"
#include "lister.h"
#include "dir_tree.h"
#include "dir_entry.h"

/**
 * Functions for determining the lister and directory tree (dir_tree)
 * objects for a directory.
 */

namespace nuc {
    /**
     * Lister and directory tree creation function type. Used to defer
     * creation of the objects till the task is initiated on a
     * background thread.
     *
     * Returns a pair with the first element being the lister object
     * and the second element being the dir_tree object.
     */
    typedef std::function<std::pair<lister*,dir_tree*>()> create_lister_fn;

    /**
     * Determines the lister and dir_tree objects for a given path.
     *
     * @param path The path of the 'directory'
     *
     * @return A pair where the first element is the lister object and
     *    the second element is the dir_tree object. If there is no
     *    lister object for the given path, both elements of the pair
     *    are nullptr.
     */
    std::pair<lister*, dir_tree*> get_lister(const path_str &path);

    /**
     * Determines the lister and dir_tree objects for a given entry,
     * using the attributes stored in the dir_entry object.
     *
     * @param ent The dir_entry object of the entry
     *
     * @return A function which when invoked returns the lister and
     *    dir_tree objects. If there is no lister object for the
     *    entry, an empty std::function is returned.
     */
    create_lister_fn get_lister_fn(const dir_entry &ent);
}


#endif // NUC_DIR_TYPE_H 


/* Local Variables: */
/* mode: c++ */
/* indent-tabs-mode: nil */
/* End: */
