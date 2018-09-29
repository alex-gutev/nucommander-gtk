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

#include "lister/tree_lister.h"
#include "stream/dir_writer.h"


using namespace nuc;

/**
 * Copies the data from the input stream @a in, to the output stream
 * @a out. Closes both streams.
 *
 * @param state Cancellation state.
 * @param in    Input stream to read data from.
 * @param out   Output stream to write data to.
 */
static void copy_file(cancel_state &state, instream *in, outstream *out);

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

        copy(state, lister.get(), writer.get());
    };
}

void nuc::copy(cancel_state &state, nuc::tree_lister *in, nuc::dir_writer *out) {
    in->list_entries([&] (const lister::entry &ent, const struct stat *st, tree_lister::visit_info info) {
        state.test_cancel();

        switch (ent.type) {
        case DT_DIR:
            if (info == nuc::tree_lister::visit_preorder)
                out->mkdir(ent.name);
            else if (info == nuc::tree_lister::visit_postorder)
                out->set_attributes(ent.name, st);
            break;

        case DT_REG:
            copy_file(state, in->open_entry(), out->create(ent.name, st));
            break;
        }

    });

    out->close();
}

static void copy_file(cancel_state &state, instream *src, outstream *dest) {
    std::unique_ptr<instream> in(src);
    std::unique_ptr<outstream> out(dest);

    size_t size;
    off_t offset;

    while (const instream::byte *block = in->read_block(size, offset)) {
        state.test_cancel();

        out->write(block, size, offset);
    }
}
