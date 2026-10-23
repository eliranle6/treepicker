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

TEST(tree, children_are_sorted_by_tag_whatever_the_creation_order) {
    std::vector<std::string> tags;
    for (int i = 0; i < 500; ++i) {
        tags.push_back("tag" + std::to_string(i * 7919 % 500));  // Shuffled
    }
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    for (auto& tag : tags) {
        tree.create_node(tag, tag, "root", nullptr);
    }

    auto& children = tree.get_root()->children;
    ASSERT_EQ(children.size(), 500);
    for (std::size_t i = 1; i < children.size(); ++i) {
        ASSERT_LE(children[i - 1]->tag, children[i]->tag);
    }
}

TEST(tree, building_a_big_flat_tree_in_sorted_order_is_fast) {
    // Children used to be inserted with a linear scan: 100k files in one dir took a minute
    auto start = std::chrono::steady_clock::now();
    auto tree = get_flat_tree(200000);
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
