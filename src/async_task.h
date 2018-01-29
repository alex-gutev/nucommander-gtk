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

#ifndef ASYNC_TASK_H
#define ASYNC_TASK_H

#include <functional>

#include <glibmm/threadpool.h>
#include <glibmm/dispatcher.h>

#include "async_queue.h"

/**
 * Contains functions for running tasks in background threads and
 * dispatching tasks to be run on the main thread.
 */

namespace nuc {
    /**
     * Task function type.
     */
    typedef std::function<void()> async_task;
    
    /**
     * Initializes the global thread pool and global main thread
     * dispatcher object.
     *
     * Must be called from the main thread, which is responsible for
     * managing the GUI, after the GTK event loop (Gtk::MainContext)
     * has been initialized.
     */
    void init_threads();

    /**
     * Returns the global thread pool to which background tasks are
     * dispatched.
     */
    Glib::ThreadPool &global_thread_pool();

    /**
     * Returns the global main thread dispatcher object. This a
     * Glib::Dispatcher object which allows a function to be queued on
     * the main event loop, in order to be run on the main thread.
     */
    Glib::Dispatcher &global_dispatcher();

    /**
     * Returns the queue of tasks, to run on the main thread.
     */
    async_queue<async_task> &global_main_queue();

    
    /**
     * Runs a function on a background thread.
     *
     * fn: The function (any callable object) to be run on a
     * background thread.
     */
    template <typename F>
    void dispatch_async(F fn) {
        global_thread_pool().push(fn);
    }

    /**
     * Queues a function to be run on the main thread.
     *
     * fn: The function (any callable object) to be run on the main
     * thread.
     */
    template <typename F>
    void dispatch_main(F fn) {
        global_main_queue().emplace(std::forward<F>(fn));
        global_dispatcher().emit();
    }
}

#endif // ASYNC_TASK_H
