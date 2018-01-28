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

#ifndef NUC_VFS_H
#define NUC_VFS_H

#include "fn_operation.h"
#include "dir_tree.h"

#include "lister.h"

#include <functional>

namespace nuc {
    /**
     * Provides a virtual file system abstraction.
     *
     * Allows reading directories regardless of whether they
     * are on disk directories, archives or remote directories.
     */
    class vfs {
        std::string cur_path;
        
        dir_tree cur_tree;
        dir_tree new_tree;
        
        operation *op = nullptr;
        int op_status = 0;
        
        void op_main(operation &op, const std::string &path);
        void op_finish(bool cancelled);
        
        void add_entry(operation &op, const lister::entry &ent, const struct stat &st);
        
    public:
        enum op_stage {
            BEGIN = 0,
            FINISH,
            ERROR,
            CANCELLED
        };
        
        typedef std::function<void(vfs &, op_stage)> callback_fn;
        
        callback_fn callback;
        
        void read(const path_str &path);
        void cancel();
        
        int status() const {
            return op_status;
        }
        
        void commit_read();
        
        template <typename F>
        void for_each(F f);
        
    private:
        
        void call_callback(operation &op, op_stage stage);
    };
}

/** Template Implementation */

template <typename F>
void nuc::vfs::for_each(F f) {
    for (auto &ent_pair : cur_tree) {
        f(ent_pair.second);
    }
}


#endif // NUC_VFS_H
