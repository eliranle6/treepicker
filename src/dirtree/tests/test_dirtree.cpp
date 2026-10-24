#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include "gtest/gtest.h"

#include <filesystem>

#include "dirtree/dirtree.h"

// A temporary dir, removed (with everything in it) when the object is destroyed
class TempDir {
public:
    TempDir(void) {
        char path_template[] = "/tmp/test_dirtree_XXXXXX";
        path = mkdtemp(path_template);
    }
    ~TempDir(void) {
        std::filesystem::remove_all(path);
    }
    void make_dir(const std::string& relative_path) {
        std::filesystem::create_directories(path + "/" + relative_path);
    }
    void make_file(const std::string& relative_path) {
        std::ofstream(path + "/" + relative_path);
    }
    std::string path;
};

/*
* The example dir, made in a temporary dir for each test (rather than kept in the tests dir, which
* tests found by a path relative to their binary):
*
*      subdir_for_tests
*      ├── firstdir
*      │   ├── anotherfile.txt
*      │   ├── link_to_file_in_third -> ../thirddir/yetanotherfile.txt
*      │   └── somefile.txt
*      ├── seconddir
*      │   └── file_in_second
*      └── thirddir
*          ├── subdir_in_third
*          │   └── file_in_subdir.txt
*          └── yetanotherfile.txt
*/
dirtree::DirTree create_example_tree() {
    TempDir temp;
    temp.make_dir("subdir_for_tests/firstdir");
    temp.make_file("subdir_for_tests/firstdir/anotherfile.txt");
    temp.make_file("subdir_for_tests/firstdir/somefile.txt");
    temp.make_dir("subdir_for_tests/seconddir");
    temp.make_file("subdir_for_tests/seconddir/file_in_second");
    temp.make_dir("subdir_for_tests/thirddir/subdir_in_third");
    temp.make_file("subdir_for_tests/thirddir/subdir_in_third/file_in_subdir.txt");
    temp.make_file("subdir_for_tests/thirddir/yetanotherfile.txt");
    symlink("../thirddir/yetanotherfile.txt",
            (temp.path + "/subdir_for_tests/firstdir/link_to_file_in_third").c_str());

    // The tree is read into memory, so the dir can go once the function returns
    dirtree::DirTree dir(temp.path + "/subdir_for_tests");
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

static dirtree::DirTree tree_from_text(const std::string& text, char delimiter = '\n') {
    dirtree::DirTree dir("-");
    std::istringstream in(text);
    dir.update_from_stream(in, delimiter);
    return dir;
}

TEST(dirtree, stream_paths_keep_their_order) {
    auto dir = tree_from_text("b\nc\na\n");
    std::vector<std::string> expected = {"b", "c", "a"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
    ASSERT_EQ(dir.get_root()->tag, ".");
}

TEST(dirtree, stream_adds_missing_parents) {
    auto dir = tree_from_text("a/b/c.txt\n");
    ASSERT_EQ(dir.get_nr_nodes(), 4);
    ASSERT_EQ(dir.get_node("a")->tag, "a");
    ASSERT_EQ(dir.get_node("a/b")->parent, "a");
    ASSERT_EQ(dir.get_node("a/b/c.txt")->tag, "c.txt");
}

TEST(dirtree, stream_paths_are_cleaned) {
    auto dir = tree_from_text("./a//b/\n/c\n./\n\n.\nd/./e\n");
    std::vector<std::string> expected = {"a", "c", "d"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
    ASSERT_NE(dir.get_node("a/b"), nullptr);
    ASSERT_NE(dir.get_node("d/e"), nullptr);
    ASSERT_EQ(dir.get_nr_nodes(), 6);
}

TEST(dirtree, stream_skips_paths_already_added) {
    // Including a dir that was added for a path under it
    auto dir = tree_from_text("a/b\na/b\na\n");
    ASSERT_EQ(dir.get_nr_nodes(), 3);
    ASSERT_EQ(dir.get_node("a")->children.size(), 1);
}

TEST(dirtree, stream_null_separated) {
    auto dir = tree_from_text(std::string("line\nbreak/x\0y\0", 16), '\0');
    std::vector<std::string> expected = {"line\nbreak", "y"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
}

TEST(dirtree, stream_last_path_without_delimiter) {
    auto dir = tree_from_text("a\nb");
    std::vector<std::string> expected = {"a", "b"};
    ASSERT_EQ(tags_of_children(dir, ""), expected);
}

TEST(dirtree, empty_stream) {
    auto dir = tree_from_text("");
    ASSERT_EQ(dir.get_nr_nodes(), 1);
}

TEST(dirtree, max_depth) {
    TempDir temp;
    temp.make_dir("a/b/c");
    temp.make_file("a/b/c/deep.txt");
    temp.make_file("top.txt");
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem(2);

    ASSERT_NE(dir.get_node("a/b"), nullptr);
    ASSERT_EQ(dir.get_node("a/b/c"), nullptr);
    ASSERT_NE(dir.get_node("top.txt"), nullptr);
}

TEST(dirtree, scan_errors) {
    if (geteuid() == 0) {
        GTEST_SKIP() << "root can read anything";
    }
    TempDir temp;
    temp.make_dir("locked");
    temp.make_file("locked/secret.txt");
    temp.make_file("open.txt");
    chmod((temp.path + "/locked").c_str(), 0);

    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();
    chmod((temp.path + "/locked").c_str(), 0755);

    ASSERT_EQ(dir.scan_errors.size(), 1);
    ASSERT_NE(dir.scan_errors[0].find("Permission denied"), std::string::npos);
    ASSERT_NE(dir.get_node("open.txt"), nullptr);
}

TEST(dirtree, no_scan_errors) {
    TempDir temp;
    temp.make_file("a.txt");
    dirtree::DirTree dir(temp.path);
    dir.update_from_filesystem();
    ASSERT_TRUE(dir.scan_errors.empty());
}
