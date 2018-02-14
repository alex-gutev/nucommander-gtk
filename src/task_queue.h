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

#ifndef NUC_TASK_QUEUE_H
#define NUC_TASK_QUEUE_H

#include <functional>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>

#include "cancel_state.h"

namespace nuc {
    /**
     * Background task queue.
     *
     * Implements a queue on which tasks are added and executed,
     * serially, on a background thread. The queue can be "cancelled",
     * which results in the running task being cancelled, using the
     * same cancellation procedure as the operation class, and the
     * removal of the remaining queued tasks, from the queue.
     *
     * Unlike the operation class, a task_queue is not a one-shot
     * object thus can continue being used even after the running task
     * is cancelled.
     */
    class task_queue {
        /**
         * Task functor type.
         *
         * Prototype: void(cancel_state &state)
         *
         * @param state  Reference to the cancellation state object.
         */
        typedef std::function<void(cancel_state &)> task_type;

        
        /**
         * Pointer to the current cancellation state object.
         */
        std::shared_ptr<cancel_state> state;

        /**
         * Mutex for synchronizing access to the queue and
         * cancellation state pointer.
         */
        std::mutex mutex;

        /**
         * The task queue.
         */
        std::queue<task_type> queue;

        /**
         * Flag: True if there is a background task loop running.
         */
        std::atomic_flag running = ATOMIC_FLAG_INIT;
        

        /* Methods */

        /**
         * Runs the task loop.
         *
         * At each iteration of the loop, a task is dequeued and
         * executed. If there are no more tasks to execute the running
         * flag is cleared. The loop is surrounded in a try-catch,
         * with the cancel_state::cancelled exception being caught. In
         * this case the loop exits without clearing the running flag.
         *
         * @param state  The cancellation state to test for cancellation.
         */
        void run_tasks(std::shared_ptr<cancel_state> state);

        /**
         * Dequeues the next task if any, and stores it in task.
         *
         * First it is checked whether the cancellation state is set
         * to cancelled, using cancel_state::test_cancel. If the state
         * is set to cancelled, a cancel_state::cancelled exception is
         * thrown.
         *
         * If the queue is not empty, the task at the head of the
         * queue is dequeued, stored in task and true is returned.
         *
         * If the queue is empty, task is not modified, the running
         * flag is cleared, and false is returned.
         *
         * @param state  The cancellation state to test for cancellation.
         * @param task   Reference, in which, the task dequeued will be stored.
         *
         * @return true if there is a task to execute (stored in task)
         *         false if there are no more tasks to execute.
         */
        bool next_task(std::shared_ptr<cancel_state> state, task_type &task);

        /**
         * Returns a pointer to the current cancellation state. If
         * there is no current cancellation state (state is null), a
         * new cancellation state is created, stored in state, and
         * returned.
         *
         * This function does not lock the mutex, thus should only be
         * called after locking the mutex.
         */
        std::shared_ptr<cancel_state> get_cancel_state();
        
        /**
         * Cancelled signal handler.
         */
        void cancelled(bool cancelled);
        
    public:
        /**
         * Adds a new task to the queue. If there is no currently
         * executing background task, a new task loop is initiated on
         * a background thread.
         */
        void add(task_type task);

        /**
         * Adds a new task to the queue and connects a callback to the
         * finish/cancel signal of the task's cancellation state.
         *
         * This callback is called after the task finishes or when the
         * task is cancelled, guaranteed to be called only once.
         *
         * The callback might not be called if the task is cancelled
         * before it has run.
         *
         * @param task    The task function.
         * @param finish  The finish callback function.
         */
        template <typename T, typename F>
        void add(T task, F finish);
        
        /**
         * Cancels the running task and removes all queued tasks from
         * the queue.
         *
         * Tasks can still be added to the queue, after calling this
         * function. The tasks will be executed as soon as the
         * finished/cancelled signal is emitted on the current
         * cancellation state (when signal_cancel is emitted).
         */
        void cancel();
    };
}


/* Template Implementation */

template <typename T, typename F>
void nuc::task_queue::add(T task, F finish) {
    add([=] (cancel_state &state) {
        state.no_cancel([&state, &finish] {
            state.signal_finish().connect(finish);
        });
        
        task(state);
        
        state.call_finish(false);
        cancel();
    });
}

#endif // NUC_TASK_QUEUE_H

// Local Variables:
// mode: c++
// End:
