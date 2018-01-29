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
     * Virtual file system abstraction.
     *
     * Allows directories to be read, regardless of whether they are
     * regular directories, archives, remote directories (provided a
     * suitable lister subclass is implemented), abstracting away the
     * choosing of the lister object.
     *
     * TODO: Implement path case canonicalization and support for
     * archives.
     */
    class vfs {
        /**
         * The current path.
         */
        std::string cur_path;

        /**
         * The directory tree of the entries read in the previous
         * operation.
         */
        dir_tree cur_tree;
        /**
         * The directory tree into which the entries read in the
         * currently running operation are placed.
         */
        dir_tree new_tree;

        /**
         * Pointer to the current running operation, nullptr if none.
         */
        operation *op = nullptr;
        
        /**
         * Status of the last operation: 0 - the operation was
         * successful, non-zero the operation failed with the non-zero
         * value being the error code.
         */
        int op_status = 0;
        
        /**
         * Operation main method. Reads the directory at 'path'.
         */
        void op_main(operation &op, const std::string &path);
        /**
         * Operation finish method. Calls the callback with stage =
         * FINISH/CANCELLED.
         */
        void op_finish(bool cancelled);

        /**
         * Adds an entry to the 'new_tree' directory tree. The adding
         * of the entry is performed with 'op' in the "no cancel"
         * state, using the 'operation::no_cancel' method.
         */
        void add_entry(operation &op, const lister::entry &ent, const struct stat &st);
        
    public:
        /**
         * List operation stage constants.
         */
        enum op_stage {
            /** Operation began. */
            BEGIN = 0,
            /** Operation completed. */
            FINISH,
            /** An error occured. */
            ERROR,
            /** Operation cancelled. */
            CANCELLED
        };

        /**
         * Callback function type. Receives two parameters:
         *
         * vfs:   Reference to the vfs object.
         * stage: op_stage constant identifying the operation stage.
         * 
         * When called with stage == FINISH or CANCELLED, there will
         * be no further invocations of the callback.
         */
        typedef std::function<void(vfs &, op_stage)> callback_fn;

        /** Callback function object. */
        callback_fn callback;

        /**
         * Initiates a background read operation for the directory at
         * 'path'.
         */
        void read(const path_str &path);

        /**
         * Cancels the background operation if any. The operation is
         * cancelled when the callback is called with stage ==
         * CANCELLED/FINISH.
         */
        void cancel();

        /**
         * Frees the last operation's state. cancel should not be
         * called after this method.
         */
        void free_op();

        /**
         * Returns the status (error code) of the last operation.
         */
        int status() const {
            return op_status;
        }

        /**
         * Discards the previous list of entries and replaces it with
         * the list read in the operation which completed last.
         */
        void commit_read();

        /**
         * Calls the function function f on each entry (passed by
         * reference as the first parameter).
         */
        template <typename F>
        void for_each(F f);
        
    private:

        /**
         * Calls the callback a stage, changes the operation state to
         * "no cancel" before calling the callback and back to
         * "can_cancel" after the callback returns.
         */
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
