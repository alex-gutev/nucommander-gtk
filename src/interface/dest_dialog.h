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

#ifndef NUC_INTERFACE_DEST_DIALOG_H
#define NUC_INTERFACE_DEST_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>

#include "paths/pathname.h"

namespace nuc {
    /**
     * Dialog that queries the user for a destination path.
     */
    class dest_dialog : public Gtk::Dialog {
        /**
         * Destination chosen function type.
         *
         * Takes one parameter the destination path entered.
         */
        typedef std::function<void(const Glib::ustring &)> chose_dest_fn;

        /**
         * The label displaying the destination path query message.
         */
        Gtk::Label *query_label;

        /**
         * The execute button.
         */
        Gtk::Button *exec_button;

        /**
         * The cancel button.
         */
        Gtk::Button *cancel_button;

        /**
         * Destination path text entry.
         */
        Gtk::Entry *dest_entry;


        /**
         * Callback function which is executed after the user has
         * entered a destination path.
         */
        chose_dest_fn dest_chosen;


        /* Initialization Methods */

        /* Signal Handlers */

        /**
         * Signal handler for the "clicked" event of the execute action
         * button.
         */
        void exec_clicked();

        /**
         * Signal handler for the "clicked" event of the "cancel"
         * button.
         */
        void cancel_clicked();;

        /**
         * Called just after the dialog is shown.
         */
        void on_show() override;

        /**
         * Signal handler for the "deleted" signal. Called when the
         * user closes the dialog.
         *
         * Hides the dialog preventing it from being deleted.
         */
        bool on_delete(const GdkEventAny *e);

    public:
        using Gtk::Dialog::show;

        /** Constructor */
        dest_dialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new destination dialog.
         */
        static dest_dialog *create();

        /**
         * Sets the contents of the query label.
         *
         * @param text The query text.
         */
        void set_query_label(Glib::ustring text) {
            query_label->set_label(std::move(text));
        }

        /**
         * Sets the title of the exec button.
         *
         * @param label The label text.
         */
        void set_exec_button_label(Glib::ustring label) {
            exec_button->set_label(std::move(label));
        }

        /**
         * Sets the contents of the destination path text entry.
         *
         * @param label The label text.
         */
        void dest_path(Glib::ustring label) {
            dest_entry->set_text(std::move(label));
        }

        /**
         * Returns the contents of the destination path text entry.
         *
         * @return The destination path.
         */
        Glib::ustring dest_path() const {
            return dest_entry->get_text();
        }

        /**
         * Displays the dialog and blocks until it is closed.
         *
         * @return The response code.
         */
        int run();

        /**
         * Shows the dialog.
         *
         * @param chose_fn Function which is called after the user
         *   enters a destination path.
         */
        void show(chose_dest_fn chose_fn);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
