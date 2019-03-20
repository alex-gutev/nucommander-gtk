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

#ifndef NUC_COPY_DELETE_H
#define NUC_COPY_DELETE_H

#include "lister/tree_lister.h"
#include "stream/dir_writer.h"

#include "tasks/task_queue.h"
#include "directory/dir_type.h"

/**
 * Delete task.
 */
namespace nuc {
    /**
     * Creates a delete task.
     *
     * @param src_type Type of the directory, containing the files to
     *    be deleted.
     *
     * @param entries Directory entries, within the source directory,
     *    to deleted.
     *
     * @return A task which deletes the entries @a entries, located in
     *    the directory @a src_type.
     */
    task_queue::task_type make_delete_task(std::shared_ptr<dir_type> src_type, const std::vector<dir_entry*> &entries);
}

#endif

// Local Variables:
// mode: c++
// End:
