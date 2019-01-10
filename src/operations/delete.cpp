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

#include "delete.h"

#include <memory>

#include "errors/restarts.h"

#include "copy.h"

/**
 * Delete task function.
 *
 * @param state Cancellation state.
 *
 * @param lister Tree lister listing the entries to delete.
 *
 * @param writer Directory writer of the directory containing the
 *   entries.
 */
static void delete_task(nuc::cancel_state &state, nuc::tree_lister &lister, nuc::dir_writer &writer);

nuc::task_queue::task_type nuc::make_delete_task(dir_type src_type, const std::vector<dir_entry*> &entries) {
    return [=] (cancel_state &state) {
        state.call_progress(progress_event(progress_event::type_begin));

        try {
            std::unique_ptr<tree_lister> lister(src_type.create_tree_lister(lister_paths(entries)));
            std::unique_ptr<dir_writer> writer(dir_type::get_writer(src_type.logical_path()));

            delete_task(state, *lister, *writer);
        }
        catch (const error &) {
            // Catch to abort operation
        }

        state.call_progress(progress_event(progress_event::type_finish));
    };
}

void delete_task(nuc::cancel_state &state, nuc::tree_lister &lister, nuc::dir_writer &writer) {
    using namespace nuc;

    lister.list_entries([&] (const lister::entry &ent, const struct stat *st, tree_lister::visit_info info) {
        global_restart skip(skip_exception::restart);

        state.call_progress(progress_event(progress_event::type_enter_file, ent.name, 1));

        try {
            if (ent.type != DT_DIR || info == tree_lister::visit_postorder) {
                writer.remove(ent.name);
            }
        }
        catch (const skip_exception &) {
            if (ent.type != DT_DIR) {
                state.call_progress(progress_event(progress_event::type_exit_file, ent.name));
            }
        }

        state.call_progress(progress_event(progress_event::type_exit_file, ent.name, 1));

        return true;
    });

    writer.close();
}
