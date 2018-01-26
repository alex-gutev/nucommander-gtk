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

#include "operation.h"

namespace nuc {
    
    template <typename FnMain, typename FnFinish>
    class fn_operation : public nuc::operation {
        FnMain fn_main;
        FnFinish fn_finish;
        
    public:
        using operation::no_cancel;
        
        fn_operation(FnMain main, FnFinish finish) : fn_main(main), fn_finish(finish) {}
        
        virtual void main() {
            fn_main(*this);
        }
        virtual void finish(bool cancelled) {
            fn_finish(*this, cancelled);
        }
    };
    
    template <typename FnMain, typename FnFinish>
    operation *make_operation(FnMain main, FnFinish finish) {
        return new fn_operation<FnMain, FnFinish>(main, finish);
    }
}

#endif // NUC_FN_OPERATION_H