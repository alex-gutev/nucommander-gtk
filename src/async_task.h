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

namespace nuc {
    typedef std::function<void()> async_task;
    
    // Must be called from the main thread
    void init_threads();
    
    //extern Glib::ThreadPool global_thread_pool;
    
    // Must be called after global_thread_pool
    Glib::ThreadPool &global_thread_pool();
    
    // Must be called on the main thread
    Glib::Dispatcher &global_dispatcher();
    
    async_queue<async_task> &global_main_queue();
    
    template <typename F>
    void dispatch_async(F fn) {
        global_thread_pool().push(fn);
    }
    
    template <typename F>
    void dispatch_main(F fn) {
        global_main_queue().emplace(std::forward<F>(fn));
        global_dispatcher().emit();
    }
}

#endif // ASYNC_TASK_H
