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

#ifndef NUC_COPY_MOVE_H
#define NUC_COPY_MOVE_H

#include "lister/tree_lister.h"
#include "stream/dir_writer.h"

#include "tasks/task_queue.h"
#include "directory/dir_type.h"

/**
 * File moving functions.
 */

namespace nuc {
    /**
     * Creates a move task.
     *
     * @param src_type Type of the directory, containing the files to
     *    be moved.
     *
     * @param entries Directory entries, within the source directory,
     *    to move.
     *
     * @param dest Path to the destination directory.
     *
     * @return A task which moves the entries @a entries, located in
     *    the directory @a src_type, to the destination directory with
     *    writer @a dest.
     */
    task_queue::task_type make_move_task(dir_type src_type, const std::vector<dir_entry*> &entries, const paths::string &dest);

    /**
     * Moves the files returned by the tree lister @a lister to the
     * destination directory.
     *
     * @param state Cancellation state.
     * @param lister Source directory tree lister.
     * @param dest Destination directory writer.
     */
	void move(cancel_state &state, const std::vector<paths::pathname> &paths, const paths::string &dest, dir_writer &dir);
}

#endif

// Local Variables:
// mode: c++
// End:
