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

#ifndef NUC_OPERATION_H
#define NUC_OPERATION_H

#include <atomic>
#include <exception>

namespace nuc {
    /**
     * Abstract operation interface.
     *
     * Provides an interface for a cancellable background
     * operation. The interface consists of two virtual methods: 'main'
     * and 'finish'.
     *
     * The operation code is placed in the 'main' method, which is
     * executed on a background thread.
     *
     * The 'finish' method is called once either after operation
     * returns, or when the operation is cancelled. The purpose of
     * this method is to inform the owner, of the operation object,
     * that it has finished/been cancelled, and allow the owner to
     * finalize the operation. The operation may still be running in
     * the background (waiting for a blocking system call to return),
     * however as far the owner is concerned the operation should be
     * finished/cancelled when 'finish' method is called. Once the
     * owner no longer needs a reference to the operation it should be
     * released, using the 'release' method.
     *
     * Operations can be cancelled using the 'cancel' method, which
     * immediately sets the state to cancelled and calls the 'finish'
     * method. The operation can entre a "no cancel" state, during
     * which the 'finish' method will not be called. If the operation
     * is cancelled whilst in the "no cancel" state, the 'finish'
     * method will be called when attempting to return to the "can
     * cancel" state. The purpose of the no cancel state is to delay
     * the calling of the 'finish' method whilst results from the
     * operation are being agglomerated. All results should be passed
     * from the operation to the owner whilst in a no cancel
     * state. Blocking system calls, however, should not be performed
     * in a "no cancel" block as this delays cancellation of the
     * operation until the system call returns.
     */
    class operation {
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
        
        /**
         * Reference Count.
         *
         * Initially two for the owner's reference and for the
         * reference of the background operation itself.
         */
        std::atomic<int> ref_count {2};
        
        /**
         * Cancellation State.
         */
        std::atomic<int> cancel_state {CAN_CANCEL};
        
        /**
         * Boolean flag for whether the 'finish' has been called.
         */
        std::atomic_flag finished = ATOMIC_FLAG_INIT;
        
        
        /**
         * Calls the finish() method if it has not been called
         * already.
         */
        void call_finish(bool cancelled);
        
    protected:
        
        /**
         * Destructor. Protected as the class takes care of its own
         * deallocation, and thus should not be stack allocated or
         * externally managed.
         */
        virtual ~operation() = default;
        
    public:
        
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
         * Operation cancelled exception.
         */
        class cancelled : public std::exception {};
        
        /**
         * Runs the operation on a background thread.
         */
        void run();
        
        /**
         * Cancels the operation.
         */
        void cancel();
        
        /**
         * Releases the object, signals to the operation object that
         * it can deallocate itself once it completes.
         * 
         * This method should be called once the owner no longer
         * requires its reference to the operation object.
         */
        void release();
        
        /**
         * Operation main method.
         *
         * This method is run in a background thread and surrounded by
         * a try/catch, handling only the 'cancelled' exception.
         */
        virtual void main() = 0;
        
        /**
         * Operation finish method.
         * 
         * Guaranteed to be called once, after the operation finishes
         * or has been cancelled. This method may be called on a
         * background thread or on the same thread from which 'cancel'
         * is called.
         *
         * Its purpose is to inform the owner of the operation that it
         * has completed/cancelled. The operation may still be running
         * when this method is called, however it should not
         * communicate with the owner.
         *
         * cancelled: true if the operation was cancelled.
         */
        virtual void finish(bool cancelled) {}
    };
}

#endif // NUC_OPERATION_H
