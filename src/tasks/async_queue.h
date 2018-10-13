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

#ifndef NUC_ASYNC_QUEUE_H
#define NUC_ASYNC_QUEUE_H

#include <queue>
#include <mutex>
#include <utility>

namespace nuc {
    /**
     * Thread-safe FIFO queue.
     */
    template <typename T>
    class async_queue {
        /**
         * STL queue container type.
         */
        typedef std::queue<T> queue;
        /**
         * Mutex lock guard type.
         */
        typedef std::lock_guard<std::mutex> lock_guard;

        /**
         * The queue.
         */
        std::queue<T> q;
        /**
         * Mutex for synchronizing access to the queue.
         */
        mutable std::mutex m_mutex;
        
    public:
        /**
         * Returns the mutex.
         */
        std::mutex mutex() {
            return m_mutex;
        }
        std::mutex mutex() const {
            return m_mutex;
        }

        /**
         * Locks the queue, blocks until the mutex is available.
         */
        void lock() {
            m_mutex.lock();
        }
        /**
         * Unlocks the queue.
         */
        void unlock() {
            m_mutex.unlock();
        }

        /**
         * The following member functions lock the queue before
         * executing and unlock the queue before returning.
         */
        
        /**
         * Returns a reference to the head element, the element which
         * will be popped off next.
         */
        typename async_queue::queue::reference front() {
            lock_guard l(m_mutex);
            return q.front();
        }
        typename async_queue::queue::const_reference front() const {
            lock_guard l(m_mutex);
            return q.front();
        }

        /**
         * Returns a reference to the tail element, the element which
         * was pushed on last.
         */
        typename async_queue::queue::reference back() {
            lock_guard l(m_mutex);
            return q.back();
        }
        typename async_queue::queue::const_reference back() const {
            lock_guard l(m_mutex);
            return q.back();
        }

        /**
         * Returns true if the queue is empty.
         */
        bool empty() const {
            lock_guard l(m_mutex);
            return q.empty();
        }

        /**
         * Returns the number of elements in the queue.
         */
        typename async_queue::queue::size_type size() const {
            lock_guard l(m_mutex);
            return q.size();
        }

        /**
         * Pushes an item onto the tail of the queue.
         */
        void push(const T &value) {
            lock_guard l(m_mutex);
            q.push(value);
        }
        void push(T&& value) {
            lock_guard l(m_mutex);
            q.push(std::move(value));
        }

        /**
         * Constructs an item onto the tail of the queue
         *
         * args: The arguments to pass to the constructor of T.
         */
        template<class... Args>
        void emplace(Args&&... args) {
            lock_guard l(m_mutex);
            q.emplace(std::forward<Args...>(args...));
        }

        /**
         * Removes an item off the head of the queue.
         */
        void pop() {
            lock_guard l(m_mutex);
            q.pop();
        }

        /**
         * Removes an item off the head of the queue and stores it in
         * 'item'. If the queue is empty, 'item' is unmodified and
         * false is returned.
         *
         * item: Reference to where the popped off item is to be
         *       stored.
         *
         * Returns true if an item was popped off, false if the queue
         * is empty.
         */
        bool pop(T &item) {
            lock_guard l(m_mutex);
        
            if (q.empty()) return false;
        
            item = std::move(q.front());
            q.pop();
            
            return true;
        }        
    };
}

#endif // NUC_ASYNC_QUEUE_H
