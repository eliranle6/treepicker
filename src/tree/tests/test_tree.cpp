#include <assert.h>
#include "gtest/gtest.h"
#include "common.h"
#include "tree/tree.h"

TEST(tree, create_root) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "", reinterpret_cast<void*>(10));
    auto root_node = tree.get_node("root_id");
    assert(root_node->data == reinterpret_cast<void*>(10));
}

TEST(tree, get_node) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "", reinterpret_cast<void*>(10));
    tree.create_node("child_node_tag", "child_node_id", "root_id", reinterpret_cast<void*>(20));
    auto node = tree.get_node("child_node_id");
    assert(node->data == reinterpret_cast<void*>(20));
}

TEST(tree, root_value_persists_after_child_is_born) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "", reinterpret_cast<void*>(10));
    tree.create_node("child_node_tag", "child_node_id", "root_id", reinterpret_cast<void*>(20));
    auto node = tree.get_node("root_id");
    assert(node->data == reinterpret_cast<void*>(10));
}

TEST(tree, node_path) {
    auto tree = get_simple_tree();
    ASSERT_EQ(tree.get_node("rich-dumb")->path, std::string("/People/Dumb people/Rich dumb people"));
    ASSERT_EQ(tree.get_node("people")->path, std::string("/People"));
}

TEST(tree, parent_of_root_is_empty_string) {
    auto tree = get_simple_tree();
    assert(tree.get_root()->parent == std::string(""));
}
