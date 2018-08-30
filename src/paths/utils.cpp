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

#include "utils.h"

#include <algorithm>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


nuc::paths::string nuc::paths::file_name(const string &path) {
    if (path.length()) {
        // If path ends in slash search from previous character
        size_t end = path.back() == '/' ? path.length() - 2 : path.length();

        // Find last slash
        size_t slash_pos = path.rfind('/', end);

        if (slash_pos != std::string::npos) {
            return path.substr(slash_pos + 1, end - slash_pos);
        }
    }

    return path;
}

nuc::paths::string nuc::paths::file_extension(const string &path) {
    // Find last '.' or '/'
    size_t pos = path.find_last_of("./");

    // If a character was found and it is not '/', return the
    // substring beginning from next character.
    if (pos != std::string::npos && path[pos] != '/') {
        return path.substr(pos + 1);
    }

    // No characters found, return empty string.
    return std::string();
}

void nuc::paths::append_component(string &path, const string &comp) {
    // If 'path' is not an empty string and does end in a slash,
    // append a slash.
    if (path.size() && path.back() != '/')
        path.append("/");

    path.append(comp);
}

nuc::paths::string nuc::paths::appended_component(string path, const string &comp) {
    append_component(path, comp);
    return path;
}


void nuc::paths::remove_last_component(nuc::paths::string &path) {
    size_t pos = path.rfind('/');

    if (pos == string::npos) {
        pos = 0;
    }
    else if (pos == 0) {
        pos = 1;
    }

    path.resize(pos);
}

nuc::paths::string nuc::paths::removed_last_component(nuc::paths::string path) {
    remove_last_component(path);
    return path;
}


nuc::paths::string nuc::paths::path_from_components(const std::vector<string> &comps) {
    string path;

    for (const string &comp : comps) {
        append_component(path, comp);
    }

    return path;
}

nuc::paths::string nuc::paths::canonicalized_path(const string &path) {
    path_components comps(path);
    std::vector<string> new_comps;

    for (string comp : comps) {
        if (comp == "..") {
            // If more than one component and last component is not '..'
            if (new_comps.size() && new_comps.back() != "..") {
                new_comps.pop_back();
            }
            else {
                // If first component, simply add to array
                new_comps.push_back(std::move(comp));
            }
        }
        else if (comp != "." && comp != "") {
            new_comps.push_back(std::move(comp));
        }
    }

    return path_from_components(new_comps);
}

bool nuc::paths::is_root(const string &path) {
    // TODO: add more sophisticated checks for non-local file systems
    return path == "/";
}

bool nuc::paths::is_child_of(string dir, const string &path) {
    if (!dir.empty() && dir.back() != '/') {
        dir.push_back('/');
    }

    if (dir.size() >= path.size()) return false;

    auto res = std::mismatch(dir.begin(), dir.end(), path.begin());

    return res.first == dir.end() && std::find(res.second, path.end(), '/') == path.end();
}

bool nuc::paths::is_prefix(const string &str1, const string &str2) {
    if (str1.size() > str2.size()) return false;

    auto res = std::mismatch(str1.begin(), str1.end(), str2.begin());

    return res.first == str1.end();
}


nuc::paths::string nuc::paths::expand_tilde(const string &path) {
    if (!path.empty() && path.front() == '~') {
        string::size_type pos = path.find_first_of('/');

        const char *home = NULL;

        if (path.length() == 1 || pos == 1) {
            if (!(home = getenv("HOME"))) {
                if (struct passwd *pw = getpwuid(getuid())) {
                    home = pw->pw_dir;
                }
            }
        }
        else {
            string user(path, 1, pos == string::npos ? string::npos : pos - 1);

            if (struct passwd *pw = getpwnam(user.c_str())) {
                return home = pw->pw_dir;
            }
        }

        if (home) {
            std::string full_path(home);

            if (pos != string::npos) {
                append_component(full_path, path.substr(pos+1));
            }

            return full_path;
        }
    }

    return path;
}
