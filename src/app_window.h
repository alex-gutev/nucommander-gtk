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

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <glibmm.h>

#include "cleanup_n.h"
#include "file_view.h"

#include "tasks/task_queue.h"

#include "errors/error_dialog.h"

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
         * Operation task queue, onto which file operations are
         * queued.
         */
        std::shared_ptr<task_queue> operations{task_queue::create()};

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

        /**
         * Signal handler for the key press event on each file_view.
         *
         * @param e The event.
         * @param src The file_view in which the event was triggered.
         */
        bool on_keypress(const GdkEventKey *e, file_view *src);


        /* Error handling */

        /**
         * File operation error handler.
         *
         * Displays a dialog with a list of error recovery options
         * (restarts).
         *
         * @param e The error.
         */
        void handle_error(const error &e);


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
        void show_error(std::promise<const restart *> &promise, const error &err, const restart_map &restarts);


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
