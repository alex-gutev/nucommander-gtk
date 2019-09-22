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

#ifndef NUC_DIRECTORY_VFS_H
#define NUC_DIRECTORY_VFS_H

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
             * There will be no further invocations of any methods, of
             * the delegate object, after this method is called.
             *
             * This method is called on the main thread.
             *
             * @param cancelled True if the operation was cancelled.
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


        /** Constructor */
        vfs();

        /**
         * Destructor
         *
         * Object must be destroyed on the main thread.
         */
        ~vfs();

        /* Accessors */

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


        /* Initiating Read Operations */

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


        /* Getting Entries */

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
        /* Signals */

        /**
         * Directory changed callback function.
         */
        changed_fn cb_changed;

        /**
         * Deleted signal.
         */
        deleted_signal sig_deleted;


        /* Directory Type */

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
        std::shared_ptr<dir_tree> cur_tree = nullptr;

        /**
         * A directory tree which contains a copy of cur_tree and is
         * modified in response to file system updates.
         *
         * The pointer should only be modified from the main thread.
         *
         * The pointed to object should only be accessed and modified
         * from a background thread, unless there is no background
         * the update operation has completed.
         */
        std::shared_ptr<dir_tree> new_tree = nullptr;


        /* Background Tasks */

        struct background_task_state;
        struct background_task;

        /**
         * Background Task State.
         */
        std::shared_ptr<background_task_state> tasks;


        /* Reading Tasks */

        struct read_dir_task;

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


        /* Reading subdirectories */

        struct read_subdir_task;

        /**
         * Adds a read task for the subdirectory @a subpath, of the
         * current directory tree, to the background task queue.
         *
         * @param subpath The subpath to read.
         * @param del The delegate object for the read operation.
         */
        void add_read_subdir(const pathname &subpath, std::shared_ptr<delegate> del);


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
         *
         * @param paused True if the monitor should be paused on
         *   creation.
         */
        void monitor_dir(bool paused = false);

        /**
         * Starts a directory monitor for the new directory or
         * restarts/resumes the previous directory monitor.
         *
         * @param cancelled True if the task was cancelled.
         * @param error Error code of the task.
         * @param refresh True if the task was a refresh task.
         */
        void start_new_monitor(bool cancelled, int error, bool refresh);

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


        struct update_task;

        /**
         * Event handler for the EVENTS_END event. This event is sent
         * after a time interval has elapsed after the last event.
         *
         * @param state Cancellation state.
         * @param tstate Update task state.
         */
        static void end_changes(cancel_state &state, std::shared_ptr<update_task> tstate);

        /**
         * Queues a task on the main thread to apply all updates,
         * stored in 'new_tree'.
         *
         * @param tstate Update task state.
         */
        static void finish_updates(std::shared_ptr<update_task> state);
    };
}


/** Template Implementation */

template <typename F>
void nuc::vfs::callback_changed(F &&fn) {
    cb_changed = std::forward<F>(fn);
}

#endif // NUC_DIRECTORY_VFS_H

// Local Variables:
// mode: c++
// End:
