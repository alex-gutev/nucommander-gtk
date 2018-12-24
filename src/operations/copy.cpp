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
static void copy_task_fn(cancel_state &state, const dir_type &src_type, const std::vector<paths::pathname> &paths, const paths::string dest);

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


void copy_task_fn(cancel_state &state, const dir_type &src_type, const std::vector<paths::pathname> &paths, const paths::string dest) {
    using namespace std::placeholders;

    // Destination directory to copy files to. Only used if a single
    // file is being copied.
    paths::string dest_dir;
    // The new name of the file to which the original file should be
    // copied to. Only used if a single file is being copied.
    paths::string new_name;

    // Exception type thrown to begin copying within the destination
    // directory, when it is determined that the directory directory
    // exists.
    struct copy_within {};

    // Save previous error handler
    auto old_handler = global_error_handler;

    // Flag indicating whether at least one file was copied
    // succesfully.
    bool copied = false;

    // Function which returns the destination file name.
    map_name_fn map_name = identity();

    // If only a single entry is being copied.
    if (paths.size() == 1) {
        dest_dir = paths::removed_last_component(dest);
        new_name = paths::file_name(dest);

        global_error_handler = [&] (const error &e) {
            if (e.code() == EEXIST && !copied) {
                dest_dir = dest;
                new_name.clear();
                map_name = identity();

                global_error_handler = old_handler;

                throw copy_within();
            }

            return old_handler(e);
        };

        const paths::string &old_name = paths::file_name(paths[0]);
        map_name = std::bind(replace_initial_dir, old_name.length() - (old_name.back() == '/' ? 1 : 0), new_name, _1);
    }

    while(true) {
        try {
            std::unique_ptr<tree_lister> lister(src_type.create_tree_lister(paths));
            std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest_dir));

            lister->add_list_callback([&copied] (const lister::entry &, const struct stat *, tree_lister::visit_info) {
                copied = true;
                return true;
            });

            copy(state, *lister, *writer, map_name);
            break;
        }
        catch (const error &e) {
            // Catch error to abort operation.
            break;
        }
        catch (const copy_within &) {
            // Retry copy operation with new destination
        }
    }
}

paths::string replace_initial_dir(size_t len, const paths::string &replacement, const paths::string &path) {
    return replacement + path.substr(len);
}


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


nuc::task_queue::task_type nuc::make_unpack_task(const dir_type &src_type, const paths::string &subpath, const std::function<void(const char *)> &callback) {
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
