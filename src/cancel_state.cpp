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
        m_signal_finish.emit(cancelled);
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

cancel_state::signal_finish_type cancel_state::signal_finish() {
    return m_signal_finish;
}
