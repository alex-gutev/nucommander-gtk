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

#ifndef NUC_ERRORS_ERROR_DIALOG_H
#define NUC_ERRORS_ERROR_DIALOG_H

#include <future>
#include <functional>

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include "error.h"

namespace nuc {
    /**
     * A dialog which displays an error and a list of recovery
     * options.
     */
    class error_dialog : public Gtk::Dialog {
        /**
         * Recovery action chosen callback function type.
         *
         * Arguments:
         *
         *  1. Pointer to the restart object of the chosen action.
         *
         *  2. Flag: true if the chosen action should be applied to
         *     all future errors.
         */
        typedef std::function<void(const restart *, bool)> chose_action_fn;

        /**
         * Columns model for an error recovery action.
         */
        class action_model_columns : public Gtk::TreeModelColumnRecord {
        public:

            /**
             * Name of the option.
             */
            Gtk::TreeModelColumn<Glib::ustring> name;

            /**
             * The error recovery restart.
             */
            Gtk::TreeModelColumn<const restart*> action;

            /** Constructor */
            action_model_columns() {
                add(name);
                add(action);
            }
        };

        /**
         * The label in which the error message is displayed.
         */
        Gtk::Label *error_label;

        /**
         * The execute action button.
         */
        Gtk::Button *exec_button;

        /**
         * Execute action for all future errors button.
         */
        Gtk::Button *all_button;

        /**
         * The tree view in which the list of recovery options is
         * displayed.
         */
        Gtk::TreeView *actions_view;


        /**
         * The list store model storing the list of recovery actions.
         */
        Glib::RefPtr<Gtk::ListStore> actions;

        /**
         * Model columns.
         */
        action_model_columns columns;


        /**
         * Callback function which is executed after the user has
         * chosen a recovery action type.
         */
        chose_action_fn action_chosen;


        /* Initialization Methods */

        /**
         * Initializes the tree view model.
         *
         * Creates the model, sets it as the model of the actions tree
         * view, and adds the columns to the tree view.
         */
        void init_model();

        /**
         * Creates the tree view model.
         *
         * @return The created list store model.
         */
        Glib::RefPtr<Gtk::ListStore> create_model();


        /* Signal Handlers */

        /**
         * Signal handler for the "clicked" event of the execute action
         * button.
         */
        void exec_clicked();

        /**
         * Signal handler for the "clicked" event of the "all" button.
         */
        void all_clicked();

        /**
         * Calls the 'action_chosen' callback function with the
         * currently selected recovery action and hides the dialog.
         *
         * @param all The value of the all flag which should be passed
         *   to the callback function.
         */
        void choose_action(bool all);

        /**
         * Signal handler for the "row_activated" event, triggered
         * when the user clicks on a row in the tree view.
         */
        void row_clicked(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn *column);

        /**
         * Signal handler for the "deleted" signal. Called when the
         * user closes the dialog.
         *
         * Sets the "abort" restart as the chosen action.
         *
         * @param e The event which triggered the delete signal.
         */
        bool on_delete(const GdkEventAny *e);

        /**
         * Signal handler for key press events.
         */
        bool key_pressed(const GdkEventKey *e);

    public:
        using Gtk::Dialog::show;

        /**
         * Action promise type.
         *
         * The promise value is a pair where the first element is a
         * pointer to the chosen restart and the second value is a
         * flag for whether the action should be executed for all
         * future errors.
         */
        typedef std::promise<std::pair<const restart *, bool>> action_promise;

        /** Constructor */
        error_dialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new error dialog.
         */
        static error_dialog *create();

        /**
         * Shows the error dialog, displaying the error @a err and the
         * recovery options @a restarts.
         *
         * When the user has chosen a recovery option, the callback
         * function @a chose_fn is called with the chosen restart.
         *
         * @param err The error to display.
         *
         * @param restarts The restart map at the point at which the
         *    error was triggered.
         *
         * @param chose_fn Callback function which is called, with the
         *   chosen restart passed as an argument when the recovery
         *   action has been chosen by the user.
         */
        void show(const error &err, const restart_map &restarts, chose_action_fn chose_fn);

        /**
         * Shows the error dialog, displaying the error @a err and the
         * recovery options @a restarts.
         *
         * When the user has chosen a recovery option, the value of
         * the promise, @a promise is set to a pointer to the chosen
         * restart.
         *
         * @param promise A promise which is set to the pointer to the
         *    chosen restart, once it is chosen by the user.
         *
         * @param err The error to display.
         *
         * @param restarts The restart map at the point at which the
         *    error was triggered.
         */
        void show(action_promise &promise, const error &err, const restart_map &restarts);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
