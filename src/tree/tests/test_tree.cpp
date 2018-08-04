#include <assert.h>
#include <iostream>
#include "gtest/gtest.h"
#include "tree/tree.h"

TEST(tree, create_root) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
    auto root_node = tree.get_node("root_id");
    assert(root_node->data == 10);
}

TEST(tree, get_node) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
    tree.create_node("child_node_tag", "child_node_id", "root_id", 20);
    auto node = tree.get_node("child_node_id");
    assert(node->data == 20);
}

TEST(tree, root_value_persists_after_child_is_born) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
    tree.create_node("child_node_tag", "child_node_id", "root_id", 20);
    auto node = tree.get_node("root_id");
    assert(node->data == 10);
}
