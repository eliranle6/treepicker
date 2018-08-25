#include <stdio.h>
#include <assert.h>
#include "gtest/gtest.h"

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
*      │   └── subdir_in_second
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
    ASSERT_EQ(dir.path, "/some/path");
}

TEST(dirtree, rootnode_properties) {
    dirtree::DirTree dir = create_example_tree();
    ASSERT_EQ(dir.get_root()->children.size(), 3);
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
        ASSERT_EQ(dir.get_root()->children[1]->children[0]->tag, "subdir_in_second");
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
                  "seconddir/subdir_in_second");
    ASSERT_EQ(dir.get_root()->children[2]->identifier,
              "thirddir");
        ASSERT_EQ(dir.get_root()->children[2]->children[0]->identifier,
                  "thirddir/subdir_in_third");
            ASSERT_EQ(dir.get_root()->children[2]->children[0]->children[0]->identifier,
                      "thirddir/subdir_in_third/file_in_subdir.txt");
        ASSERT_EQ(dir.get_root()->children[2]->children[1]->identifier,
                  "thirddir/yetanotherfile.txt");
}
