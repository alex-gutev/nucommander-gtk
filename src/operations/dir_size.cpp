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

#include "dir_size.h"

#include <memory>

#include "tasks/async_task.h"

using namespace nuc;

/**
 * Count the number of files in a directory.
 *
 * @param state Cancellation state.
 * @param type Type of the containing directory.
 * @param dir Subpath to the directory.
 *
 * @return The number of files in the directory.
 */
static size_t count_files(std::shared_ptr<cancel_state> state, std::shared_ptr<dir_type> type, const pathname &dir);

/**
 * Calls the callback function, with the number of files, on the main
 * thread. The call is performed in the no_cancel state.
 *
 * @param state Cancellation state.
 * @param callback The callback function.
 * @param nfiles The number of files.
 */
static void call_callback(std::shared_ptr<cancel_state> state, dir_size_callback callback, size_t nfiles);

void nuc::dir_size(std::shared_ptr<cancel_state> state, std::shared_ptr<dir_type> type, const pathname &dir, dir_size_callback callback) {
    using namespace std::placeholders;

    dispatch_async([=] {
        try {
            size_t files = count_files(state, type, dir);

            call_callback(state, callback, files);
        }
        catch (const cancel_state::cancelled &) {
            // Operation Cancelled
        }
        catch (const error &) {
            // Operation Aborted
        }
    });
}

size_t count_files(std::shared_ptr<cancel_state> state, std::shared_ptr<dir_type> type, const pathname &dir) {
    std::unique_ptr<tree_lister> lister{type->create_tree_lister({dir})};
    size_t nfiles = 0;

    lister->list_entries([&] (const lister::entry &ent, const struct stat *, tree_lister::visit_info info) {
        state->test_cancel();

        if (ent.type != DT_DIR) {
            nfiles++;
        }

        return true;
    });

    return nfiles;
}

void call_callback(std::shared_ptr<cancel_state> state, dir_size_callback callback, size_t nfiles) {
    dispatch_main([=] {
        try {
            state->no_cancel(std::bind(callback, nfiles));
        }
        catch (const cancel_state::cancelled &) {
            // Operation Cancelled
        }
    });
}
