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
    template <typename T>
    class async_queue {
        typedef std::queue<T> queue;
        typedef std::lock_guard<std::mutex> lock_guard;

        std::queue<T> q;
        mutable std::mutex m_mutex;
        
    public:
        
        std::mutex mutex() {
            return m_mutex;
        }
        std::mutex mutex() const {
            return m_mutex;
        }
        
        void lock() {
            m_mutex.lock();
        }
        void unlock() {
            m_mutex.unlock();
        }

        typename async_queue::queue::reference front() {
            lock_guard l(m_mutex);
            return q.front();
        }
        typename async_queue::queue::const_reference front() const {
            lock_guard l(m_mutex);
            return q.front();
        }
        
        typename async_queue::queue::reference back() {
            lock_guard l(m_mutex);
            return q.back();
        }
        typename async_queue::queue::const_reference back() const {
            lock_guard l(m_mutex);
            return q.back();
        }

        bool empty() const {
            lock_guard l(m_mutex);
            return q.empty();
        }

        typename async_queue::queue::size_type size() const {
            lock_guard l(m_mutex);
            return q.size();
        }

        void push(const T &value) {
            lock_guard l(m_mutex);
            q.push(value);
        }
        void push(T&& value) {
            lock_guard l(m_mutex);
            q.push(std::move(value));
        }

        template<class... Args>
        void emplace(Args&&... args) {
            lock_guard l(m_mutex);
            q.emplace(std::forward<Args...>(args...));
        }

        void pop() {
            lock_guard l(m_mutex);
            q.pop();
        }
        
        bool pop(T &item) {
            lock_guard l(m_mutex);
        
            if (q.empty()) return false;
        
            item = q.front();
            q.pop();
            
            return true;
        }        
    };
}

#endif // NUC_ASYNC_QUEUE_H
