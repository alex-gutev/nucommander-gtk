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

#ifndef NUC_INTERFACE_PROGRESS_DIALOG_H
#define NUC_INTERFACE_PROGRESS_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/levelbar.h>

#include "paths/utils.h"

namespace nuc {
    /**
     * Dialog that displays operation progress.
     */
    class progress_dialog : public Gtk::Dialog {
        /**
         * Label displaying the name of the file currently being
         * copied.
         */
        Gtk::Label *file_label;

        /**
         * The cancel button.
         */
        Gtk::Button *cancel_button;

        /**
         * The hide button.
         */
        Gtk::Button *hide_button;

        /**
         * Level bar showing progress on the current file.
         */
        Gtk::LevelBar *file_levelbar;


        /* Initialization Methods */

        /* Signal Handlers */

        /**
         * Signal handler for the "clicked" event of the "cancel"
         * button.
         */
        void cancel_clicked();;

        /**
         * Signal handler for the "clicked" event of the hide button.
         */
        void hide_clicked();

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
        /** Constructor */
        progress_dialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder> &builder);

        /**
         * Creates a new destination dialog.
         */
        static progress_dialog *create();

        /**
         * Set the current file label.
         *
         * @param file Path to the current file.
         */
        void set_file_label(const Glib::ustring &file) {
            file_label->set_text(std::move(file));
        }

        /**
         * Set the maximum value of the file level bar.
         *
         * @param value The maximum value i.e. the file's size.
         */
        void set_file_size(double value) {
            file_levelbar->set_max_value(value);
        }

        /**
         * Sets the number of bytes copied of the current file.
         *
         * @param value The number of bytes copied.
         */
        void file_progress(double value) {
            file_levelbar->set_value(value);
        }

        /**
         * Returns the current value of the file level bar.
         *
         * @return The level bar's value.
         */
        double file_progress() const {
            return file_levelbar->get_value();
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
