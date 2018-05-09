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
#include <atomic>
#include <memory>

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
    class vfs : public std::enable_shared_from_this<vfs> {
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
        std::shared_ptr<task_queue> queue;

        /**
         * Status of the last operation: 0 - the operation was
         * successful, non-zero the operation failed with the non-zero
         * value being the error code.
         */
        int op_status = 0;

        /**
         * Flag: true if a read operation is ongoing.
         */
        std::atomic<bool> reading{false};
        /**
         * Flag: true if an update/refresh operation is ongoing.
         */
        std::atomic<bool> updating{false};

        /**
         * Flag: Once set no more background tasks will be initiated.
         *
         * This is set when the object begins cleaning up.
         */
        bool finalized = false;

        
        /**
         * Adds a task to the background task queue.
         * 
         * Task function prototype:
         *
         *     void(vfs *this, cancel_state &state)
         *
         * An std::shared_ptr, of the vfs object, is created (using
         * shared_from_this()). The task is wrapped in a lambda
         * function which captures the shared_ptr (by copy), and calls
         * the task function passing in the raw 'this' pointer
         * (obtained using shared_ptr::get()) as the first argument.
         *
         * The task holds a strong reference to the vfs object thus,
         * 'this' will not be deallocated before the task has finished
         * executing (unless the task is cancelled before it is run).
         *
         * To use a method as the task, simply pass a pointer to the
         * method, using std::bind to bind additional arguments. Do
         * not use a lambda which captures the 'this' pointer directly
         * as that does not ensure that the object will be in memory
         * when the task is run.
         */
        template <typename F>
        void queue_task(F task);

        /**
         * Same as the queue_task, however allows a finish callback to
         * be specified.
         *
         * Finish callback prototype:
         *
         *    void(vfs *this, cancel_state &state)
         *
         * As in queue_task(task), the task and finish callbacks are
         * wrapped in lambda functions which hold a strong reference
         * to the vfs object.
         */
        template <typename T, typename F>
        void queue_task(T task, F finish);

        /**
         * Queue the function fn to be run on the main thread.
         *
         * The function fn is wrapped in a lambda which captures a
         * weak pointer to the object. If the object is still in
         * memory when the lambda is run, the function is called with
         * the raw pointer passed as the first argument.
         */
        template <typename F>
        void queue_main(F fn);
        
        /**
         * Cancels the directory monitor and all background tasks, and
         * sets the finalized flag.
         */
        void finalize();

        
        /* Reading Tasks */

        /**
         * Adds a read task to the task queue.
         *
         * @param path The path of the directory to read
         *
         * @param refresh True if the current directory is being
         *                reread, false if a new directory is being
         *                read.
         */
        void add_read_task(const std::string &path, bool refresh);
        
        /**
         * Read directory task. Reads the directory at 'path'.
         */
        void read_dir(cancel_state &state, const std::string &path, bool refresh);
        /**
         * Read task finish callback. Calls the finish callback.
         */
        void finish_read(bool cancelled, const path_str &path, bool refresh);

        
        /**
         * Adds an entry to the 'new_tree' directory tree. The adding
         * of the entry is performed with the cancellation state set
         * to "no cancel".
         */
        void add_entry(cancel_state &state, const lister::entry &ent, const struct stat &st);

        /**
         * Cancels all update tasks on the task queue.
         */
        void cancel_update();


        /* Directory Monitoring */

        dir_monitor monitor;

        /**
         * Begins monitoring the directory at path.
         */
        void monitor_dir(const path_str &path, bool paused = false);

        /**
         * Resumes the directory monitor.
         *
         * May only be called from the main thread.
         */
        void resume_monitor();
        
        /**
         * Signal handler for the file event signal.
         */
        void file_event(dir_monitor::event e);

        /**
         * Event handler for the EVENTS_BEGIN event. This event is
         * sent before the first event in a series of events.
         */
        void begin_changes(cancel_state &state);

        /**
         * Event handler for the EVENTS_END event. This event is sent
         * after a time interval has elapsed after the last event.
         */
        void end_changes(cancel_state &state);

        /**
         * Handler functions for the create, change, delete and rename
         * file events.
         */
        void file_created(cancel_state &state, const path_str &path);
        void file_changed(cancel_state &state, const path_str &path);
        void file_deleted(cancel_state &state, const path_str &path);
        void file_renamed(cancel_state &state, const path_str &src, const path_str &dest);

        /**
         * Obtains the stat attributes of a file. First the stat
         * system call is attempted, it that fails the lstat system
         * call is attempted.
         *
         * @param path The file path.
         *
         * @param st   Pointer to the stat struct where the stat
         *             attributes will be read into
         *
         * @return true if successful, false if both stat and lstat
         *         failed.
         */
        static bool file_stat(const path_str &path, struct stat *st);
        
        
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
        typedef std::function<void(bool, int, bool)> finish_fn;

        
        /* Constructor */
        vfs();

        /**
         * Creates a new vfs object and returns an std::shared_ptr
         * pointer to it.
         */
        static std::shared_ptr<vfs> create();
        
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
         *
         * Should only be called on the main thread, and should not be
         * called again until the finish callback is called.
         */
        void read(const path_str &path);

        /**
         * Cancels the background operation if any. The operation is
         * considered cancelled when the finish callback is called.
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
         * the list read in the previous operation.
         *
         * Should only be called on the main thread.
         */
        void commit_read();

        /**
         * Calls the function function f on each entry (passed as the
         * first parameter).
         */
        template <typename F>
        void for_each(F f);

        /**
         * Asynchronous cleanup method.
         *
         * Calls the cleanup method once all background tasks have
         * been cancelled.
         *
         * @param fn The cleanup method.
         *
         * This method should only be called on the main thread. The
         * function fn will be called on the main thread.
         */
        template <typename F>
        void cleanup(F fn);
        
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
         *
         * If error is zero (task was successful), the task queue is
         * paused in order to prevent other tasks running until
         * commit_read() is called.
         */
        void call_finish(bool cancelled, int error, bool refresh);
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


template <typename F>
void nuc::vfs::cleanup(F fn) {
    finalize();
    
    queue->add([fn] (cancel_state &state) {
        dispatch_main(fn);
    });
}

#endif // NUC_VFS_H

// Local Variables:
// mode: c++
// End:
