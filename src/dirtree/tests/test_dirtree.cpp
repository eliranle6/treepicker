#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <assert.h>
#include "gtest/gtest.h"

#include <boost/filesystem/operations.hpp>

#include "dirtree/dirtree.h"
#include "utils/getexepath.h"

/*
* The following dir structure is present in the tests dir:
*
*      subdir_for_tests
*      ├── firstdir
*      │   ├── anotherfile.txt
*      │   └── somefile.txt
*      ├── seconddir
*      │   └── file_in_second
*      └── thirddir
*          ├── subdir_in_third
*          │   └── file_in_subdir.txt
*          └── yetanotherfile.txt
*/


dirtree::DirTree create_example_tree() {
    boost::filesystem::path tests_subdir =
        boost::filesystem::path(getexepath()).parent_path().string()
        + "/../../../../src/dirtree/tests/subdir_for_tests";
    dirtree::DirTree dir(tests_subdir.string());
    dir.update_from_filesystem();
    return dir;
}

TEST(dirtree, dirpath) {
    dirtree::DirTree dir("/some/path");
    ASSERT_EQ(dir.path.string(), "/some/path");
}

TEST(dirtree, rootnode_properties) {
    dirtree::DirTree dir = create_example_tree();
    ASSERT_EQ(dir.get_root()->tag, "subdir_for_tests");
    ASSERT_EQ(dir.get_root()->identifier, "");
}

TEST(dirtree, read_flat_dir__nr_children_of_root) {
    dirtree::DirTree dir = create_example_tree();
    ASSERT_EQ(dir.get_root()->children.size(), 3);
}

TEST(dirtree, read_flat_dir__tree_node_tags) {
    dirtree::DirTree dir = create_example_tree();
    ASSERT_EQ(dir.get_root()->children[0]->tag, "firstdir");
        ASSERT_EQ(dir.get_root()->children[0]->children[0]->tag, "anotherfile.txt");
        ASSERT_EQ(dir.get_root()->children[0]->children[1]->tag, "somefile.txt");
    ASSERT_EQ(dir.get_root()->children[1]->tag, "seconddir");
        ASSERT_EQ(dir.get_root()->children[1]->children[0]->tag, "file_in_second");
    ASSERT_EQ(dir.get_root()->children[2]->tag, "thirddir");
        ASSERT_EQ(dir.get_root()->children[2]->children[0]->tag, "subdir_in_third");
            ASSERT_EQ(dir.get_root()->children[2]->children[0]->children[0]->tag,
                      "file_in_subdir.txt");
        ASSERT_EQ(dir.get_root()->children[2]->children[1]->tag, "yetanotherfile.txt");
}

TEST(dirtree, read_flat_dir__tree_node_identifiers) {
    dirtree::DirTree dir = create_example_tree();
    ASSERT_EQ(dir.get_root()->children[0]->identifier,
              "firstdir");
        ASSERT_EQ(dir.get_root()->children[0]->children[0]->identifier,
                  "firstdir/anotherfile.txt");
        ASSERT_EQ(dir.get_root()->children[0]->children[1]->identifier,
                  "firstdir/somefile.txt");
    ASSERT_EQ(dir.get_root()->children[1]->identifier,
              "seconddir");
        ASSERT_EQ(dir.get_root()->children[1]->children[0]->identifier,
                  "seconddir/file_in_second");
    ASSERT_EQ(dir.get_root()->children[2]->identifier,
              "thirddir");
        ASSERT_EQ(dir.get_root()->children[2]->children[0]->identifier,
                  "thirddir/subdir_in_third");
            ASSERT_EQ(dir.get_root()->children[2]->children[0]->children[0]->identifier,
                      "thirddir/subdir_in_third/file_in_subdir.txt");
        ASSERT_EQ(dir.get_root()->children[2]->children[1]->identifier,
                  "thirddir/yetanotherfile.txt");
}

// A temporary dir, removed (with everything in it) when the object is destroyed
class TempDir {
public:
    TempDir(void) {
        char path_template[] = "/tmp/test_dirtree_XXXXXX";
        path = mkdtemp(path_template);
    }
    ~TempDir(void) {
        boost::filesystem::remove_all(path);
    }
    void make_dir(const std::string& relative_path) {
        boost::filesystem::create_directories(path + "/" + relative_path);
    }
    void make_file(const std::string& relative_path) {
        std::ofstream(path + "/" + relative_path);
    }
    std::string path;
};

static std::vector<std::string> tags_of_children(dirtree::DirTree& dir, const std::string& id) {
    std::vector<std::string> tags;
    for (auto& child : dir.get_node(id)->children) {
        tags.push_back(child->tag);
    }
    return tags;
}

TEST(dirtree, nonexistent_dir_throws) {
    dirtree::DirTree dir("/no/such/dir/for/test_dirtree");
    ASSERT_THROW(dir.update_from_filesystem(), std::runtime_error);
}

TEST(dirtree, children_are_sorted) {
    TempDir temp;
    for (auto name : {"b", "c", "a", "B", "a-b"}) {
        temp.make_file(name);
    }
    temp.make_dir("a.d");
    temp.make_file("a.d/z");
    temp.make_file("a.d/y");
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();

    std::vector<std::string> expected = {"B", "a", "a-b", "a.d", "b", "c"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
    expected = {"y", "z"};
    ASSERT_EQ(tags_of_children(dir, "a.d"), expected);
}

TEST(dirtree, unusual_file_names) {
    TempDir temp;
    temp.make_dir("dir with spaces");
    temp.make_file("dir with spaces/100% %s %n.txt");
    temp.make_file("dir with spaces/line\nbreak");
    temp.make_file("dir with spaces/\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d");
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();

    std::vector<std::string> expected = {"100% %s %n.txt", "line\nbreak",
                                         "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d"};
    ASSERT_EQ(tags_of_children(dir, "dir with spaces"), expected);
    ASSERT_EQ(dir.get_nr_nodes(), 5);
}

TEST(dirtree, symlinks_are_skipped) {
    TempDir temp;
    temp.make_dir("real");
    temp.make_file("real/file");
    ASSERT_EQ(symlink((temp.path + "/real").c_str(), (temp.path + "/link").c_str()), 0);
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();

    std::vector<std::string> expected = {"real"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
}

TEST(dirtree, empty_dir) {
    TempDir temp;
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();
    ASSERT_EQ(dir.get_nr_nodes(), 1);
    ASSERT_EQ(dir.get_root()->children.size(), 0);
}

TEST(dirtree, big_flat_dir) {
    // More output than a pipe holds (64KB), and too many files to add with a linear scan each
    TempDir temp;
    for (int i = 0; i < 20000; ++i) {
        temp.make_file("file" + std::to_string(i));
    }
    auto start = std::chrono::steady_clock::now();
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_EQ(dir.get_root()->children.size(), 20000);
    ASSERT_LT(elapsed, std::chrono::seconds(5));
}
