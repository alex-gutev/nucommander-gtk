/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  <copyright holder> <email>
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

#ifndef NUC_CLEANUP_ALL_H
#define NUC_CLEANUP_ALL_H

#include <atomic>

namespace nuc {
    /**
     * Asynchronous cleanup functor, which calls a cleanup function
     * after being called 'n' times.
     *
     * This is used when an object has a number of child objects which
     * need to be cleaned up asynchronously, using their cleanup()
     * methods. This functor can be passed to 'n' child objects'
     * cleanup methods and the cleanup function will only be called
     * after it is called from all 'n' objects.
     *
     * The functor can safely be called from multiple threads.
     */
    template <typename F>
    class cleanup_n {
        /**
         * Cleanup count - the number of times the functor must be
         * called until it calls the cleanup function 'fn'.
         *
         * Initially this is 'n' (number of objects) and decremented
         * after each call.
         *
         * This is a pointer as it has to be shared between 'n' copies
         * of the functor.
         */
        std::atomic<int> *count;

        /**
         * The cleanup function to call after the functor has been
         * called 'n' times.
         */
        F fn;
        
    public:
        /**
         * Constructor.
         *
         * @param n Number of objects - number of times the functor
         *          must be called before the cleanup function is
         *          called.
         *
         * @param fn The cleanup function.
         */
        cleanup_n(int n, F fn) : count(new std::atomic<int>(n)), fn(fn) {}

        /**
         * Atomically decrements the cleanup count. If it is 0 calls
         * the cleanup function.
         */
        void operator()() const {
            if (!--(*count)) {
                delete count;
                fn();
            }
        }
    };

    /**
     * Creates a cleanup_n functor.
     */
    template <typename F>
    cleanup_n<F> cleanup_n_fn(int n, F fn) {
        return cleanup_n<F>(n, fn);
    }
}

#endif // NUC_CLEANUP_ALL_H

// Local Variables:
// mode: c++
// End:
