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
    class operation {
        /**
         * Cancellation state constants
         */
        enum {
            /** Operation can be cancelled */
            CAN_CANCEL = 0,
            /** In a no cancel block. */
            NO_CANCEL = 1,
            /** Operation has been cancelled. */
            CANCELLED  = 2
        };
        
        /**
         * Reference Count.
         */
        std::atomic<int> ref_count {2};
        
        /**
         * Cancellation state
         */
        std::atomic<int> cancel_state {CAN_CANCEL};
        
        /**
         * Boolean flag for whether finish has been called.
         */
        std::atomic_flag finished;
        
        
        /**
         * Calls the finish() method if it has not been called
         * already.
         */
        void call_finish(bool cancelled);
        
    protected:
        
        /**
         * Destructor. Protected as the class takes are of its own
         * deallocation, and thus should not be stack allocated or
         * externally managed.
         */
        virtual ~operation() = default;
        
    public:
        
        /**
         * Enters the "no cancel" state.
         */
        void enter_no_cancel();
        
        /**
         * Exits the "no cancel" state
         */
        void exit_no_cancel();
        
        /**
         * Executes the callable 'f' in a "no cancel" state,
         * that is the call to 'f' is preceeded by 'enter_no_cancel()',
         * and followed by 'exit_no_cancel()'.
         */
        template <typename F>
        void no_cancel(F f) {
            enter_no_cancel();
            f();
            exit_no_cancel();
        }
        
        /**
         * Operation cancelled exception
         */
        class cancelled : public std::exception {
        };
        
        /**
         * Runs the operation on a background thread
         */
        void run();
        
        /**
         * Cancels the operation.
         */
        void cancel();
        
        /**
         * Releases the object, signals to the object that it can
         * deallocate itself once the background operation completes.
         * 
         * This method should be called once the external reference is no
         * longer required, the object will be deallocated once the operation
         * completes (when 'main' returns). If the operation has already
         * completed, it is deallocated immediately.
         */
        void release();
        
        /**
         * Operation main method.
         */
        virtual void main() = 0;
        
        /**
         * Operation finish method.
         * 
         * Guaranteed to be called once, after the operation finishes
         * or has been cancelled. This method may be called on a background
         * thread or on the same thread from which 'cancel' is called.
         */
        virtual void finish(bool cancelled) {}
    };
}

#endif // NUC_OPERATION_H
