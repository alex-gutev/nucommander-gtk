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

void task_queue::add(task_type task) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.emplace(task);
    
    if (!running.test_and_set()) {
        std::shared_ptr<cancel_state> state = get_cancel_state();
        dispatch_async([=] {
            run_tasks(state);
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
    std::lock_guard<std::mutex> lock(mutex);
    
    state->test_cancel();
    
    if (!queue.empty()) {
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
        state->signal_finish().connect(sigc::mem_fun(*this, &task_queue::cancelled));
    }
    
    return state;
}



void task_queue::cancel() {
    std::queue<task_type> old_queue;
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        
        queue.swap(old_queue);
        state->cancel();
    }
}


void task_queue::cancelled(bool cancelled) {
    m_signal_cancel.emit();
    
    dispatch_async([=] {
        std::shared_ptr<cancel_state> cstate;
        
        {
            std::lock_guard<std::mutex> lock(mutex);
            
            state = nullptr;
            cstate = get_cancel_state();
            
            running.test_and_set();
        }
        
        run_tasks(cstate);
    });
}


