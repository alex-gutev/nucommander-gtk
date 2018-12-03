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

#define BOOST_TEST_MODULE path_utils
#include <boost/test/unit_test.hpp>

#include <string>
#include <iostream>

#include "paths/utils.h"

BOOST_AUTO_TEST_SUITE(path_components);

BOOST_AUTO_TEST_CASE(simple_path) {
    nuc::paths::string path = "/simple/file/path";

    nuc::paths::path_components comps(path);
    auto it = comps.begin();

    BOOST_CHECK(*it == "/");
    ++it;
    BOOST_CHECK(*it == "simple");
    ++it;
    BOOST_CHECK(*it == "file");
    ++it;
    BOOST_CHECK(*it == "path");
    ++it;
    BOOST_CHECK(it == comps.end());
}

BOOST_AUTO_TEST_CASE(relative_path) {
    nuc::paths::string path = "a/relative/../path";

    nuc::paths::path_components comps(path);
    auto it = comps.begin();

    BOOST_CHECK(*it == "a");
    ++it;
    BOOST_CHECK(*it == "relative");
    ++it;
    BOOST_CHECK(*it == "..");
    ++it;
    BOOST_CHECK(*it == "path");
    ++it;
    BOOST_CHECK(it == comps.end());
}

BOOST_AUTO_TEST_CASE(duplicate_slashes) {
    nuc::paths::string path = "//path/with//duplicate/slashes";

    nuc::paths::path_components comps(path);
    auto it = comps.begin();

    BOOST_CHECK(*it == "/");
    ++it;
    BOOST_CHECK(*it == "path");
    ++it;
    BOOST_CHECK(*it == "with");
    ++it;
    BOOST_CHECK(*it == "duplicate");
    ++it;
    BOOST_CHECK(*it == "slashes");
    ++it;
    BOOST_CHECK(it == comps.end());
}

BOOST_AUTO_TEST_CASE(trailing_slash) {
    nuc::paths::string path = "/path/with/trailing/slash/";

    nuc::paths::path_components comps(path);
    auto it = comps.begin();

    BOOST_CHECK(*it == "/");
    ++it;
    BOOST_CHECK(*it == "path");
    ++it;
    BOOST_CHECK(*it == "with");
    ++it;
    BOOST_CHECK(*it == "trailing");
    ++it;
    BOOST_CHECK(*it == "slash");

    BOOST_CHECK(it != comps.end());
    ++it;
    BOOST_CHECK(it == comps.end());
}

BOOST_AUTO_TEST_CASE(all_components) {
    nuc::paths::string path = "/simple/file/path";

    std::vector<nuc::paths::string> all = nuc::paths::path_components::all(path);
    std::vector<nuc::paths::string> expected({"/", "simple", "file", "path"});

    BOOST_CHECK(all == expected);
}

