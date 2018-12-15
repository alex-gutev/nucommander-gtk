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

#include "move.h"

#include <memory>

#include "copy.h"

#include "errors/restarts.h"

#include "stream/dir_writer.h"

using namespace nuc;

/**
 * Exception which is thrown when the copy restart is invoked.
 *
 * This is used to begin a copy task after a rename task fails due to
 * the destination being located on another device.
 */
struct begin_copy_exception {};

/**
 * Attempts to move the files in @a paths to the destination directory
 * @a dest. A copy restart is established which, if invoked, copies
 * the files to the destination directory.
 *
 * @param state Cancellation state.
 *
 * @param src_type Directory type of the source directory containing
 *   the items to be renamed.
 *
 * @param paths Vector containing the subpaths, within the source
 *   directory, to be renamed.
 *
 * @param dest Path to the destination directory.
 */
static void move_or_copy(cancel_state &state, const dir_type &src_type, const std::vector<paths::string> &paths, const paths::string &dest);

/**
 * Copies the files in @a paths to the destination directory @a dest.
 *
 * @param state Cancellation state.
 *
 * @param src_type Directory type of the source directory containing
 *   the items to be copied.
 *
 * @param paths Vector containing the subpaths, within the source
 *   directory, to be copied.
 *
 * @param dest Path to the destination directory.
 */
static void copy_files(cancel_state &state, const dir_type &src_type, const std::vector<paths::string> &paths, const paths::string &dest);

task_queue::task_type nuc::make_move_task(dir_type src_type, const std::vector<dir_entry*> &entries, const paths::string &dest) {
    std::vector<paths::string> paths;

    for (dir_entry *ent : entries) {
        paths.push_back(ent->subpath());

        if (ent->type() == dir_entry::type_dir)
            paths.back() += '/';
    }

    return [=] (cancel_state &state) {
        try {
            move_or_copy(state, src_type, paths, dest);
        }
        catch (const error &e) {
            // Catch error to abort operation.
        }
    };
}

static void move_or_copy(cancel_state &state, const dir_type &src_type, const std::vector<paths::string> &paths, const paths::string &dest) {
    // Check that the source and destination directories are on the
    // same file system.
    if (auto fs_type = dir_type::on_same_fs(src_type.path(), dest)) {
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(src_type.path()));

        global_restart copy(restart("copy", [] (const error &e, boost::any) {
            throw begin_copy_exception();
        },
        [] (const error &e) {
            return e.code() == EXDEV;
        }));

        try {
            move(state, paths, fs_type == dir_type::fs_type_dir ? dest : dir_type::get_subpath(dest), *writer);
            writer->close();
        }
        catch (const begin_copy_exception &) {
            copy_files(state, src_type, paths, dest);
        }
    }
    else {
        copy_files(state, src_type, paths, dest);
    }
}

void nuc::move(cancel_state &state, const std::vector<paths::string> &items, const paths::string &dest, dir_writer &dir) {
    for (const paths::string &item : items) {
        global_restart skip(skip_exception::restart);

        try {
            dir.rename(item, paths::appended_component(dest, paths::file_name(item)));
        }
        catch (const skip_exception &) {
            // Do nothing to skip the current file
        }
    }
}

static void copy_files(cancel_state &state, const dir_type &src_type, const std::vector<paths::string> &paths, const paths::string &dest) {
    std::unique_ptr<tree_lister> lister{src_type.create_tree_lister(paths)};
    std::unique_ptr<dir_writer> dest_writer{dir_type::get_writer(dest)};

    std::unique_ptr<dir_writer> src_writer{dir_type::get_writer(src_type.logical_path())};

    lister->add_list_callback([&] (const lister::entry &ent, const struct stat *st, tree_lister::visit_info info) {
        global_restart skip(skip_exception::restart);

        try {
            if (ent.type != DT_DIR || info == tree_lister::visit_postorder) {
                src_writer->remove(ent.name);
            }
        }
        catch (const skip_exception &) {
            // Do nothing to skip file.
        }

        return true;
    });

    nuc::copy(state, *lister, *dest_writer);

    src_writer->close();
}
