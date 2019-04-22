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

#ifndef NUC_SETTINGS_APP_SETTINGS_H
#define NUC_SETTINGS_APP_SETTINGS_H

#include <giomm/settings.h>

#if GLIBMM_MAJOR_VERSION == 2 && GLIBMM_MINOR_VERSION < 54
#include "variant.h"
#endif

namespace nuc {
    class app_settings {
    public:
        app_settings();

        static app_settings &instance();

        Glib::RefPtr<Gio::Settings> settings() {
            return m_settings;
        }


        /* Retrieving and Setting Settings */

        /**
         * Retrieves the directory refresh timeout from settings.
         *
         * @return The timeout.
         */
        int dir_refresh_timeout() const;

        /**
         * Sets the value of the directory refresh timeout in
         * settings.
         *
         * @param timeout The timeout value to set.
         */
        void dir_refresh_timeout(int timeout);


        /**
         * Returns the array of the identifiers of the columns which
         * should be displayed.
         *
         * @return Vector of column string identifiers.
         */
        std::vector<std::string> columns() const;

        /**
         * Sets the array of the identifiers of the columns which
         * should be displayed.
         *
         * @param columns Vector of column string identifiers.
         */
        void columns(const std::vector<std::string> &columns);


        /**
         * Returns the name of the column by which file lists should
         * be sorted initially.
         *
         * @return The column name.
         */
        std::string default_sort_column() const;

        /**
         * Sets the name of the column by which file lists should
         * be sorted initially.
         *
         * @param column The column name.
         */
        void default_sort_column(const std::string &column);


        /**
         * Returns the keybindings map.
         *
         * @return The map.
         */
        std::map<Glib::ustring, Glib::ustring> keybindings() const;

    private:
        /**
         * ID of the GSettings schema of the application's settings.
         */
        static constexpr const char *settings_id = "org.agware.NuCommander";

        /**
         * GSettings object.
         */
        Glib::RefPtr<Gio::Settings> m_settings;

        /**
         * Cached value of the directory refresh timeout.
         */
        int m_dir_refresh_timeout;
    };
}

#endif

// Local Variables:
// mode: c++
// End:
