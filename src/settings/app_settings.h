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

namespace nuc {
    class app_settings {
        /**
         * ID of the GSettings schema of the application's settings.
         */
        static constexpr const char *settings_id = "org.agware.NuCommander";

        /**
         * GSettings object.
         */
        Glib::RefPtr<Gio::Settings> m_settings;

    public:

        /**
         * GSettings path where the plugins' settings are stored.
         */
        static constexpr const char *settings_path = "/org/agware/NuCommander/";

        app_settings() : m_settings(Gio::Settings::create(settings_id)) {}

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
    };
}

#endif

// Local Variables:
// mode: c++
// End:
