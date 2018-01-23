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

#include "lister.h"

#include "async_task.h"

using namespace nuc;

/**
 * Cancellation state constants
 */
enum {
    /** Not in callback and not cancelled. */
    CAN_CANCEL  = 0,
    /** Currently in the callback. */
    IN_CALLBACK = 1,
    /** Operation has been cancelled. */
    CANCELLED   = 2
};

lister::lister(lister::callback_fn cb, void *ctx) {
    callback = cb;
    context = ctx;
    
    cancel_state = CAN_CANCEL;
    finished.clear();
}

void lister::cancel() {
    // Set cancel state to cancelled
    int val = cancel_state.exchange(CANCELLED);
    
    // If the operation was not cancelled while in the callback, call
    // finish callback.
    if (val == CAN_CANCEL) {
        call_finish(ECANCELED);
    }    
}

void lister::release() {
    if (ref_count.fetch_sub(1) == 1) {
        delete this;
    }
}

void lister::call_finish(int error) {
    // Set finished to true, if finished was not previously true
    // (finish callback has not been called), call callback.
    if (!finished.test_and_set()) {
        err = error;
        callback(this, FINISH, NULL, NULL);
    }
}

int lister::call_callback(lister::stage_t stage, const lister::entry *ent, const struct stat *st) {
    int exp_val = CAN_CANCEL;
    
    // Check that the operation has not been cancelled, and set the
    // cancel state to in callback.
    if (cancel_state.compare_exchange_strong(exp_val, IN_CALLBACK)) {
        callback(this, stage, ent ,st);
        
        // Check that the operation was not cancelled while calling
        // the callback, i.e. check that the state is still 'in
        // callback'.
        exp_val = IN_CALLBACK;
        if (cancel_state.compare_exchange_strong(exp_val, CAN_CANCEL)) {
            return 0;
        }
    }
    
    // The operation has been cancelled, set error to ECANCELED, and
    // return -1
    
    err = ECANCELED;
    return -1;
}

void lister::begin(const lister::path_str &path) {
    cancel_state = CAN_CANCEL;
    
    dispatch_async([=]{
        int err = init(path);
        
        if (!err) {
            read_async();
        }
        else {
            call_finish(err);
        }
        
        release();
    });
}

bool lister::begin(int fd, bool dupfd) {
    if (!reads_fd()) return false;

    cancel_state = CAN_CANCEL;
    
    dispatch_async([=]{
        int err = init(fd, dupfd);
        
        if (!err) {
            read_async();
        }
        else {
            call_finish(err);
        }
        
        release();
    });

    return true;
}


