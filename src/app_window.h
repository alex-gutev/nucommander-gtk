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

#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <unordered_map>

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <glibmm.h>

#include "cleanup_n.h"
#include "file_view.h"

#include "tasks/task_queue.h"

#include "errors/errors.h"
#include "errors/error_dialog.h"

#include "interface/dest_dialog.h"

namespace nuc {
    /**
     * Main Application Window.
     *
     * Creates the file view widgets and adds them to the paned view
     * container.
     */
    class app_window : public Gtk::ApplicationWindow {
    protected:
        /**
         * The builder object used to created the window widget.
         */
        Glib::RefPtr<Gtk::Builder> builder;

        /**
         * The paned view container widget.
         */
        Gtk::Paned *pane_view;

        /**
         * Left file view widget.
         */
        file_view *left_view;
        /**
         * Right file view widget.
         */
        file_view *right_view;


        /**
         * Error dialog: displays error details and a list of recovery
         * options.
         */
        error_dialog *err_dialog = nullptr;

        /**
         * Destination dialog: Queries the user for a destination
         * path.
         */
        nuc::dest_dialog *m_dest_dialog = nullptr;

        /**
         * Operation task queue, onto which file operations are
         * queued.
         */
        std::shared_ptr<task_queue> operations{task_queue::create()};


        /* Initialization Methods */

        /**
         * Creates and adds the file view widgets to the paned view
         * container, and sets up the focus chain.
         */
        void init_pane_view();
        /**
         * Creates a file view widget, stores the pointer to it in
         * 'ptr' and adds to the paned containter, with 'pane'
         * indicating which pane to add it to.
         *
         * ptr:  Reference to where the pointer to the widget is to be
         *       stored.
         *
         * pane: 1 - to add the file view to the left pane.
         *       2 - to add the file view to the right pane.
         */
        void add_file_view(file_view * &ptr, int pane);

        /**
         * Creates a new file view widget builder.
         */
        Glib::RefPtr<Gtk::Builder> file_view_builder();


        /* Signal Handlers */

        /**
         * Signal handler for the key press event on each file_view.
         *
         * @param e The event.
         * @param src The file_view in which the event was triggered.
         */
        bool on_keypress(const GdkEventKey *e, file_view *src);

        /**
         * Signal handler for the activate entry signal.
         *
         * @param src The file_view in which the entry was activated.
         * @param flist The file_view's file list controller.
         * @param ent The activated entry.
         */
        void on_entry_activate(file_view *src, file_list_controller *flist, dir_entry *ent);

        /**
         * Opens the file at @a path, with the default application for
         * the file type.
         *
         * Performs synchronous IO to determine the file type thus
         * should only be run from the task queue.
         *
         * @param path Path to the file.
         */
        void open_file(const char *path);


        /* Error handling */

        /**
         * Error handler functor object.
         *
         * Stores a map of error codes and the restarts which should
         * be invoked, when a restart is chosen for all future errors
         * of the same type.
         */
        struct error_handler {
            /**
             * Map of chosen restarts for all errors of a given type.
             *
             * The keys are the error objects and the corresponding
             * values are the identifier names of the chosen restarts.
             */
            std::unordered_map<error, std::string> chosen_actions;

            /**
             * The dialog window.
             */
            app_window *window;

            /**
             * Constructor.
             *
             * @param window The dialog window.
             */
            error_handler(app_window *window) : window(window) {}

            /**
             * Error handler function.
             *
             * @param e The error.
             */
            void operator()(const error &e);
        };


        /* Error Dialog */

        /**
         * Creates the error dialog, and stores a pointer to it in
         * err_dialog, if it has not been created already.
         */
        void create_error_dialog();

        /**
         * Displays the error dialog with a particular error and
         * recovery options.
         *
         * @param promise A promise which is set to the value of the
         *   chosen recovery option, once it is chosen by the user.
         *
         * @param err The error.
         *
         * @param restarts The restart map (recovery options).
         */
        void show_error(error_dialog::action_promise &promise, const error &err, const restart_map &restarts);

        /**
         * Displays the error dialog with the error @e and the
         * recovery options returned by restarts().
         *
         * Blocks until the user chooses an error recovery action.
         *
         * @param e The error.
         *
         * @return A pair consisting of a pointer to the chosen
         *    restart and a flag, which is true if the recovery action
         *    should be applied to all future errors of the same type.
         */
        std::pair<const restart *, bool> choose_action(const error &e);

    public:
        /* Constructor. */
        app_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /* Destructor */
        ~app_window();

        /**
         * Creates a new application window object.
         */
        static app_window *create();

        /**
         * Adds an operation to the operations task queue.
         *
         * @param op The operation to add.
         */
        void add_operation(task_queue::task_type op);

        /**
         * Returns a pointer to the destination dialog.
         *
         * @return Pointer to the destination dialog.
         */
        nuc::dest_dialog *dest_dialog();

        /**
         * Asynchronous cleanup method.
         *
         * @param fn The cleanup function to call once it is safe to
         *           deallocate the object.
         *
         * This method should only be called on the main thread. The
         * function fn will be called on the main thread.
         */
        template <typename F>
        void cleanup(F fn);
    };
}

template <typename F>
void nuc::app_window::cleanup(F fn) {
    auto cleanup_fn = cleanup_n_fn(2, fn);

    left_view->cleanup(cleanup_fn);
    right_view->cleanup(cleanup_fn);
}

#endif // APP_WINDOW_H

// Local Variables:
// mode: c++
// End:
