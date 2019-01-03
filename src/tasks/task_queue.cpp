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

#include "task_queue.h"

#include "async_task.h"


using namespace nuc;

std::shared_ptr<task_queue> task_queue::create() {
    return std::make_shared<task_queue>();
}


void task_queue::add(task_type task) {
    std::lock_guard<mutex_type> lock(mutex);
    queue.emplace(task);

    begin_loop();
}

void task_queue::begin_loop() {
    if (!paused && !running.test_and_set()) {
        std::shared_ptr<cancel_state> state = get_cancel_state();
        std::shared_ptr<task_queue> ptr = shared_from_this();
        
        dispatch_async([=] {
            ptr->run_tasks(state);
        });
    }
}

void task_queue::run_tasks(std::shared_ptr<cancel_state> state) {
    task_type task;
    
    try {
        while (next_task(state, task)) {
            task(*state);
        }
    }
    catch (cancel_state::cancelled&) {}
}

bool task_queue::next_task(std::shared_ptr<cancel_state> state, task_type &task) {
    std::lock_guard<mutex_type> lock(mutex);
    
    state->test_cancel();
    
    if (!queue.empty() && !paused) {
        task = std::move(queue.front());
        queue.pop();
        return true;
    }
    
    running.clear();
    return false;
}

std::shared_ptr<cancel_state> task_queue::get_cancel_state() {
    if (!state) {
        state = std::make_shared<cancel_state>();

        std::weak_ptr<task_queue> ptr(shared_from_this());

        state->add_finish_callback([=] (bool cancelled) {
            if (auto this_ptr = ptr.lock()) {
                this_ptr->cancelled(cancelled);
            }
        });
    }
    
    return state;
}


void task_queue::cancel() {
    std::queue<task_type> old_queue;
    
    {
        std::lock_guard<mutex_type> lock(mutex);
        
        queue.swap(old_queue);
        if (state) state->cancel();
    }
}

void task_queue::cancelled(bool cancelled) {
    auto ptr = shared_from_this();

    dispatch_async([=] {
        ptr->resume_loop();
    });
}

void task_queue::resume_loop() {
    std::shared_ptr<cancel_state> cstate;
    
    {
        std::lock_guard<mutex_type> lock(mutex);
        
        state = nullptr;
        cstate = get_cancel_state();
        
        running.test_and_set();
    }
    
    run_tasks(cstate);
}

void task_queue::pause() {
    std::lock_guard<mutex_type> lock(mutex);

    paused = true;
}

void task_queue::resume() {
    std::lock_guard<mutex_type> lock(mutex);

    paused = false;
    begin_loop();
}
