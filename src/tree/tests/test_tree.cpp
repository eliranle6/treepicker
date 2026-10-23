#include <assert.h>
#include <chrono>
#include <stdexcept>
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

TEST(tree, duplicate_node_throws) {
    auto tree = get_simple_tree();
    ASSERT_THROW(tree.create_node("Again", "dumb", "people", nullptr), std::invalid_argument);
}

TEST(tree, missing_parent_throws) {
    auto tree = get_simple_tree();
    ASSERT_THROW(tree.create_node("Orphan", "orphan", "no-such-parent", nullptr),
                 std::invalid_argument);
}

TEST(tree, children_keep_the_creation_order) {
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    for (auto tag : {"b", "c", "a"}) {
        tree.create_node(tag, tag, "root", nullptr);
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
    tree.create_node("root", "root", "", nullptr);
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 200000; ++i) {
        const std::string tag = std::to_string(i * 7919 % 200000);  // Shuffled
        tree.create_node(tag, tag, "root", nullptr);
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

TEST(tree, children) {
    auto tree = get_simple_tree();
    auto& children = tree.children("dumb");
    ASSERT_EQ(children.size(), 2);
    ASSERT_EQ(children[0]->identifier, "poor-dumb");
}

TEST(tree, children_of_unknown_node_throws) {
    auto tree = get_simple_tree();
    ASSERT_THROW(tree.children("no-such-node"), std::invalid_argument);
    // And doesn't add the id to the tree
    ASSERT_EQ(tree.get_node("no-such-node"), nullptr);
    ASSERT_EQ(tree.get_nr_nodes(), 7);
}

TEST(tree, get_or_create_node) {
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    auto created = tree.get_or_create_node("a", "a", "root", nullptr);
    ASSERT_NE(created, nullptr);
    // Already there: the same node, not a second one
    ASSERT_EQ(tree.get_or_create_node("a", "a", "root", nullptr), created);
    ASSERT_EQ(tree.get_root()->children.size(), 1);
    // No such parent: nothing is added
    ASSERT_EQ(tree.get_or_create_node("b", "b", "no-such-parent", nullptr), nullptr);
    ASSERT_EQ(tree.get_nr_nodes(), 2);
}
