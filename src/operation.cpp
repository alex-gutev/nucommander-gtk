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

#include "operation.h"
#include "async_task.h"

using namespace nuc;

void operation::run() {
    dispatch_async([=] {
        bool cancelled = false;
        
        try {
            main();
        }
        catch (cancel_state::cancelled &e) {
            cancelled = true;
        }
        
        state.call_finish(cancelled);
        release();
    });
}

void operation::cancel() {
    state.cancel();
}

void operation::release() {
    if (--ref_count == 0) {
        delete this;
    }
}

void operation::free() {
    delete this;
}
