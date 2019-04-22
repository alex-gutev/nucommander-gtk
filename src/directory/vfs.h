/*
 * NuCommander
 * Copyright (C) 2018-2019  Alexander Gutev <alex.gutev@gmail.com>
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

#include "paths/pathname.h"

#include "tasks/task_queue.h"

#include "dir_type.h"
#include "dir_monitor.h"

namespace nuc {
    /**
     * Virtual file system abstraction.
     *
     * Provides functionality for reading directories, with the
     * correct lister object chosen automatically, and manages the
     * reading on a background thread. Additionally the contents of
     * the directory are viewed as a directory tree rather than a flat
     * list of files.
     */
    class vfs : public std::enable_shared_from_this<vfs> {
        /**
         * The dir_type object, responsible for creating the lister
         * and dir_tree objects.
         *
         * Should only be modified from the main thread.
         */
        std::shared_ptr<dir_type> dtype;


        /* Directory Tree */

        /**
         * The directory tree containing the entries of the current
         * directory.
         *
         * Both the pointer and pointed to object should only be
         * accessed and modified from the main thread.
         */
        std::unique_ptr<dir_tree> cur_tree = nullptr;

        /**
         * A directory tree which contains a copy of cur_tree and is
         * modified in response to file system updates.
         *
         * The pointer and pointed to object should only be accessed
         * and modified from a background thread, unless there is no
         * background thread running.
         */
        std::unique_ptr<dir_tree> new_tree = nullptr;


        /* Background Tasks */

        /**
         * Background task queue.
         */
        std::shared_ptr<task_queue> queue;

        /**
         * Flag: true if a read operation is ongoing.
         */
        std::atomic<bool> reading{false};
        /**
         * Flag: true if updates to the directory tree have been
         * received which have not been applied yet.
         */
        std::atomic<bool> updating{false};

        /**
         * Flag: Once set no more background tasks should be initiated.
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
         * Same as queue_main however the task_queue is paused prior
         * to dispatching @a fn on the main thread.
         *
         * @param      fn Function to execute on the main thread.
         */
        template <typename F>
        void queue_main_wait(F fn);

        /**
         * Cancels the directory monitor and all background tasks, and
         * sets the finalized flag.
         */
        void finalize();

        /**
         * Constructor.
         *
         * Private to prevent construction in an automatic storage
         * context.
         */
        vfs();

    public:
        /**
         * Operation Delegate Interface.
         */
        class delegate {
        public:
            virtual ~delegate() = default;

            /**
             * Called when a read operation has just begun.
             */
            virtual void begin() = 0;

            /**
             * Called when a new entry has been read.
             *
             * @param ent Reference to the entry.
             */
            virtual void new_entry(dir_entry &ent) = 0;

            /**
             * Called when the operation has finished or has been
             * cancelled.
             *
             * There will be no further invocations of any methods (of
             * the delegate object), after this method is called.
             *
             * @param cancelled True if the operation has been cancelled.
             *
             * @param error Operation error code or 0 if no error
             *   occurred.
             */
            virtual void finish(bool cancelled, int error) = 0;
        };

        /**
         * Directory changed callback function type.
         *
         * The callback function should return a delegate for the
         * refresh operation. If NULL is returned the refresh
         * operation is not initiated.
         */
        typedef std::function<std::shared_ptr<delegate>()> changed_fn;

        /**
         * Directory deleted signal type.
         */
        typedef sigc::signal<void, pathname> deleted_signal;


        /**
         * Creates a new vfs object and returns an std::shared_ptr
         * pointer to it.
         */
        static std::shared_ptr<vfs> create();

        /**
         * Sets the directory changed callback function.
         *
         * The callback function is called after the directory has
         * changed and prior to initiating a refresh operation to
         * refresh the vfs.
         *
         * The callback function should return a delegate for the
         * refresh operation. If NULL is returned the refresh
         * operation is not initiated.
         *
         * @param fn The update callback function.
         */
        template <typename F>
        void callback_changed(F&& fn);

        /**
         * Returns the deleted signal, which is emitted when the
         * directory is deleted.
         */
        deleted_signal signal_deleted();

        /**
         * Returns the logical path of the current directory in the
         * vfs.
         *
         * The logical path includes the path to the directory itself
         * and the subpath.
         *
         * @return The current path.
         */
        pathname::string path() {
            return dtype->logical_path();
        }

        /**
         * Initiates a background read operation for the directory at
         * @a path.
         *
         * Should only be called on the main thread.
         *
         * @param path The path of the directory to read.
         * @param del The delegate object for the read operation.
         */
        void read(const pathname &path, std::shared_ptr<delegate> del);

        /**
         * Attempts to list the contents of the entry @a ent.
         *
         * Should only be called on the main thread.
         *
         * @param ent The entry to list, must be an entry which was
         *   passed to the add entry callback between the last
         *   invocations of the begin and end callbacks.
         *
         * @param del The delegate object for the read operation.
         *
         * @return True if the entry is a directory which can be
         *   listed, false otherwise.
         */
        bool descend(const dir_entry &ent, std::shared_ptr<delegate> del);

        /**
         * Attempts to list the contents of the parent directory. This
         * can only be done if the VFS is currently in a virtual
         * directory hierarchy which has been read all at
         * once. Otherwise the parent directory has to be read using
         * the read() method.
         *
         * Should only be called on the main thread.
         *
         * @param del The delegate object for the read operation.
         *
         * @return True if the parent directory's contents can be
         *   listed, false otherwise.
         */
        bool ascend(std::shared_ptr<delegate> del);

        /**
         * Cancels the current background operation if any. The
         * operation is considered cancelled when the finish method,
         * of the delegate object, is called.
         *
         * @return True if there was an ongoing read operation that
         *    was cancelled.
         */
        bool cancel();

        /**
         * Returns the directory type of the VFS's current directory.
         *
         * @return     dir_type
         */
        std::shared_ptr<dir_type> directory_type() const {
            return dtype;
        }

        /**
         * Returns the first entry with name @a name in the current
         * subdirectory.
         *
         * @param name The name of the entry.
         *
         * @return Pointer to the entry, 'nullptr' if no entry with
         *    name @a name found.
         */
        dir_entry *get_entry(const pathname::string &name) {
            return cur_tree->get_entry(name);
        }

        /**
         * Returns all entries with name @a name in the current
         * subdirectory.
         *
         * @param name The name of the entry/entries to return.
         *
         * @return A pair of iterators, the first iterator points to
         *    the first entry, the second iterator is the past the
         *    end iterator.
         */
        dir_tree::entry_range get_entries(const pathname::string &path) {
            return cur_tree->get_entries(path);
        }

        /**
         * Creates a task which unpacks the file to a temporary
         * location, if it is not immediately accessible via the OS's
         * file system API.
         *
         * Once the file has been unpacked, a callback function is
         * called with the full path to the unpacked file passed as an
         * argument.
         *
         * @param ent The entry to unpack. This must be an entry that
         *   is contained in this vfs object.
         *
         * @param fn Callback function of one argument, the full path
         *   to the unpacked file.
         */
        task_queue::task_type access_file(const dir_entry &ent, std::function<void(const pathname &)> fn);

    private:
        /**
         * Directory changed callback function.
         */
        changed_fn cb_changed;

        /**
         * Deleted signal.
         */
        deleted_signal sig_deleted;


        /* Reading Tasks */

        /**
         * Read Directory Task State.
         */
        struct read_dir_state {
            /** Flag: Is this a refresh operation */
            bool refresh;

            /** Operation Delegate */
            std::shared_ptr<delegate> m_delegate;

            /** Error Code */
            std::atomic<int> error{0};

            /** dir_type of directory to read */
            std::shared_ptr<dir_type> type;
            /** dir_tree into which directory is read */
            std::unique_ptr<dir_tree> tree;

            /**
             * Constructor.
             *
             * @param refresh True if this is a refresh operation.
             * @param del The delegate object for the read operation.
             */
            read_dir_state(bool refresh, std::shared_ptr<delegate> del) : refresh(refresh), m_delegate(del) {}

            /* Disable Copying */
            read_dir_state(const read_dir_state &) = delete;
            read_dir_state &operator=(const read_dir_state &) = delete;
        };


        /**
         * Adds a read task to the task queue.
         *
         * @param path The path of the directory to read
         *
         * @param refresh True if the current directory is being
         *   reread, false if a new directory is being read.
         *
         * @param del The delegate object for the read operation.
         */
        void add_read_task(const pathname &path, bool refresh, std::shared_ptr<delegate> del);

        /**
         * Adds a read task to the task queue.
         *
         * @param type The dir_type object of the directory to be
         *   read.
         *
         * @param refresh True if the current directory is being
         *   reread, false if a new directory is being read.
         *
         * @param del The delegate object for the read operation.
         */
        void add_read_task(std::shared_ptr<dir_type> type, bool refresh, std::shared_ptr<delegate> del);

        /**
         * Adds a directory refresh task, for the current directory,
         * to the background task queue.
         */
        void add_refresh_task();

        /**
         * Reads the directory at @a path.
         *
         * @param state The cancellation state.
         * @param tstate Read directory task state.
         * @param path  The path to read.
         */
        void read_path(cancel_state &state, std::shared_ptr<read_dir_state> tstate, const pathname &path);

        /**
         * Reads the directory using the lister and dir_tree objects
         * created by the dir_type object found in the task state @a
         * tstate.
         *
         * @param state The cancellation state.
         * @param tstate Read directory task state.
         */
        void list_dir(cancel_state &state, std::shared_ptr<read_dir_state> tstate);

        /**
         * Read task finish callback function.
         *
         * @param cancelled True if the task was cancelled.
         * @param tstate Read directory task state.
         */
        void finish_read(bool cancelled, std::shared_ptr<read_dir_state> tstate);


        /**
         * Adds an entry to directory tree 'tstate->tree'. The adding
         * of the entry is performed with the cancellation state set
         * to "no cancel".
         *
         * @param state Cancellation state.
         * @param tstate Read directory task state.
         * @param ent The entry to add.
         * @param st State attributes of the entry to add.
         */
        void add_entry(cancel_state &state, read_dir_state &tstate, const lister::entry &ent, const struct stat &st);


        /**
         * Cancels all tasks on the task queue and cancels the
         * directory monitor to prevent more update tasks from being
         * queued.
         */
        void cancel_update();


        /* Finish Read Tasks */

        /**
         * Clear the reading and updating flags.
         */
        void clear_flags();

        /**
         * Starts a directory monitor for the new directory or
         * restarts/resumes the previous directory monitor.
         *
         * @param cancelled True if the task was cancelled.
         * @param error Error code of the task.
         * @param refresh True if the task was a refresh task.
         */
        void start_new_monitor(bool cancelled, int error, bool refresh);


        /* Reading subdirectories */

        /**
         * Read Subdirectory Task State.
         */
        struct read_subdir_state {
            /** Subpath to the subdirectory */
            pathname subpath;

            /** Operation Delegate */
            std::shared_ptr<delegate> m_delegate;

            /** Error code */
            std::atomic<int> error{0};

            /**
             * Constructor.
             *
             * @param path Subdirectory subpath.
             * @param del The delegate object for the read operation.
             */
            read_subdir_state(pathname path, std::shared_ptr<delegate> del) : subpath(std::move(path)), m_delegate(del) {}

            /* Disable copying */
            read_subdir_state(const read_subdir_state &) = delete;
            read_subdir_state &operator=(const read_subdir_state &) = delete;
        };

        /**
         * Adds a read task for the subdirectory @a subpath, of the
         * current directory tree, to the background task queue.
         *
         * @param subpath The subpath to read.
         * @param del The delegate object for the read operation.
         */
        void add_read_subdir(const pathname &subpath, std::shared_ptr<delegate> del);

        /**
         * Read subdirectory task.
         *
         * Reads the subdirectory of the directory tree if it
         * exists. The subdirectory is obtained and the callback
         * functions are called as though a read operation is being
         * carried out.
         *
         * @param state  The cancellation state.
         * @param tstate Read subdirectory task state.
         */
        void read_subdir(cancel_state &state, std::shared_ptr<read_subdir_state> tstate);

        /**
         * Read subdirectory task finish callback.
         *
         * Queues a task on the main thread, which sets the subpath of
         * the directory tree and calls the finish callback.
         *
         * @param cancelled True if the task was cancelled.
         * @param tstate Read subdirectory task state.
         */
        void finish_read_subdir(bool cancelled, std::shared_ptr<read_subdir_state> tstate);

        /**
         * Checks whether the current subdirectory of the directory
         * tree still exists. If not the first parent directory of the
         * subdirectory is read.
         */
        void refresh_subdir();


        /* Directory Monitoring */

        /* Directory Monitor */
        dir_monitor monitor;

        /**
         * Begins monitoring the current directory.
         */
        void monitor_dir(bool paused = false);

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
         *
         * @param Cancellation State.
         * @param del The delegate object for the refresh operation.
         */
        void end_changes(cancel_state &state, std::shared_ptr<delegate> del);

        /**
         * Queues a task on the main thread to apply all updates,
         * stored in 'new_tree'.
         *
         * @param del The delegate object for the read operation.
         */
        void finish_updates(std::shared_ptr<delegate> del);

        /**
         * Handler functions for the create, change, delete and rename
         * file events.
         */
        void file_created(cancel_state &state, const pathname::string &path);
        void file_changed(cancel_state &state, const pathname::string &path);
        void file_deleted(cancel_state &state, const pathname::string &path);
        void file_renamed(cancel_state &state, const pathname::string &src, const pathname::string &dest);

        /**
         * Removes an entry from the new directory tree (new_tree).
         *
         * @param subpath The subpath of the entry
         */
        void remove_entry(const pathname::string &subpath);

        /**
         * Obtains the stat attributes of a file. First the stat
         * system call is attempted, if that fails the lstat system
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
        static bool file_stat(const pathname::string &path, struct stat *st);


        /** Calling Delegate Methods */

        /**
         * Calls the begin method of the delegate object @a del.
         *
         * The cancellation state is switched switched to the "no
         * cancel" state, before calling the method, and switched to
         * the "can cancel" state, after calling the method.
         *
         * @param state Cancellation State.
         * @param del Operation Delegate
         */
        void call_begin(cancel_state &state, std::shared_ptr<delegate> del);
    };
}


/** Template Implementation */

template <typename F>
void nuc::vfs::callback_changed(F &&fn) {
    cb_changed = std::forward<F>(fn);
}

#endif // NUC_VFS_H

// Local Variables:
// mode: c++
// End:
