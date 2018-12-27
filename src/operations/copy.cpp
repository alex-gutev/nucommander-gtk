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

#include "copy.h"

#include <memory>

#include "errors/restarts.h"

#include "lister/tree_lister.h"
#include "stream/dir_writer.h"

#include "stream/file_outstream.h"

using namespace nuc;

/**
 * Copy task function.
 *
 * @param state Cancellation state.
 *
 * @param src_type Type of the directory containing the files to be
 *   copied.
 *
 * @param paths The subpaths of the entries which should be copied.
 *
 * @param dest Destination path entered by the user.
 */
static void copy_task_fn(cancel_state &state, const dir_type &src_type, const std::vector<paths::pathname> &paths, const paths::pathname &dest);

/**
 * Replaces the initial components of @a subpath with other
 * components. Used when copying a directory to a directory with a
 * different name.
 *
 * @param comp_len Length of the initial components to replace.
 *
 * @param replacement Replacement string.
 *
 * @param path The subpath.
 *
 * @return A path in which the initial @a comp_len characters, of @a
 *   subpath, are replaced with @a replacement.
 */
static paths::string replace_initial_dir(size_t comp_len, const paths::string &replacement, const paths::string &path);

/**
 * Copies the data from the input stream @a in, to the output stream
 * @a out. Closes both streams.
 *
 * @param state Cancellation state.
 * @param in    Input stream to read data from.
 * @param out   Output stream to write data to.
 */
static void copy_file(cancel_state &state, instream &in, outstream &out);

/**
 * Copies the files read from the tree lister @a lst, to temporary
 * files.
 *
 * @param state Cancellation state.
 * @param lst Source directory tree lister.
 *
 * @param callback Callback which is called (with the paths to the
 *    temporary files) after each file is copied successfully.
 */
static void copy_to_temp(cancel_state &state, tree_lister &lst, const std::function<void(const char *)> &callback);


//// Implementation

/// Creating the copy task function

nuc::task_queue::task_type nuc::make_copy_task(dir_type src_type, const std::vector<dir_entry *> &entries, const paths::string &dest) {
    using namespace std::placeholders;

    return std::bind(copy_task_fn, _1, src_type, lister_paths(entries), dest);
}

std::vector<paths::pathname> nuc::lister_paths(const std::vector<dir_entry*> &entries) {
    std::vector<paths::pathname> paths;

    for (dir_entry *ent : entries) {
        paths.push_back(paths::pathname(ent->subpath(), ent->type() == dir_entry::type_dir));
    }

    return paths;
}


void copy_task_fn(cancel_state &state, const dir_type &src_type, const std::vector<paths::pathname> &paths, const paths::pathname &dest) {
    using namespace std::placeholders;

    try {
        paths::pathname dest_dir;
        map_name_fn map_name;

        std::tie(dest_dir, map_name) = determine_dest_dir(dest, paths);

        std::unique_ptr<tree_lister> lister(src_type.create_tree_lister(paths));
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest_dir));

        copy(state, *lister, *writer, map_name);
    }
    catch (const error &e) {
        // Catch error to abort operation.
    }
}

std::pair<paths::string, map_name_fn> nuc::determine_dest_dir(const paths::pathname &dest, const std::vector<paths::pathname> &paths) {
    using namespace std::placeholders;

    if (paths.size() == 1 && !dest.is_dir()) {
        return std::make_pair(
            dest.remove_last_component(),
            std::bind(replace_initial_dir, paths[0].basename().length(), dest.basename(), _1)
        );
    }

    return std::make_pair(dest, identity());
}

paths::string replace_initial_dir(size_t len, const paths::string &replacement, const paths::string &path) {
    return replacement + path.substr(len);
}


/// Actual copying of files

void nuc::copy(cancel_state &state, nuc::tree_lister &in, nuc::dir_writer &out, const map_name_fn &map_name) {
    in.list_entries([&] (const lister::entry &ent, const struct stat *st, tree_lister::visit_info info) {
        // Flag for whether the directory's contents should be copied
        // if the directory itself could not be copied.
        bool copy_dir = false;

        global_restart skip(skip_exception::restart);
        global_restart write_into(restart("write into", [&] (const error &, boost::any) {
            copy_dir = true;
            throw skip_exception();
        }, [=] (const error &e) {
            return e.error_type() == error::type_create_dir && e.code() == EEXIST;
        }));

        state.test_cancel();

        const paths::string &ent_name = map_name(ent.name);

        try {
            switch (ent.type) {
            case DT_DIR:
                if (info == nuc::tree_lister::visit_preorder)
                    out.mkdir(ent_name);
                else if (info == nuc::tree_lister::visit_postorder)
                    out.set_attributes(ent_name, st);
                break;

            case DT_REG:{
                std::unique_ptr<instream> src(in.open_entry());
                std::unique_ptr<outstream> dest(out.create(ent_name, st));

                copy_file(state, *src, *dest);
            } break;

            case DT_LNK:
                out.symlink(ent_name, in.symlink_path(), st);
                break;
            }
        }
        catch (const skip_exception &) {
            // Do nothing in order to skip the current file.
            return copy_dir;
        }

        return true;
    });

    out.close();
}

static void copy_file(cancel_state &state, instream &in, outstream &out) {
    size_t size;
    off_t offset;

    while (const instream::byte *block = in.read_block(size, offset)) {
        state.test_cancel();

        out.write(block, size, offset);
    }
}


/// Unpacking files from archives

nuc::task_queue::task_type nuc::make_unpack_task(const dir_type &src_type, const paths::pathname &subpath, const std::function<void(const char *)> &callback) {
    return [=] (cancel_state &state) {
        std::unique_ptr<tree_lister> ls(src_type.create_tree_lister({subpath}));

        copy_to_temp(state, *ls, callback);
    };
}

void copy_to_temp(cancel_state &state, tree_lister &lst, const std::function<void(const char *)> &callback) {
    lst.list_entries([&] (const lister::entry &ent, const struct stat *st, tree_lister::visit_info info) {
        // TODO: Use mkstemps to create temporary with same extension
        char name[] = "/tmp/nucommander-tmp-XXXXXX";
        int fd = mkstemp(name);

        if (fd >= 0) {
            std::unique_ptr<instream> in(lst.open_entry());
            std::unique_ptr<outstream> out(new file_outstream(fd));

            copy_file(state, *in, *out);

            state.no_cancel([&] {
                callback(name);
            });
        }

        return true;
    });
}
