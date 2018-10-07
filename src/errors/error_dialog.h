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
         * The execute action button.
         */
        Gtk::Button *exec_button;

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
         * A function which is executed after a recovery action has
         * been chosen by the user.
         *
         * Arguments:
         *
         *  1. Pointer to the restart object of the chosen action.
         */
        std::function<void(const restart *)> action_chosen;


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
         * Signal handler for the "row_activated" event, triggered
         * when the user clicks on a row in the tree view.
         */
        void row_clicked(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn *column);

    public:
        using Gtk::Dialog::show;

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
        void show(std::promise<const restart *> &promise, const error &err, const restart_map &restarts);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
