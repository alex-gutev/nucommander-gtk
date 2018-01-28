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

#include "async_task.h"

static nuc::async_queue<nuc::async_task> main_queue;

static Glib::ThreadPool * thread_pool;
static Glib::Dispatcher * dispatcher;

static void main_thread_dispatcher();


void nuc::init_threads() {
    dispatcher = new Glib::Dispatcher();
    
    dispatcher->connect(sigc::ptr_fun(main_thread_dispatcher));
    
    thread_pool = new Glib::ThreadPool();
}


Glib::ThreadPool &nuc::global_thread_pool() {
    return *thread_pool;
}

Glib::Dispatcher &nuc::global_dispatcher() {
    return *dispatcher;
}

nuc::async_queue<nuc::async_task> &nuc::global_main_queue() {
    return main_queue;
}


void main_thread_dispatcher() {
    nuc::async_task task;
    
    if (main_queue.pop(task)) {
        task();
    }
}