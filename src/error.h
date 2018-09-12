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

#ifndef NUC_ERROR_H
#define NUC_ERROR_H

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <exception>

#include <boost/any.hpp>

/**
 * Error handling functions.
 */

namespace nuc {
    /**
     * Error exception.
     */
    class error : public std::exception {
        /**
         * Error code.
         */
        int m_code;

        /**
         * Flag: true if the operation can be retried.
         */
        bool m_can_retry;

    public:

        /**
         * Constructs an error exception.
         *
         * @param code The error code.
         *
         * @param can_retry Flag for whether the operation can be
         *    retried.
         */
        error(int code, bool can_retry = true) : m_code(code), m_can_retry(can_retry) {}

        /**
         * Returns the error code identifying the error.
         *
         * @return The error code.
         */
        int code() const {
            return m_code;
        }

        /**
         * Returns true if the operation, which triggered the error,
         * can be retired.
         *
         * @return True if the operation can be retried, false
         *   otherwise.
         */
        bool can_retry() const {
            return m_can_retry;
        }
    };


    /* Restarts */

    /**
     * A function object containing an error handling action which is
     * invoked when the function object is invoked.
     */
    struct restart {
        /**
         * Error handling action function.
         *
         * Takes the following parameters:
         *
         *  - error: The error exception.
         *  - arg: An argument, of any type, for use by the function.
         */
        typedef std::function<void(const error &, boost::any)> action_fn;

        /**
         * A string identifying the restart.
         */
        const std::string name;
        /**
         * The error handling action function.
         */
        const action_fn action;

        /**
         * Creates a restart with an identifier and error handling
         * action function.
         *
         * @param name String identifier of the restart.
         * @param action Action function.
         */
        restart(std::string name, action_fn action) : name(name), action(action) {}

        /**
         * Invokes the restart error handling action function.
         *
         * @param error The error which was raised.
         *
         * @param arg An extra argument, of any type, provided to the
         *    error handling function. What this argument should
         *    contain or is used for, is dependent on the restart.
         */
        void operator() (const error &err, boost::any arg = nullptr) const {
            action(err, arg);
        }
    };

    /**
     * Map of established restarts (error handler functions)
     * where the keys are the restart identifiers and the
     * corresponding values are the restarts.
     */
    extern thread_local std::unordered_map<std::string, restart> restarts;


    /**
     * Establishes a global restart and adds it to the map stored in
     * the global thread local variable restarts. The restart is
     * automatically removed from the map when the destructor is
     * called.
     */
    class global_restart {
        /**
         * Restart identifier string.
         */
        std::string name;

    public:

        global_restart(const global_restart &r) = delete;
        global_restart &operator = (const global_restart &r) = delete;

        /**
         * Creates a global_restart object which establishes the
         * restart @a r. The restart is added to the restarts map.
         *
         * The restart should not have an identifier which is equal to
         * an identifier of an already established restart.
         *
         * @param r The restart to establish/
         */
        global_restart(restart r);

        /**
         * Removes the established restart from the restarts map.
         */
        ~global_restart();
    };


    const restart restart_retry("retry", [] (const error &, boost::any) {});

    /**
     * Abort restart.
     *
     * Rethrows the error exception, thus aborting the operation.
     */
    const restart restart_abort("abort", [] (const error &e, boost::any) {
        throw;
    });


    /* Error handler functions */

    /**
     * Error handler function. Takes one argument the error.
     */
    typedef std::function<void(const error &)> error_handler_fn;


    /**
     * Repeatedly attempts an operation @a op, until either it is
     * successful (@a op does not throw an exception) or the error
     * handler @a handler throws an exception.
     *
     * Returns normally if @a op returns normally.
     *
     * If an exception, that is a subclass of error, is thrown from @a
     * op, the function @a handler is called and then, if @a handler
     * returns, calls @a op again.
     *
     * @param op The operation function to attempt.
     *
     * @param handler The error handler function to call if @a op
     *    throws an exception.
     */
    template<typename F>
    void try_op(F op, error_handler_fn handler);

    /**
     * Provides error handling functionality for operations performed
     * by the object.
     */
    class restartable {
    protected:
        /**
         * Error handler function
         */
        error_handler_fn err_handler;

        /**
         * Repeatedly calls a function (@a op) until it is successful,
         * that is it does not throw an exception.
         *
         * If @a op throws err_handler is called, after which
         * (provided err_handler does not throw) @a op is called
         * again.
         *
         * @param op The function to call, performing some operation.
         */
        template <typename F>
        void try_op(F op) {
            nuc::try_op(op, err_handler);
        }

    public:

        /**
         * Returns the object's error handler function.
         */
        error_handler_fn error_handler() const {
            return err_handler;
        }

        /**
         * Sets the object's error handler function.
         */
        void error_handler(error_handler_fn fn) {
            err_handler = std::move(fn);
        }
    };
}

/* Template Implementation */

template<typename F>
void nuc::try_op(F op, error_handler_fn handler) {
    while (true) {
        try {
            op();
            break;
        }
        catch (const error &e) {
            handler(e);

            if (!e.can_retry())
                throw;
        }
    }
}

#endif

// Local Variables:
// mode: c++
// End:
