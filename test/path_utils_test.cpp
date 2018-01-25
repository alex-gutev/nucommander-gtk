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

#include "path_utils.h"

BOOST_AUTO_TEST_SUITE(path_components);

BOOST_AUTO_TEST_CASE(simple_path) {
    std::string path = "/simple/file/path";
    
    nuc::path_components comps(path);
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
    std::string path = "a/relative/../path";
    
    nuc::path_components comps(path);
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
    std::string path = "//path/with//duplicate/slashes";
    
    nuc::path_components comps(path);
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
    std::string path = "/path/with/trailing/slash/";
    
    nuc::path_components comps(path);
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
    std::string path = "/simple/file/path";
    
    std::vector<std::string> all = nuc::path_components::all(path);
    std::vector<std::string> expected({"/", "simple", "file", "path"});
    
    BOOST_CHECK(all == expected);
}

BOOST_AUTO_TEST_CASE(url_paths) {
    std::string url = "ftp://user@example.org/some/dir";
    
    nuc::path_components comps(url);
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
    std::string file = nuc::file_name("/path/to/a/file.txt");
    
    BOOST_CHECK(file == "file.txt");
}

BOOST_AUTO_TEST_CASE(file_name_no_path) {
    std::string file = nuc::file_name("file.txt");
    
    BOOST_CHECK(file == "file.txt");
}


BOOST_AUTO_TEST_CASE(file_extension) {
    std::string ext = nuc::file_extension("/path/file.txt");
    
    BOOST_CHECK(ext == "txt");
}

BOOST_AUTO_TEST_CASE(file_extension_in_path) {
    std::string ext = nuc::file_extension("/path/dir.d/file");
    
    BOOST_CHECK(ext == "");
}

BOOST_AUTO_TEST_CASE(file_extension_many) {
    std::string ext = nuc::file_extension("/path/to/file.tar.gz");
    
    BOOST_CHECK(ext == "gz");
}

BOOST_AUTO_TEST_CASE(file_extension_none) {
    std::string ext = nuc::file_extension("somefile");
    
    BOOST_CHECK(ext == "");
}

BOOST_AUTO_TEST_CASE(file_extension_empty) {
    std::string ext = nuc::file_extension("");
    
    BOOST_CHECK(ext == "");
}


BOOST_AUTO_TEST_CASE(append_component) {
    std::string path = "/path/to/a";
    
    nuc::append_component(path, "file.txt");
    
    BOOST_CHECK(path == "/path/to/a/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_slash) {
    std::string path = "/path/to/a/";
    
    nuc::append_component(path, "file.txt");
    
    BOOST_CHECK(path == "/path/to/a/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_root) {
    std::string path = "/";
    
    nuc::append_component(path, "file.txt");
    
    BOOST_CHECK(path == "/file.txt");
}

BOOST_AUTO_TEST_CASE(append_component_empty) {
    std::string path;
    
    nuc::append_component(path, "/");
    
    BOOST_CHECK(path == "/");
}


BOOST_AUTO_TEST_CASE(path_from_components) {
    std::vector<std::string> comps({"/", "path", "to", "file.txt"});
    std::string path = nuc::path_from_components(comps);
    
    BOOST_CHECK(path == "/path/to/file.txt");
}

BOOST_AUTO_TEST_CASE(path_from_components_dir) {
    std::vector<std::string> comps({"..", "path", "to", "dir", ""});
    std::string path = nuc::path_from_components(comps);
    
    BOOST_CHECK(path == "../path/to/dir/");
}

BOOST_AUTO_TEST_CASE(path_from_components_empty_components) {
    std::vector<std::string> comps({"/", "", "path", "", "to/", "file"});
    std::string path = nuc::path_from_components(comps);
    
    BOOST_CHECK(path == "/path/to/file");
}


BOOST_AUTO_TEST_CASE(canonicalized_path) {
    std::string path = nuc::canonicalized_path("a/relative/../path/./");
    
    BOOST_CHECK(path == "a/path");
}

BOOST_AUTO_TEST_CASE(canonicalized_path_relative) {
    std::string path = nuc::canonicalized_path("a/relative/../../../path/./dir");
    
    BOOST_CHECK(path == "../path/dir");
}

BOOST_AUTO_TEST_CASE(canonicalized_path_malformed) {
    std::string path = nuc::canonicalized_path(".././../a///bad/path/");
    
    BOOST_CHECK(path == "../../a/bad/path");
}

BOOST_AUTO_TEST_SUITE_END();