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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE path_utils

#include <boost/test/unit_test.hpp>

#include <string>
#include <set>
#include <iostream>

#include "paths/pathname.h"

BOOST_AUTO_TEST_SUITE(constructors)

BOOST_AUTO_TEST_CASE(constructor1) {
    nuc::pathname::string path_str("/foo/bar/baz");
    nuc::pathname path(path_str);

    BOOST_CHECK_EQUAL(path_str, path.path());
}
BOOST_AUTO_TEST_CASE(constructor2) {
    nuc::pathname::string path_str("/foo/bar/baz");
    nuc::pathname path(path_str, true);

    BOOST_CHECK_EQUAL((path_str + "/"), path.path());
}
BOOST_AUTO_TEST_CASE(constructor3) {
    nuc::pathname::string path_str("/foo/bar/baz/");
    nuc::pathname path(path_str, false);

    BOOST_CHECK_EQUAL("/foo/bar/baz", path.path());
}

BOOST_AUTO_TEST_CASE(components1) {
    nuc::pathname path({"foo", "bar", "baz"});

    BOOST_CHECK_EQUAL(path.path(), "foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(components2) {
    nuc::pathname path({"foo", "bar", "baz"}, true);

    BOOST_CHECK_EQUAL(path.path(), "foo/bar/baz/");
}
BOOST_AUTO_TEST_CASE(components3) {
    nuc::pathname path({"/", "foo", "bar", "baz"});

    BOOST_CHECK_EQUAL(path.path(), "/foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(components4) {
    nuc::pathname path{std::vector<nuc::pathname::string>()};

    BOOST_CHECK_EQUAL(path.path(), "");
}
BOOST_AUTO_TEST_CASE(components5) {
    nuc::pathname path(std::vector<nuc::pathname::string>(), true);

    BOOST_CHECK_EQUAL(path.path(), "");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_components)

BOOST_AUTO_TEST_CASE(components1) {
    nuc::pathname path("/foo/bar/baz");

    BOOST_CHECK(!path.is_dir());
    BOOST_CHECK(path.components() == std::vector<nuc::pathname::string>({"/", "foo", "bar", "baz"}));
}
BOOST_AUTO_TEST_CASE(components2) {
    nuc::pathname path("/foo/bar/baz/");

    BOOST_CHECK(path.is_dir());
    BOOST_CHECK(path.components() == std::vector<nuc::pathname::string>({"/", "foo", "bar", "baz"}));
}
BOOST_AUTO_TEST_CASE(components3) {
    nuc::pathname path("foo/bar/baz/");

    BOOST_CHECK(path.components() == std::vector<nuc::pathname::string>({"foo", "bar", "baz"}));
}
BOOST_AUTO_TEST_CASE(components4) {
    nuc::pathname path("foo");

    BOOST_CHECK(path.components() == std::vector<nuc::pathname::string>({"foo"}));
}
BOOST_AUTO_TEST_CASE(components5) {
    nuc::pathname path;

    BOOST_CHECK(path.components().empty());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_ensure_dir)

BOOST_AUTO_TEST_CASE(ensure_dir1) {
    nuc::pathname path("/foo/bar");

    BOOST_CHECK_EQUAL(path.ensure_dir(true).path(), "/foo/bar/");
    BOOST_CHECK_EQUAL(path.ensure_dir(false).path(), "/foo/bar");
}
BOOST_AUTO_TEST_CASE(ensure_dir2) {
    nuc::pathname path("/foo/bar/");

    BOOST_CHECK_EQUAL(path.ensure_dir(true).path(), "/foo/bar/");
    BOOST_CHECK_EQUAL(path.ensure_dir(false).path(), "/foo/bar");
}
BOOST_AUTO_TEST_CASE(ensure_dir3) {
    nuc::pathname path("/");

    BOOST_CHECK_EQUAL(path.ensure_dir(true).path(), "/");
    BOOST_CHECK_EQUAL(path.ensure_dir(false).path(), "/");
}
BOOST_AUTO_TEST_CASE(ensure_dir4) {
    nuc::pathname path("");

    BOOST_CHECK_EQUAL(path.ensure_dir(true).path(), "");
    BOOST_CHECK_EQUAL(path.ensure_dir(false).path(), "");
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_append)

BOOST_AUTO_TEST_CASE(append1) {
    nuc::pathname path1("/foo/bar");
    nuc::pathname path2 = path1.append("baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(append2) {
    nuc::pathname path1("/foo");
    nuc::pathname path2 = path1.append("bar/baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(append3) {
    nuc::pathname path1("/foo/");
    nuc::pathname path2 = path1.append("bar/baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_remove_last_component)

BOOST_AUTO_TEST_CASE(remove_last1) {
    nuc::pathname path = nuc::pathname("/foo/bar/baz").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/foo/bar");
}
BOOST_AUTO_TEST_CASE(remove_last2) {
    nuc::pathname path = nuc::pathname("/foo/bar/baz/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/foo/bar/");
}
BOOST_AUTO_TEST_CASE(remove_last3) {
    nuc::pathname path = nuc::pathname("/foo/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/");
}
BOOST_AUTO_TEST_CASE(remove_last4) {
    nuc::pathname path = nuc::pathname("/foo").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/");
}
BOOST_AUTO_TEST_CASE(remove_last5) {
    nuc::pathname path = nuc::pathname("/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "");
}
BOOST_AUTO_TEST_CASE(remove_last6) {
    nuc::pathname path = nuc::pathname("foo").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_merge)

BOOST_AUTO_TEST_CASE(merge1) {
    nuc::pathname path1("/foo/bar");
    nuc::pathname path2("baz.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/foo/baz.txt");
}
BOOST_AUTO_TEST_CASE(merge2) {
    nuc::pathname path1("/foo/bar/");
    nuc::pathname path2("baz.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/foo/bar/baz.txt");
}
BOOST_AUTO_TEST_CASE(merge3) {
    nuc::pathname path1("/foo/bar/");
    nuc::pathname path2("/baz/file.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/baz/file.txt");
}
BOOST_AUTO_TEST_CASE(merge4) {
    nuc::pathname path1("/foo/bar/");
    nuc::pathname path2("~/baz/file.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "~/baz/file.txt");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_canonicalize)

BOOST_AUTO_TEST_CASE(canonicalize1) {
    nuc::pathname path = nuc::pathname("a/relative/../path/./").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "a/path");
}
BOOST_AUTO_TEST_CASE(canonicalize2) {
    nuc::pathname path = nuc::pathname("a/relative/../path/./").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "a/path/");
}
BOOST_AUTO_TEST_CASE(canonicalize3) {
    nuc::pathname path = nuc::pathname("a/relative/../../../path/./dir").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "../path/dir");
}
BOOST_AUTO_TEST_CASE(canonicalize4) {
    nuc::pathname path = nuc::pathname("a/relative/../../../path/./dir").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "../path/dir/");
}
BOOST_AUTO_TEST_CASE(canonicalize5) {
    nuc::pathname path = nuc::pathname(".././../a///bad/path/").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "../../a/bad/path");
}
BOOST_AUTO_TEST_CASE(canonicalize6) {
    nuc::pathname path = nuc::pathname(".././../a///bad/path/").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "../../a/bad/path/");
}
BOOST_AUTO_TEST_CASE(canonicalize7) {
    nuc::pathname path = nuc::pathname("/../dir/file").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "dir/file");
}
BOOST_AUTO_TEST_CASE(canonicalize8) {
    BOOST_CHECK_EQUAL(nuc::pathname("/").canonicalize().path(), "/");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_basename)

BOOST_AUTO_TEST_CASE(basename1) {
    nuc::pathname path("/foo/bar/baz.txt");

    BOOST_CHECK_EQUAL(path.basename(), "baz.txt");
    BOOST_CHECK_EQUAL(path.extension(), "txt");
}
BOOST_AUTO_TEST_CASE(basename2) {
    nuc::pathname path("/foo/bar/baz");

    BOOST_CHECK_EQUAL(path.basename(), "baz");
    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(basename3) {
    nuc::pathname path("/foo/bar/baz/");

    BOOST_CHECK_EQUAL(path.basename(), "baz");
    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(basename4) {
    nuc::pathname path("foo");

    BOOST_CHECK_EQUAL(path.basename(), "foo");
}
BOOST_AUTO_TEST_CASE(basename5) {
    nuc::pathname path("/");

    BOOST_CHECK_EQUAL(path.basename(), "");
}

BOOST_AUTO_TEST_CASE(extension1) {
    nuc::pathname path("hello.txt");

    BOOST_CHECK_EQUAL(path.extension(), "txt");
}
BOOST_AUTO_TEST_CASE(extension2) {
    nuc::pathname path("dir.ext/hello.txt");

    BOOST_CHECK_EQUAL(path.extension(), "txt");
}
BOOST_AUTO_TEST_CASE(extension3) {
    nuc::pathname path("no_extension");

    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(extension4) {
    nuc::pathname path(".config");

    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(extension5) {
    nuc::pathname path("/dir/.config");

    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(extension6) {
    nuc::pathname path("file.");

    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(extension7) {
    nuc::pathname path("hello.txt.gz");

    BOOST_CHECK_EQUAL(path.extension(), "gz");
}

BOOST_AUTO_TEST_CASE(filename1) {
    nuc::pathname path("hello.txt");

    BOOST_CHECK_EQUAL(path.filename(), "hello");
}
BOOST_AUTO_TEST_CASE(filename2) {
    nuc::pathname path("dir.ext/hello.txt");

    BOOST_CHECK_EQUAL(path.filename(), "hello");
}
BOOST_AUTO_TEST_CASE(filename3) {
    nuc::pathname path("no_extension");

    BOOST_CHECK_EQUAL(path.filename(), "no_extension");
}
BOOST_AUTO_TEST_CASE(filename4) {
    nuc::pathname path(".config");

    BOOST_CHECK_EQUAL(path.filename(), ".config");
}
BOOST_AUTO_TEST_CASE(filename5) {
    nuc::pathname path("/dir/.config");

    BOOST_CHECK_EQUAL(path.filename(), ".config");
}
BOOST_AUTO_TEST_CASE(filename6) {
    nuc::pathname path("file.");

    BOOST_CHECK_EQUAL(path.filename(), "file.");
}
BOOST_AUTO_TEST_CASE(filename7) {
    nuc::pathname path("hello.txt.gz");

    BOOST_CHECK_EQUAL(path.filename(), "hello.txt");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_is_type_functions)

BOOST_AUTO_TEST_CASE(is_root1) {
    nuc::pathname path("/foo");

    BOOST_CHECK(!path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root2) {
    nuc::pathname path("/");

    BOOST_CHECK(path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root3) {
    nuc::pathname path("foo");

    BOOST_CHECK(!path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root4) {
    nuc::pathname path("");

    BOOST_CHECK(!path.is_root());
}

BOOST_AUTO_TEST_CASE(is_relative1) {
    nuc::pathname path("foo/bar");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative2) {
    nuc::pathname path("foo");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative3) {
    nuc::pathname path("");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative4) {
    nuc::pathname path("/foo/bar");

    BOOST_CHECK(!path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative5) {
    nuc::pathname path("/");

    BOOST_CHECK(!path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative6) {
    nuc::pathname path("~user");

    BOOST_CHECK(!path.is_relative());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_is_subpath)

BOOST_AUTO_TEST_CASE(is_subpath1) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar/baz/file.txt");

    BOOST_CHECK(child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath2) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/baz/file.txt");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath3) {
    nuc::pathname parent("/foo/bar/");
    nuc::pathname child("/foo/bar");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath4) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath5) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar.txt");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath6) {
    nuc::pathname parent("");
    nuc::pathname child("foo");

    BOOST_CHECK(child.is_subpath(parent));
}

BOOST_AUTO_TEST_CASE(is_child_of1) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar/file.txt");

    BOOST_CHECK(child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of2) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar/baz/file.txt");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of3) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/baz/file.txt");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of4) {
    nuc::pathname parent("/foo/bar/");
    nuc::pathname child("/foo/bar");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of5) {
    nuc::pathname parent("/foo/bar");
    nuc::pathname child("/foo/bar");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of6) {
    nuc::pathname parent("");
    nuc::pathname child("foo");

    BOOST_CHECK(child.is_child_of(parent));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_has_dirs)

BOOST_AUTO_TEST_CASE(has_dirs1) {
    nuc::pathname path("/foo/bar");

    BOOST_CHECK(path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs2) {
    nuc::pathname path("foo.txt");

    BOOST_CHECK(!path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs3) {
    nuc::pathname path("/");

    BOOST_CHECK(path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs4) {
    nuc::pathname path("");

    BOOST_CHECK(!path.has_dirs());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_subpath_offset)

BOOST_AUTO_TEST_CASE(subpath_offset) {
    std::set<nuc::pathname> paths;

    paths.insert("/foo/bar");
    paths.insert("/foo/baz/");
    paths.insert("/foo/dir/");

    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/bar"), 5);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/bar.txt"), nuc::pathname::string::npos);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/bar/file.txt"), nuc::pathname::string::npos);

    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/baz/"), 5);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/baz/file.txt"), 5);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/baz/bar/file.txt"), 5);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/baz.txt"), nuc::pathname::string::npos);

    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "something else"), nuc::pathname::string::npos);
    BOOST_CHECK_EQUAL(nuc::pathname::subpath_offset(paths, "/foo/zzz"), nuc::pathname::string::npos);
}

BOOST_AUTO_TEST_SUITE_END();
