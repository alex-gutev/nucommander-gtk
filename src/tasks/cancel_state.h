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

#ifndef NUC_CANCEL_STATE_H
#define NUC_CANCEL_STATE_H

#include <atomic>
#include <exception>
#include <functional>

namespace nuc {
    /**
     * Cancellation state.
     *
     * Implements the cancellation logic, used by the operation class.
     */
    class cancel_state {
    public:
        /**
         * Finish callback function type.
         *
         * Prototype: void(bool cancelled)
         *
         * @param cancelled  true if the operation was cancelled.
         */
        typedef std::function<void(bool)> finish_fn;
        
        /**
         * Operation cancelled exception.
         */
        class cancelled : public std::exception {};

        
        /** Default constructor */
        cancel_state() = default;

        /**
         * Disable copying as the cancellation state should not be
         * copied. If it is accidentally copied, it is most likely a
         * bug.
         */
        cancel_state(const cancel_state &) = delete;
        cancel_state& operator=(const cancel_state &) = delete;

        
        /**
         * Enters the "no cancel" state. Throws a 'cancelled'
         * exception, if the operation has been cancelled.
         */
        void enter_no_cancel();
        
        /**
         * Exits the "no cancel" state. Throws a 'cancelled' exception
         * if the operation was cancelled whilst in the no cancel
         * state.
         */
        void exit_no_cancel();
        
        /**
         * Tests whether the operation has been cancelled.  If the
         * operation has been cancelled, a cancelled exception is
         * thrown.
         */
        void test_cancel();
        
        /**
         * Executes the callable 'f' in the "no cancel" state,
         * effectively the call to 'f' is preceeded by
         * 'enter_no_cancel()', and followed by 'exit_no_cancel()'.
         */
        template <typename F>
        void no_cancel(F f) {
            enter_no_cancel();
            f();
            exit_no_cancel();
        }
        

        /**
         * Cancels the operation.
         */
        void cancel();

        /**
         * Calls the finish callback functions, if they have not been
         * called already, and atomically sets finished to true.
         *
         * @param cancelled True if the operation was cancelled.
         */
        void call_finish(bool cancelled);

        /**
         * Adds a finish callback function, which is called once the
         * operation finishes (call_finish() is called) or is
         * cancelled.
         *
         * The callback function is guaranteed to be called only once,
         * after the operation finishes or is cancelled. If the
         * operation is cancelled while in the "no cancel" state, the
         * function is called as soon as the operation attempts to
         * return to the "can cancel" state (exit_no_cancel() is
         * called).
         *
         * This function is not atomic, thus should only be called
         * when in the no cancel state or before the operation is run.
         *
         * @param fn    The finish callback function.
         *
         * @param after If true the callback is called after the
         *    previous callbacks, otherwise it is called before the
         *    previous callbacks.
         *
         */
        void add_finish_callback(finish_fn fn, bool after = true);
        
    private:
        /**
         * Cancellation state constants.
         */
        enum {
            /** Operation can be cancelled. */
            CAN_CANCEL = 0,
            /** In the no cancel state. */
            NO_CANCEL = 1,
            /** Operation has been cancelled. */
            CANCELLED  = 2
        };
        
        /** Cancellation State. */
        std::atomic<int> state {CAN_CANCEL};
        
        /**
         * Boolean flag for whether the finish signal has been
         * emitted.
         */
        std::atomic_flag finished = ATOMIC_FLAG_INIT;

        /**
         * Finish callback function.
         */
        finish_fn m_finish;
    };
}

#endif // NUC_CANCEL_STATE_H

// Local Variables:
// mode: c++
// End:
