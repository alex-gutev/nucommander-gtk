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

#include "pathname.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


using namespace nuc::paths;

pathname pathname::from_components(const std::vector<string> &comps, bool is_dir) {
    pathname path;

    for (const string &comp : comps) {
        std::move(path).append(comp);
    }

    path.ensure_trail_slash(is_dir);

    return path;
}

void pathname::ensure_trail_slash(bool is_dir) {
    if (m_path.size()) {
        if (is_dir && m_path.back() != '/')
            m_path.push_back('/');
        else if (!is_dir && m_path.back() == '/' && m_path.size() > 1)
            m_path.pop_back();
    }
}


bool pathname::is_dir() const {
    return m_path.size() && m_path.back() == '/';
}

std::vector<string> pathname::components() const {
    std::vector<string> components;

    size_t pos = m_path.find('/');

    if (pos == 0) {
        components.push_back("/");
    }
    else {
        components.push_back(m_path.substr(0, pos));
    }

    while (pos < m_path.length() - 1) {
        pos++;

        size_t next_pos = m_path.find('/', pos);
        size_t count = next_pos - pos;

        if (count) {
            components.push_back(m_path.substr(pos, count));
        }

        pos = next_pos;
    }

    return components;
}


pathname& pathname::append(const nuc::paths::pathname &path) && {
    if (m_path.size() && !is_dir())
        m_path.push_back('/');

    m_path.append(path.m_path);
    return *this;
}

pathname pathname::append(const nuc::paths::pathname &path) const & {
    return pathname(*this).append(path);
}


pathname& pathname::remove_last_component() && {
    if (m_path.size() > 1) {
        bool dir = is_dir();
        size_t pos = m_path.rfind('/', m_path.size() - (dir ? 2 : 0));

        if (pos == string::npos) {
            pos = 0;
            dir = false;
        }
        else if (!pos || dir) {
            pos++;
        }

        m_path.resize(pos);
    }
    else {
        m_path.clear();
    }

    return *this;
}

pathname pathname::remove_last_component() const & {
    return pathname(*this).remove_last_component();
}


pathname& pathname::merge(const nuc::paths::pathname &path) && {
    if (path.is_relative()) {
        if (!is_dir())
            *this = remove_last_component();

        *this = append(path);
    }
    else {
        m_path = path.m_path;
    }

    return *this;
}

pathname pathname::merge(const nuc::paths::pathname &path) const & {
    return pathname(*this).merge(path);
}


pathname& pathname::canonicalize(bool is_dir) && {
    std::vector<string> new_comps;

    for (string &comp : components()) {
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

    m_path = from_components(new_comps, is_dir).m_path;

    return *this;
}

pathname pathname::canonicalize(bool is_dir) const & {
    return pathname(*this).canonicalize(is_dir);
}


pathname pathname::expand_tilde() const {
    if (!m_path.empty() && m_path.front() == '~') {
        size_t pos = m_path.find_first_of('/');
        const string &tilde = m_path.substr(0, pos);

        const char *home = NULL;

        if (tilde.length() == 1) {
            if (!(home = getenv("HOME"))) {
                if (struct passwd *pw = getpwuid(getuid())) {
                    home = pw->pw_dir;
                }
            }
        }
        else if (struct passwd *pw = getpwnam(tilde.substr(1).c_str())) {
                return home = pw->pw_dir;
        }

        if (home) {
            return pos != string::npos ? pathname(home).append(m_path.substr(pos+1)) : pathname(home);
        }
    }

    return *this;
}


string pathname::basename() const {
    if (m_path.length()) {
        // If path ends in slash search from previous character
        size_t end = m_path.length() - (is_dir() ? 2 : 0);
        // Find last slash
        size_t last_slash = m_path.rfind('/', end);

        if (last_slash == string::npos) {
            last_slash = -1;
        }

        return m_path.substr(last_slash + 1, end - last_slash);
    }

    return m_path;
}

string pathname::extension() const {
    string name = basename();

    size_t pos = name.rfind('.');

    if (pos != string::npos) {
        return name.substr(pos + 1);
    }

    return string();
}

size_t pathname::basename_offset() const {
    // If path ends in slash search from previous character
    size_t end = m_path.length() - (is_dir() ? 2 : 0);
    // Find last slash
    size_t offset = m_path.rfind('/', end);

    return offset == string::npos ? 0 : offset + 1;
}


bool pathname::is_root() const {
    return m_path == "/";
}

bool pathname::is_relative() const {
    return m_path.empty() || (m_path.front() != '/' && m_path.front() != '~');
}


bool pathname::is_child_of(const nuc::paths::pathname &parent) const {
    if (is_subpath(parent)) {
        size_t offset = m_path.rfind('/', m_path.size() - (is_dir() ? 2 : 0));

        if (offset == string::npos)
            offset = 0;

        return offset == parent.path().size();
    }

    return false;
}

bool pathname::is_subpath(const nuc::paths::pathname &parent, bool check_dir) const {
    if (parent.empty()) return true;
    if (parent.path().size() >= m_path.size()) return false;

    if (check_dir && !parent.is_dir()) return false;

    auto res = std::mismatch(parent.path().begin(), parent.path().end(), m_path.begin());

    if (res.first == parent.path().end()) {
        return parent.is_dir() || *res.second == '/';
    }

    return false;
}

bool pathname::has_dirs() const {
    return m_path.find('/') != string::npos;
}


size_t pathname::subpath_offset(const std::set<paths::pathname> &paths, const nuc::paths::pathname &path) {
    // Iterator to first path not less than path i.e. either equal to
    // path or the first path which is not a parent of path
    auto it = paths.lower_bound(path);

    if (paths.begin() != paths.end()) {
        if (it == paths.end() || *it != path)
            --it;

        // If the iterator points to a directory equal to path or is a
        // directory and a parent directory of path.
        if (*it == path || path.is_subpath(*it, true)) {
            return it->basename_offset();
        }
    }

    return string::npos;
}

// Local Variables:
// mode: c++
// End:
