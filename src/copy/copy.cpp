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
    std::vector<paths::string> paths;

    for (dir_entry *ent : entries) {
        paths.push_back(ent->subpath());

        if (ent->type() == dir_entry::type_dir)
            paths.back() += '/';
    }

    return [=] (cancel_state &state) {
        std::unique_ptr<tree_lister> lister(src_type.create_tree_lister(paths));
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest));

        try {
            copy(state, *lister, *writer);
        }
        catch (const error &e) {
            // Catch error to abort operation.
        }
    };
}

void nuc::copy(cancel_state &state, nuc::tree_lister &in, nuc::dir_writer &out) {
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

        try {
            switch (ent.type) {
            case DT_DIR:
                if (info == nuc::tree_lister::visit_preorder)
                    out.mkdir(ent.name);
                else if (info == nuc::tree_lister::visit_postorder)
                    out.set_attributes(ent.name, st);
                break;

            case DT_REG:{
                std::unique_ptr<instream> src(in.open_entry());
                std::unique_ptr<outstream> dest(out.create(ent.name, st));

                copy_file(state, *src, *dest);
            } break;

            case DT_LNK:
                out.symlink(ent.name, in.symlink_path(), st);
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