BOOST_AUTO_TEST_CASE(url_paths) {
    nuc::paths::string url = "ftp://user@example.org/some/dir";

    nuc::paths::path_components comps(url);
    auto it = comps.begin();

    BOOST_CHECK(*it == "ftp:");
    ++it;
    BOOST_CHECK(*it == "user@example.org");
    ++it;
    BOOST_CHECK(*it == "some");
    ++it;
    BOOST_CHECK(*it == "dir");
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(utility_functions);

BOOST_AUTO_TEST_CASE(file_name) {
    nuc::paths::string file = nuc::paths::file_name("/path/to/a/file.txt");

    BOOST_CHECK(file == "file.txt");
}

BOOST_AUTO_TEST_CASE(file_name_ends_slash) {
    nuc::paths::string file = nuc::paths::file_name("/path/to/dir/");

    BOOST_CHECK(file == "dir");
}

BOOST_AUTO_TEST_CASE(file_name_no_path) {
    nuc::paths::string file = nuc::paths::file_name("file.txt");

    BOOST_CHECK(file == "file.txt");
}


BOOST_AUTO_TEST_CASE(file_extension) {
    nuc::paths::string ext = nuc::paths::file_extension("/path/file.txt");

    BOOST_CHECK(ext == "txt");
}

BOOST_AUTO_TEST_CASE(file_extension_in_path) {
    nuc::paths::string ext = nuc::paths::file_extension("/path/dir.d/file");

    BOOST_CHECK(ext == "");
}

BOOST_AUTO_TEST_CASE(file_extension_many) {
    nuc::paths::string ext = nuc::paths::file_extension("/path/to/file.tar.gz");

    BOOST_CHECK(ext == "gz");
}

BOOST_AUTO_TEST_CASE(file_extension_none) {
    nuc::paths::string ext = nuc::paths::file_extension("somefile");

    BOOST_CHECK(ext == "");
}

BOOST_AUTO_TEST_CASE(file_extension_empty) {
    nuc::paths::string ext = nuc::paths::file_extension("");

    BOOST_CHECK(ext == "");
}


BOOST_AUTO_TEST_CASE(append_component) {
    nuc::paths::string path = "/path/to/a";

    nuc::paths::append_component(path, "file.txt");

    BOOST_CHECK(path == "/path/to/a/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_slash) {
    nuc::paths::string path = "/path/to/a/";

    nuc::paths::append_component(path, "file.txt");

    BOOST_CHECK(path == "/path/to/a/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_root) {
    nuc::paths::string path = "/";

    nuc::paths::append_component(path, "file.txt");

    BOOST_CHECK(path == "/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_empty) {
    nuc::paths::string path;

    nuc::paths::append_component(path, "/");

    BOOST_CHECK(path == "/");
}


BOOST_AUTO_TEST_CASE(path_from_components) {
    std::vector<nuc::paths::string> comps({"/", "path", "to", "file.txt"});
    nuc::paths::string path = nuc::paths::path_from_components(comps);

    BOOST_CHECK(path == "/path/to/file.txt");
}

BOOST_AUTO_TEST_CASE(path_from_components_dir) {
    std::vector<nuc::paths::string> comps({"..", "path", "to", "dir", ""});
    nuc::paths::string path = nuc::paths::path_from_components(comps);

    BOOST_CHECK(path == "../path/to/dir/");
}

BOOST_AUTO_TEST_CASE(path_from_components_empty_components) {
    std::vector<nuc::paths::string> comps({"/", "", "path", "", "to/", "file"});
    nuc::paths::string path = nuc::paths::path_from_components(comps);

    BOOST_CHECK(path == "/path/to/file");
}


BOOST_AUTO_TEST_CASE(canonicalized_path) {
    nuc::paths::string path = nuc::paths::canonicalized_path("a/relative/../path/./");

    BOOST_CHECK(path == "a/path");
}

BOOST_AUTO_TEST_CASE(canonicalized_path_relative) {
    nuc::paths::string path = nuc::paths::canonicalized_path("a/relative/../../../path/./dir");

    BOOST_CHECK(path == "../path/dir");
}

BOOST_AUTO_TEST_CASE(canonicalized_path_malformed) {
    nuc::paths::string path = nuc::paths::canonicalized_path(".././../a///bad/path/");

    BOOST_CHECK(path == "../../a/bad/path");
}

BOOST_AUTO_TEST_CASE(canonicalized_path_root_parent) {
    nuc::paths::string path = nuc::paths::canonicalized_path("/../dir/file");

    BOOST_CHECK(path == "dir/file");
}

BOOST_AUTO_TEST_CASE(prefix_path) {
    BOOST_CHECK(nuc::paths::is_prefix("/root/foo/", "/root/foo/bar"));
    BOOST_CHECK(!nuc::paths::is_prefix("/root/foo/", "/root/foobar"));
    BOOST_CHECK(!nuc::paths::is_prefix("/root/foo/bar", "/root/foo"));
}

BOOST_AUTO_TEST_CASE(child_path) {
    BOOST_CHECK(nuc::paths::is_child_of("/root/foo/", "/root/foo/bar"));
    BOOST_CHECK(nuc::paths::is_child_of("/root/foo", "/root/foo/bar"));

    BOOST_CHECK(!nuc::paths::is_child_of("/root/foo", "/root/foobar"));
    BOOST_CHECK(!nuc::paths::is_child_of("/root/foo", "/root/foo/bar/baz"));
}

BOOST_AUTO_TEST_CASE(is_subpath) {
    BOOST_CHECK(nuc::paths::is_subpath("/root/foo/", "/root/foo/bar"));
    BOOST_CHECK(nuc::paths::is_subpath("/root/foo/", "/root/foo/bar/baz"));

    BOOST_CHECK(!nuc::paths::is_subpath("/root/foo", "/root/foo/bar"));
    BOOST_CHECK(!nuc::paths::is_subpath("/root/foo", "/root/foobar"));
}

BOOST_AUTO_TEST_SUITE_END();
