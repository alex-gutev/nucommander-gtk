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

#include <functional>

#include "task_queue.h"

#include "dir_tree.h"

#include "lister.h"
#include "dir_monitor.h"

namespace nuc {
    /**
     * Virtual file system abstraction.
     *
     * Reads directories, choosing the correct lister object for the
     * directory, and presents a uniform file system abstraction, in
     * which a single directory, in the directory tree, can be viewed
     * at a time.
     *
     * TODO: Implement path case canonicalization and support for
     * archives.
     */
    class vfs {
        /**
         * The current path.
         */
        std::string cur_path;

        
        /* Directory Tree */
        
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

        
        /* Background Tasks */

        /**
         * Background task queue.
         */
        task_queue queue;

        /**
         * Status of the last operation: 0 - the operation was
         * successful, non-zero the operation failed with the non-zero
         * value being the error code.
         */
        int op_status = 0;
        

        /* Reading Tasks */

        /**
         * Read directory task. Reads the directory at 'path'.
         */
        void read_dir(cancel_state &state, const std::string &path);
        /**
         * Task finish callback. Calls the finish callback.
         */
        void finish_read(bool cancelled);

        
        /**
         * Adds an entry to the 'new_tree' directory tree. The adding
         * of the entry is performed with the cancellation state set
         * to "no cancel".
         */
        void add_entry(cancel_state &state, const lister::entry &ent, const struct stat &st);

        
    public:
        /**
         * Begin callback function type. 
         *
         * Called, on the background thread, before beginning a
         * background operation. Takes the following arguments:
         *
         * bool: True if this a refresh operation, false if this is a
         *       new read operation.
         */
        typedef std::function<void(bool)> begin_fn;
        /**
         * New entry callback function type.
         *
         * Called, on the background thread, when a new entry is
         * read. Takes the following arguments:
         *
         * dir_entry &: Reference to the entry.
         */
        typedef std::function<void(dir_entry &)> new_entry_fn;
        /**
         * Finish callback function type.
         *
         * Called, on the background thread, after the background
         * operation has completed or is cancelled. After this
         * callback is called there will be no further invocations of
         * any of the callbacks regarding the last completed operation.
         *
         * Takes the following arguments:
         *
         * bool: True if the operation was cancelled, false if it ran
         *       to completion.
         *
         * int:  The error code, 0 if no error occurred.
         */
        typedef std::function<void(bool, int)> finish_fn;

        
        /**
         * Sets the begin callback function.
         */
        template <typename F>
        void callback_begin(F&& fn);
        /**
         * Sets the new entry callback function.
         */
        template <typename F>
        void callback_new_entry(F&& fn);
        /**
         * Sets the finish callback function.
         */
        template <typename F>
        void callback_finish(F&& fn);

        
        /**
         * Initiates a background read operation for the directory at
         * 'path'.
         */
        void read(const path_str &path);

        /**
         * Cancels the background operation if any. The operation is
         * considered cancelled when the finish callback is called.
         *
         * This function should only in between the begin callback and
         * finish callback being called.
         */
        void cancel();

        
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
         * New entry callback function.
         */
        new_entry_fn cb_new_entry;
        /**
         * Finish callback function.
         */
        finish_fn cb_finish;
        /**
         * Begin callback function.
         */
        begin_fn cb_begin;

        
        /**
         * Calls the begin callback. The cancellation is switched
         * switched to the "no cancel" state, before calling the
         * callback, and switched to the "can cancel" state, after
         * calling the callback.
         */
        void call_begin(cancel_state &state, bool refresh);
        /**
         * Calls the new entry callback.
         */
        void call_new_entry(dir_entry &ent);
        /**
         * Calls the finish callback.
         */
        void call_finish(bool cancelled, int error);
    };
}


/** Template Implementation */

template <typename F>
void nuc::vfs::callback_begin(F&& fn) {
    cb_begin = std::forward<F>(fn);
}

template <typename F>
void nuc::vfs::callback_new_entry(F&& fn) {
    cb_new_entry = std::forward<F>(fn);
}

template <typename F>
void nuc::vfs::callback_finish(F&& fn) {
    cb_finish = std::forward<F>(fn);
}

template <typename F>
void nuc::vfs::for_each(F f) {
    for (auto &ent_pair : cur_tree) {
        f(ent_pair.second);
    }
}


#endif // NUC_VFS_H

// Local Variables:
// mode: c++
// End:
