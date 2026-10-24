#include <assert.h>
#include <chrono>
#include <stdexcept>
#include "gtest/gtest.h"
#include "common.h"
#include "tree/tree.h"

TEST(tree, create_root) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "");
    auto root_node = tree.get_node("root_id");
    ASSERT_EQ(root_node, tree.get_root());
    ASSERT_EQ(root_node->tag, "root_tag");
}

TEST(tree, get_node) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "");
    tree.create_node("child_node_tag", "child_node_id", "root_id");
    auto node = tree.get_node("child_node_id");
    ASSERT_EQ(node->tag, "child_node_tag");
    ASSERT_EQ(node->parent, "root_id");
}

TEST(tree, root_persists_after_child_is_born) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "");
    tree.create_node("child_node_tag", "child_node_id", "root_id");
    ASSERT_EQ(tree.get_root()->identifier, "root_id");
    ASSERT_EQ(tree.get_root()->children.size(), 1);
}

TEST(tree, parent_of_root_is_empty_string) {
    auto tree = get_simple_tree();
    assert(tree.get_root()->parent == std::string(""));
}

TEST(tree, duplicate_node_throws) {
    auto tree = get_simple_tree();
    ASSERT_THROW(tree.create_node("Again", "dumb", "people"), std::invalid_argument);
}

TEST(tree, missing_parent_throws) {
    auto tree = get_simple_tree();
    ASSERT_THROW(tree.create_node("Orphan", "orphan", "no-such-parent"),
                 std::invalid_argument);
}

TEST(tree, children_keep_the_creation_order) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    for (auto tag : {"b", "c", "a"}) {
        tree.create_node(tag, tag, "root");
    }

    auto& children = tree.get_root()->children;
    ASSERT_EQ(children.size(), 3);
    ASSERT_EQ(children[0]->tag, "b");
    ASSERT_EQ(children[1]->tag, "c");
    ASSERT_EQ(children[2]->tag, "a");
}

TEST(tree, building_a_big_flat_tree_in_any_order_is_fast) {
    // Children were once kept sorted, which made adding them in random order quadratic: 200k
    // children took about 10 seconds
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 200000; ++i) {
        const std::string tag = std::to_string(i * 7919 % 200000);  // Shuffled
        tree.create_node(tag, tag, "root");
    }
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_EQ(tree.get_root()->children.size(), 200000);
    ASSERT_LT(elapsed, std::chrono::seconds(5));
}

TEST(tree, node_depth) {
    auto tree = get_simple_tree();
    ASSERT_EQ(tree.get_node_depth(tree.get_root()), 0);
    ASSERT_EQ(tree.get_node_depth(tree.get_node("smart")), 1);
    ASSERT_EQ(tree.get_node_depth(tree.get_node("poor-smart")), 2);
}

TEST(tree, node_depth_of_a_deep_tree) {
    // get_node_depth() once asserted that trees are at most 100 levels deep
    auto tree = get_chain_tree(300);
    ASSERT_EQ(tree.get_node_depth(tree.get_node("d300")), 300);
}

TEST(tree, nr_nodes) {
    ASSERT_EQ(get_simple_tree().get_nr_nodes(), 7);
    ASSERT_EQ(treelib::Tree().get_nr_nodes(), 0);
}

TEST(tree, unknown_node_is_null) {
    auto tree = get_simple_tree();
    ASSERT_EQ(tree.get_node("no-such-node"), nullptr);
    // And isn't added to the tree
    ASSERT_EQ(tree.get_nr_nodes(), 7);
}

TEST(tree, get_or_create_node) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    auto created = tree.get_or_create_node("a", "a", "root");
    ASSERT_NE(created, nullptr);
    // Already there: the same node, not a second one
    ASSERT_EQ(tree.get_or_create_node("a", "a", "root"), created);
    ASSERT_EQ(tree.get_root()->children.size(), 1);
    // No such parent: nothing is added
    ASSERT_EQ(tree.get_or_create_node("b", "b", "no-such-parent"), nullptr);
    ASSERT_EQ(tree.get_nr_nodes(), 2);
}
