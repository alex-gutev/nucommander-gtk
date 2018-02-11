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

#include "vfs.h"

#include "dir_lister.h"

#include <memory>


using namespace nuc;

void vfs::read(const path_str& path) {
    if (!op) {
        op = make_operation([=] (cancel_state &state) {
            op_main(state, path);
        }, [=] (cancel_state &state, bool cancelled) {
            op_finish(cancelled);
        });
        
        op_status = 0;
        op->run();
    }
}

void vfs::cancel() {
    if (op) op->cancel();
}

void vfs::free_op() {
    op->release();
    op = nullptr;
}

void vfs::commit_read() {
    cur_tree.swap(new_tree);
    new_tree.clear();
}


void vfs::op_main(cancel_state &state, const std::string &path) {
    std::unique_ptr<lister> listr(new dir_lister());

    call_begin(state, false);
    
    try {
        listr->open(path);
        
        lister::entry ent;
        struct stat st;
        
        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(state, ent, st);
            }
        }
    }
    catch (lister::error &e) {
        state.no_cancel([=] {
            op_status = e.code();
        });
    }
}

void vfs::op_finish(bool cancelled) {
    if (cancelled || op_status) {
        new_tree.clear();
    }

    call_finish(cancelled, op_status);
}

void vfs::add_entry(cancel_state &state, const lister::entry &ent, const struct stat &st) {
    state.no_cancel([=] {
        dir_entry &new_ent = new_tree.add_entry(ent, st);
        call_new_entry(new_ent);
    });
}



//// Callbacks

void vfs::call_begin(cancel_state &op, bool refresh) {
    op.no_cancel([=] {
        cb_begin(refresh);
    });
}

void vfs::call_new_entry(dir_entry &ent) {
    cb_new_entry(ent);
}

void vfs::call_finish(bool cancelled, int error) {
    cb_finish(cancelled, error);
}
