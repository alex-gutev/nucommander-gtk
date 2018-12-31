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

#include "cancel_state.h"

using namespace nuc;

void cancel_state::call_finish(bool cancelled) {
    if (!finished.test_and_set()) {
        m_finish(cancelled);
    }
}

void cancel_state::enter_no_cancel() {
    int exp_val = CAN_CANCEL;
    
    if (!state.compare_exchange_strong(exp_val, NO_CANCEL))
        throw cancelled();
}

void cancel_state::exit_no_cancel() {
    int exp_val = NO_CANCEL;
    
    if (!state.compare_exchange_strong(exp_val, CAN_CANCEL)) {
        call_finish(true);
        throw cancelled();
    }
}

void cancel_state::test_cancel() {
    if (state.load() == CANCELLED)
        throw cancelled();
}

void cancel_state::cancel() {
    // Set cancel state to cancelled
    int val = state.exchange(CANCELLED);
    
    if (val == CAN_CANCEL) {
        call_finish(true);
    }
}

void cancel_state::add_finish_callback(finish_fn fn, bool after) {
    finish_fn prev_fn = m_finish;

    if (!prev_fn) {
        m_finish = fn;
    }
    else if (after) {
        m_finish = [=] (bool cancelled) {
            prev_fn(cancelled);
            fn(cancelled);
        };
    }
    else {
        m_finish = [=] (bool cancelled) {
            fn(cancelled);
            prev_fn(cancelled);
        };
    }
}

void cancel_state::call_progress(const progress_event &event) {
    if (progress) {
        no_cancel([&] {
            progress(event);
        });
    }
}
