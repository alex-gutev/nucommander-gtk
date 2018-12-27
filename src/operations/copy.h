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

#ifndef NUC_COPY_H
#define NUC_COPY_H

#include "util/util.h"

#include "lister/tree_lister.h"
#include "stream/dir_writer.h"

#include "tasks/task_queue.h"
#include "directory/dir_type.h"

/**
 * File copying functions.
 */

namespace nuc {
    /**
     * Creates a copy task.
     *
     * @param src_type Type of the directory, containing the files to
     *    be copied.
     *
     * @param entries Directory entries, within the source directory,
     *    to copy.
     *
     * @param dest Path to the destination directory.
     *
     * @return A task which copies the entries @a entries, located in
     *    the directory @a src_type, to the destination directory with
     *    writer @a dest.
     */
    task_queue::task_type make_copy_task(dir_type src_type, const std::vector<dir_entry*> &entries, const paths::string &dest);

    /**
     * Returns the subpaths of the entries which should be visited by
     * a tree lister.
     *
     * If an entry is a directory a trailing slash is appended to the
     * subpath.
     *
     * @param entries Array of the entries which should be visited.
     *
     * @return Array of subpaths.
     */
    std::vector<paths::pathname> lister_paths(const std::vector<dir_entry*> &entries);

    /**
     * Function which returns the name of the file to which a file
     * should be copied.
     *
     * @param name The name of the original file.
     *
     * @return The name of the file to which the original file should
     *   be copied.
     */
    typedef std::function<paths::string(const paths::string &)> map_name_fn;

    /**
     * Determines whether the destination path is the path to the
     * destination directory where the source files should be
     * copied/moved or, in the case of a single file, the name of the
     * file to which the source file should be copied/moved.
     *
     * @param dest The destination path.
     * @param paths Paths of the files being copied/moved.
     *
     * @return A pair where the first element is the path to the
     *   destination directory and the second element is the function
     *   which maps source file names to destination file names.
     */
    std::pair<paths::string, map_name_fn> determine_dest_dir(const paths::pathname &dest, const std::vector<paths::pathname> &paths);

    /**
     * Copies the files returned by the tree lister @a lister to the
     * destination directory, with directory writer @a dest.
     *
     * @param state Cancellation state.
     *
     * @param lister Source directory tree lister.
     *
     * @param dest Destination directiory writer.
     *
     * @param new_name A function, which is called on the name of each
     *   file, and should return the name of the file to which the
     *   file should be copied.
     */
	void copy(cancel_state &state, tree_lister &lister, dir_writer &dest, const map_name_fn &new_name = identity());

    /**
     * Creates a task which copies a file from a source directory to a
     * temporary file. This primarily useful to unpack files from
     * archives before opening them in an external program.
     *
     * @param src_type The directory type of the source directory.
     *
     * @param subpath Subpath of the file, within the source
     *    directory.
     *
     * @param callback A callback that is called when the operation
     *    completes (if it completes successfully), with the path to
     *    the temporary file passed as an argument.
     *
     * @return The task.
     */
    task_queue::task_type make_unpack_task(const dir_type &src_type, const paths::pathname &subpath, const std::function<void(const char *)> &callback);
}

#endif

// Local Variables:
// mode: c++
// End:
