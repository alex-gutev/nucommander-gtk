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

#ifndef NUC_ERRORS_ERROR_H
#define NUC_ERRORS_ERROR_H

#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <exception>

#include <boost/any.hpp>

#include <glibmm/ustring.h>

#include "tasks/task_queue.h"

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
         * Unique constant identifying the type of error in the
         * context of the application, e.g. a read error, write error.
         */
        int m_type = 0;

        /**
         * Flag: true if the operation can be retried.
         */
        bool m_can_retry;

        /**
         * String explaining the error. If empty the string is assumed
         * to be a system error constant with the error description
         * obtained using strerror.
         */
        Glib::ustring error_string;

    public:

        /**
         * Error type codes.
         */
        enum type_code {
            // General Error type
            type_general = 0,

            // File Errors
            type_create_file,
            type_write_file,
            type_read_file,

            type_rename_file,
            type_delete_file,

            // Directory Errors
            type_create_dir,

            // Attribute Errors
            type_set_mode,
            type_set_owner,
            type_set_times
        };


        /**
         * Constructs an error exception.
         *
         * @param code The error code.
         *
         * @param can_retry Flag for whether the operation can be
         *    retried.
         */
        error(int code, bool can_retry = true) : error(code, 0, can_retry) {}

        /**
         * Constructs an error exception object.
         *
         * @param code The error code.
         *
         * @param type The error type code.
         *
         * @param can_retry Flag for whether the operation can be
         *    retried.
         */
        error(int code, int type, bool can_retry) : error(code, type, can_retry, "") {}

        /**
         * Constructs an error exception object.
         *
         * @param code The error code.
         *
         * @param type The error type code.
         *
         * @param can_retry Flag for whether the operation can be
         *    retried.
         *
         * @param error_string String describing the error. Not used
         *    if empty.
         */
        template <typename T>
        error(int code, int type, bool can_retry, T&&error_string) :
            m_code(code), m_type(type), m_can_retry(can_retry), error_string(std::forward<T>(error_string)) {}

        virtual ~error() = default;

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
        bool can_retry() const noexcept {
            return m_can_retry;
        }

        /**
         * Returns a string explaining the error.
         *
         * The default implementation simply returns the system error
         * description, returned by strerror.
         *
         * This method is not thread-safe and should thus only be
         * called from the main thread.
         *
         * @return A string explaining the error.
         */
        virtual Glib::ustring explanation() const noexcept;

        /**
         * Returns a string explaining the type of error that
         * occurred.
         *
         * By default returns an empty string;
         *
         * @return A string describing the error type.
         */
        virtual Glib::ustring type_explanation() const noexcept {
            return "";
        }

        /**
         * Returns A unique constant identifying the type of error.
         */
        int error_type() const noexcept {
            return m_type;
        }
    };

    /**
     * Error equality comparison operator.
     *
     * Two error objects are considered equal if they have the same
     * error code and type.
     */
    inline bool operator==(const error &e1, const error &e2) {
        return e1.code() == e2.code() && e1.error_type() == e2.error_type();
    }

    /**
     * Error comparison operator.
     *
     * @a e1 is considered less than @a e2 if its error_type is less
     * than the error_type of @a e2 and its error code is less than
     * the error code of @a e2.
     *
     * If either @a e1, @a e2, have an error code of zero, the error
     * code is not taken into consideration in the comparison.
     */
    inline bool operator<(const error &e1, const error &e2) {
        return e1.error_type() < e2.error_type() && e1.code() && e2.code() && e1.code() < e2.code();
    }


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
         * Applicable function. Tests whether the restart is
         * applicable to a given error.
         *
         * Takes the error exception as an argument.
         */
        typedef std::function<bool(const error &)> applicable_fn;

        /**
         * A string identifying the restart.
         */
        const std::string name;
        /**
         * The error handling action function.
         */
        const action_fn action;

        /**
         * Checks whether the restart is applicable to a given error.
         *
         * @param e The error exception.
         *
         * @return True if the restart is applicable to the given
         *   error, false otherwise.
         */
        const applicable_fn applicable;

        /**
         * Creates a restart with an identifier and error handling
         * action function.
         *
         * @param name String identifier of the restart.
         * @param action Action function.
         * @param applicable Applicable function.
         */
        restart(std::string name, action_fn action, applicable_fn applicable = [] (const error &) { return true; })
                : name(name), action(action), applicable(applicable) {}

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
     * Restart map type.
     *
     * Holds a map of restarts where each key is unique string
     * identifying a restart and the corresponding value is the
     * "restart" object.
     */
    typedef std::unordered_map<std::string, restart> restart_map;

    /**
     * Returns a reference to the global thread-local restart map.
     */
    restart_map &restarts();

    /**
     * Establishes a global restart and adds it to the global
     * thread-local restart map. The restart is automatically removed
     * from the map when the destructor is called.
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
         * restart @a r. The restart is added to the restart map.
         *
         * The restart should not have an identifier which is equal to
         * an identifier of an already established restart.
         *
         * @param r The restart to establish/
         */
        global_restart(restart r);

        /**
         * Removes the established restart from the restart map.
         */
        ~global_restart();
    };

    /**
     * Retry restart.
     *
     * Does nothing resulting in the failed operation being retried.
     */
    const restart restart_retry("retry", [] (const error &, boost::any) {});

    /**
     * Abort restart.
     *
     * Rethrows the error exception, thus aborting the operation.
     */
    const restart restart_abort("abort", [] (const error &e, boost::any) {
        throw;
    });

    /**
     * Returns a map mapping error type names to type codes.
     *
     * @return An unordered_map.
     */
    std::unordered_map<std::string, int> &error_type_map();
    /**
     * Returns a map mapping error names to error codes.
     *
     * @return An unordered_map.
     */
    std::unordered_map<std::string, int> &error_code_map();


    /* Automatic Error Handlers */

    /**
     * Returns the map of default automatic error handlers, retrieved
     * from settings.
     *
     * The map keys are error objects with the error_type and code
     * set, and the corresponding values are the string identifiers of
     * the restarts.
     *
     * @return Error handler map.
     */
    std::map<error, std::string> auto_error_handlers();

    /* Error handler functions */

    /**
     * Error handler function. Takes one argument the error.
     */
    typedef std::function<void(const error &)> error_handler_fn;

    /**
     * Global thread local error handler function. This function is
     * called when an operation fails.
     */
    extern thread_local error_handler_fn global_error_handler;

    /**
     * Establishes a scoped thread local global error handler.
     */
    struct error_handler {
        /* The previous global error handler */
        error_handler_fn old_handler;

        /**
         * Constructor.
         *
         * @param handler The handler to set as the global error
         *    handler for the lifetime of this object.
         *
         * @param call_old If true the previous error handler is
         *   called if @a handler returns normally.
         */
        template <typename F>
        error_handler(F&& handler, bool call_old = false) {
            old_handler = global_error_handler;

            if (call_old && old_handler) {
                global_error_handler = [=] (const error &e) {
                    handler(e);
                    old_handler(e);
                };
            }
            else {
                global_error_handler = std::forward<F>(handler);
            }
        }

        /**
         * Restores global_error_handler to its previous value.
         */
        ~error_handler() {
            global_error_handler = std::move(old_handler);
        }
    };

    /**
     * Cancellable error handler function type.
     *
     * Arguments:
     *
     *  - Cancellation state.
     *  - The error exception.
     */
    typedef std::function<void(cancel_state &, const error &)> cancellable_handler;

    /**
     * Creates a new task which runs the task @a task with the error
     * handler function @a handler as the global error handler.
     */
    template<typename F>
    task_queue::task_type with_error_handler(F task, cancellable_handler handler) {
        using namespace std::placeholders;

        return [=] (cancel_state &state) {
            error_handler e([&state, handler] (const error &e) {
                handler(state, e);
            });

            task(state);
        };
    }

    /**
     * Repeatedly attempts an operation @a op, until it is
     * successful. The global error handler function is called when an
     * exception is thrown from @a op.
     *
     * @param op The operation function to attempt.
     */
    template<typename F>
    void try_op(F op) {
        try_op(op, global_error_handler);
    }

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
    template<typename Fop, typename Fh>
    void try_op(Fop op, Fh&& handler);
}

/* Template Implementation */

template<typename Fop, typename Fh>
void nuc::try_op(Fop op, Fh&& handler) {
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

// Error object hash function
namespace std {
    template <> struct hash<nuc::error> {
        size_t operator()(const nuc::error &e) const {
            hash<size_t> h{};
            size_t hash = (h(e.code()) * 2654435761U) ^ h(e.error_type());

            return hash;
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
