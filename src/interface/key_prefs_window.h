/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef NUC_INTERFACE_KEY_PREFS_DIALOG_H
#define NUC_INTERFACE_KEY_PREFS_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "paths/pathname.h"

namespace nuc {
    /**
     * Key bindings Preferences Dialog
     */
    class key_prefs_window : public Gtk::Window {
        /* Key Binding Preferences */

        /**
         * Model Columns for the key
         */
        struct model_columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> command;
            Gtk::TreeModelColumn<Glib::ustring> key;

            model_columns();
        };

        /** Column Model */
        model_columns model;

        /**
         * List store model - storing the list of key bindings.
         */
        Glib::RefPtr<Gtk::ListStore> bindings_list;

        /**
         * Key bindings tree view.
         */
        Gtk::TreeView *bindings_view;

        /* Add and remove binding buttons */
        Gtk::Button *kb_add_button;
        Gtk::Button *kb_remove_button;


        /** Buttons */

        Gtk::Button *ok_button;
        Gtk::Button *apply_button;
        Gtk::Button *cancel_button;


        /* Key Binding Preferences */

        /**
         * Retrieves the key bindings from settings and stores them in
         * bindings_list.
         */
        void get_bindings();

        /**
         * Saves the key bindings in bindings_list to settings.
         */
        void store_bindings();

        /**
         * Add a new key binding to the tree model and set the tree
         * views's cursor to the created row.
         */
        void add_binding();

        /**
         * Remove the selected key binding from the tree model.
         */
        void remove_binding();


        /* Signal Handlers */

        /**
         * Signal handlers of the clicked events of the buttons.
         */
        void apply_clicked();
        void ok_clicked();
        void cancel_clicked();

        /**
         * Default delete event handler.
         *
         * Calls gtk_widget_hide_on_delete to prevent the dialog from
         * being deleted.
         *
         * @param      e The event.
         *
         * @return     Returns true.
         */
        bool on_delete(const GdkEventAny *e);

        /**
         * Default key press event handler.
         *
         * @param      e The event.
         *
         * @return     True if the event was handled.
         */
        bool on_key_press_event(GdkEventKey *e);

    public:
        /** constructor */
        key_prefs_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new instance of the key-bindings preferences
         * dialog.
         *
         * @return The key_prefs_dialog instance.
         */
        static key_prefs_window *create();

        /**
         * Returns the shared instance of the dialog.
         *
         * @return The shared instance.
         */
        static key_prefs_window *instance();

        /**
         * Shows the dialog.
         *
         * Refreshes the key-bindings list if the dialog is not
         * already visible.
         */
        void show();
    };
}

#endif

// Local Variables:
// mode: c++
// End:
