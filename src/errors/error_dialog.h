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

        /**
         * Sets the contents of the error label to the description of
         * the error @a e.
         *
         * @param e The error.
         */
        void set_error_label(const error &e);

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
         * Signal handler for key press events.
         */
        bool key_pressed(const GdkEventKey *e);

    public:
        using Gtk::Dialog::run;

        /** Response Typres */
        enum response_type {
            /**
             * A restart was chosen to handle this single error only.
             */
            RESPONSE_CHOOSE = Gtk::RESPONSE_OK,
            /**
             * No restart was chosen, default to abort.
             */
            RESPONSE_ABORT = Gtk::RESPONSE_NONE,
            /**
             * A restart was chosen to handle this error and all
             * future errors of the same type.
             */
            RESPONSE_ALL = 1
        };


        /** Constructor */
        error_dialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new error dialog.
         */
        static error_dialog *create();


        /**
         * Sets the label to display the description of the error @a
         * err, and shows the lists of applicable restarts.
         *
         * @param err The error.
         * @param restarts The restart map.
         */
        void set_error(const error &err, const restart_map &restarts);

        /**
         * Gets the selected restart. Restart to 'restart_abort' if no
         * restart is selected.
         *
         * @return Pointer to the restart.
         */
        const restart *get_restart() const;

        /**
         * Displays the dialog, modally, displaying the error @a err
         * and the list of applicable restarts in @a restarts.
         *
         * The dialog is automatically hidden after a restart is
         * chosen or it is closed by the user.
         *
         * @param err The error.
         * @param restarts The restarts.
         *
         * @return A pair with the first element being the chosen
         *   restart and the second element being a boolean, which is
         *   true if the chosen restart should be executed for all
         *   future errors of the same type.
         */
        std::pair<const restart *, bool> run(const error &err, const restart_map &restarts);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
