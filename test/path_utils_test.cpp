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

#define BOOST_TEST_MODULE path_utils testcases
#include <boost/test/unit_test.hpp>

#include <string>
#include <iostream>

#include "path_utils.h"


BOOST_AUTO_TEST_CASE(simple_path) {
    std::string path = "/simple/file/path";
    
    auto comps = nuc::path_components(path);
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
    
    auto comps = nuc::path_components(path);
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
    
    auto comps = nuc::path_components(path);
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
    
    auto comps = nuc::path_components(path);
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
    ++it;
    BOOST_CHECK(*it == "");
    ++it;
    BOOST_CHECK(it == comps.end());    
}

BOOST_AUTO_TEST_CASE(all_components) {
    std::string path ="/simple/file/path";
    
    std::vector<std::string> all = nuc::path_components::all(path);
    std::vector<std::string> expected({"/", "simple", "file", "path"});
    
    BOOST_CHECK(all == expected);
}