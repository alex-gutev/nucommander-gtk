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

#ifndef NUC_INTERFACE_PROGRESS_DIALOG_H
#define NUC_INTERFACE_PROGRESS_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

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
         * Progress bar showing progress on the current file.
         */
        Gtk::ProgressBar *file_progressbar;

        /**
         * Label displaying the name of the directory currently being
         * copied.
         */
        Gtk::Label *dir_label;
        /**
         * Progress bar showing progress on the current directory.
         */
        Gtk::ProgressBar *dir_progressbar;

        /**
         * Box containing the widgets in the dialog.
         */
        Gtk::Box *box;

        /**
         * The cancel button.
         */
        Gtk::Button *cancel_button;

        /**
         * The hide button.
         */
        Gtk::Button *hide_button;

        /**
         * File progress.
         */
        size_t file_prog = 0;
        /**
         * File Size;
         */
        size_t file_size = 0;

        /**
         * Directory Progress
         */
        size_t dir_prog = 0;
        /**
         * Directory Size
         */
        size_t dir_size = 0;


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
         * Set the size of the file, the progress on which is
         * currently being displayed.
         *
         * @param value The file size.
         */
        void set_file_size(size_t value) {
            file_size = value;
        }

        /**
         * Sets the number of bytes copied of the current file.
         *
         * @param value The number of bytes copied.
         */
        void file_progress(size_t value) {
            file_prog = value;

            if (file_size)
                file_progressbar->set_fraction(value / double(file_size));
            else
                file_progressbar->set_fraction(value ? 1 : 0);
        }

        /**
         * Returns the file progress value.
         *
         * @return File progress value.
         */
        size_t file_progress() const {
            return file_prog;
        }

        /**
         * Hides the directory progress bar and label.
         */
        void hide_dir();

        /**
         * Show the directory progress bar and label
         */
        void show_dir();

        /**
         * Sets the directory label.
         *
         * @param dir Name of the directory being copied.
         */
        void set_dir_label(const Glib::ustring &dir) {
            dir_label->set_text(dir);
        }

        /**
         * Sets the size of the directory, the progress on which is
         * currently being displayed.
         *
         * @param value Directory size.
         */
        void set_dir_size(size_t value) {
            dir_size = value;
        }

        /**
         * Sets the directory progress.
         *
         * @param value Directory progress value.
         */
        void dir_progress(size_t value) {
            dir_prog = value;

            if (dir_size)
                dir_progressbar->set_fraction(value / double(dir_size));
            else
                dir_progressbar->set_fraction(0);
        }

        /**
         * Returns the directory progress.
         *
         * @return Directory progress value.
         */
        size_t dir_progress() const {
            return dir_prog;
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
