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

#ifndef NUC_FN_OPERATION_H
#define NUC_FN_OPERATION_H

#include <utility>

#include "operation.h"

namespace nuc {
    /**
     * operation subclass allowing a callable to be specified as the
     * main method.
     */
    template <typename FnMain>
    class fn_operation : public nuc::operation {
        /** main method callable. */
        FnMain fn_main;
        
    public:
        /**
         * Constructs an operation with a callable object serving as
         * the main method. The callable receives a reference to
         * the cancel_state object as the first argument.
         *
         * The finish argument is connected to the finish signal.
         */
        template <typename Arg1, typename Arg2>
        fn_operation(Arg1&& main, Arg2&& finish) : fn_main(std::forward<Arg1>(main)) {
            state.signal_finish().connect(
                sigc::bind<0, cancel_state&>(std::forward<Arg2>(finish), state));
        }

        /** Method overrides. */
        
        virtual void main() override {
            fn_main(state);
        }
    };

    /**
     * Creates a new fn_operation with main and finish callables.
     *
     * main:   The callable replacing the main method.
     * finish: The callable connected to the finish signal.
     */
    template <typename FnMain, typename FnFinish>
    operation *make_operation(FnMain&& main, FnFinish&& finish) {
        return new fn_operation<FnMain>(std::forward<FnMain>(main), std::forward<FnFinish>(finish));
    }
}

#endif // NUC_FN_OPERATION_H

// Local Variables:
// mode: c++
// End:
