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

#ifndef NUC_ERROR_MACROS_H
#define NUC_ERROR_MACROS_H

/**
 * Short-hand for calling the try_op method with a function in which
 * if the expression op returns non-zero, raise_error is called with
 * the value of errno.
 */
#define TRY_OP(op) TRY_OP_(op, raise_error(errno))

/**
 * Short-hand for calling the try_op method with a function in which
 * if the expression op returns non-zero, the expression fail is
 * executed.
 */
#define TRY_OP_(op, fail) try_op([&] { \
			if ((op)) (fail); \
		});

#endif
