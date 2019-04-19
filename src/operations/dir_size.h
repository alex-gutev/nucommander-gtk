/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef NUC_OPERATIONS_DIR_SIZE_H
#define NUC_OPERATIONS_DIR_SIZE_H

#include <memory>

#include "tasks/cancel_state.h"

#include "directory/dir_type.h"

/* Directory Size Operation */

namespace nuc {
    /**
     * Directory size operation callback function.
     *
     * @param size Number of files in the directory.
     */
    typedef std::function<void(size_t)> dir_size_callback;

    /**
     * Begins an operation for determining the number of files in a
     * directory.
     *
     * @param state Cancellation state.
     *
     * @param type Type of the containing directory.
     *
     * @param path Path to the directory of which to get its size.
     *
     * @param callback Callback function to call (on the main thread)
     *   once the directory's size has been determined.
     */
    void dir_size(std::shared_ptr<cancel_state> state, std::shared_ptr<dir_type> type, const pathname &path, dir_size_callback callback);
};

#endif

// Local Variables:
// mode: c++
// End:
