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
        op = make_operation([=] (operation &op) {
            op_main(op, path);
        }, [=] (operation &op, bool cancelled) {
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


void vfs::op_main(operation &op, const std::string &path) {
    std::unique_ptr<lister> listr(new dir_lister());
    
    call_callback(op, BEGIN);
    
    try {
        listr->open(path);
        
        lister::entry ent;
        struct stat st;
        
        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(op, ent, st);
            }
        }
    }
    catch (lister::error &e) {
        op.no_cancel([=] {
            op_status = e.code();
            callback(*this, ERROR);
        });
    }
}

void vfs::op_finish(bool cancelled) {
    if (cancelled || op_status) {
        new_tree.clear();
    }

    callback(*this, cancelled ? CANCELLED : FINISH);
}

void vfs::add_entry(operation &op, const lister::entry &ent, const struct stat &st) {
    op.no_cancel([=] {
        new_tree.add_entry(ent, st);
    });
}

void vfs::call_callback(operation &op, vfs::op_stage stage) {
    op.no_cancel([=] {
        callback(*this, stage);
    });
}




