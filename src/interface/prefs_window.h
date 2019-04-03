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
#include <gtkmm/spinbutton.h>

#include "paths/pathname.h"

namespace nuc {
    /**
     * Key bindings Preferences Dialog
     */
    class prefs_window : public Gtk::Window {
    public:
        /** constructor */
        prefs_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new instance of the key-bindings preferences
         * dialog.
         *
         * @return The key_prefs_dialog instance.
         */
        static prefs_window *create();

        /**
         * Returns the shared instance of the dialog.
         *
         * @return The shared instance.
         */
        static prefs_window *instance();

        /**
         * Shows the dialog.
         *
         * Refreshes the key-bindings list if the dialog is not
         * already visible.
         */
        void show();

    private:
        /* General Preferences */

        Gtk::SpinButton *refresh_timeout_entry;


        /* Key Binding Preferences */

        struct kb_model_columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> command;
            Gtk::TreeModelColumn<Glib::ustring> key;

            kb_model_columns();
        };

        kb_model_columns kb_model;

        Glib::RefPtr<Gtk::ListStore> bindings_list;
        Gtk::TreeView *bindings_view;

        Gtk::Button *kb_add_button;
        Gtk::Button *kb_remove_button;


        /* Plugin Settings */

        struct plugin_model_columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> path;
            Gtk::TreeModelColumn<Glib::ustring> regex;

            plugin_model_columns();
        };

        plugin_model_columns plugin_model;

        Glib::RefPtr<Gtk::ListStore> plugins_list;
        Gtk::TreeView *plugins_view;

        Gtk::Button *plugin_add_button;
        Gtk::Button *plugin_remove_button;


        /* Error Handler Settings */

        struct eh_model_columns : public Gtk::TreeModelColumnRecord {
            Gtk::TreeModelColumn<Glib::ustring> type;
            Gtk::TreeModelColumn<int> code;
            Gtk::TreeModelColumn<Glib::ustring> restart;

            eh_model_columns();
        };

        eh_model_columns eh_model;

        Glib::RefPtr<Gtk::ListStore> eh_list;
        Gtk::TreeView *eh_view;

        Gtk::Button *eh_add_button;
        Gtk::Button *eh_remove_button;


        /** Buttons */

        Gtk::Button *ok_button;
        Gtk::Button *apply_button;
        Gtk::Button *cancel_button;


        /* General Prefrences */

        /**
         * Initializes the widgets on the general page.
         *
         * @param builder The builder object.
         */
        void init_general(const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Retrieves the general settings from GSettings.
         */
        void get_general_settings();
        /**
         * Stores the general settings in GSettings.
         */
        void store_general_settings();


        /* Key Binding Preferences */

        void init_keybindings(const Glib::RefPtr<Gtk::Builder> &builder);

        void get_bindings();
        void store_bindings();


        /* Plugin Settings */

        void init_plugins(const Glib::RefPtr<Gtk::Builder> &builder);

        void get_plugins();
        void store_plugins();


        /* Error Handler Settings */

        void init_error_handlers(const Glib::RefPtr<Gtk::Builder> &builder);

        void get_error_handlers();
        void store_error_handlers();


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
    };
}

#endif

// Local Variables:
// mode: c++
// End:
