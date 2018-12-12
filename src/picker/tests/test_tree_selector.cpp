#include "gtest/gtest.h"

#include "picker/tree_selector.h"
#include "tree/tests/common.h"


TEST(tree_selector, default_selection) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

TEST(tree_selector, default_selection_in_empty_tree) {
    treelib::Tree<char> tree;
    picker::TreeSelector tree_selector(tree);
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, explore_selection) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, explore_selection_in_empty_tree) {
    treelib::Tree<char> tree;
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, move_selection_one_down) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    tree_selector.move_selection_one_down();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}

TEST(tree_selector, move_selection_one_down__does_not_pass_bounds) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    tree_selector.move_selection_one_down();
    tree_selector.move_selection_one_down();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}

TEST(tree_selector, move_selection_one_up) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    tree_selector.move_selection_one_down();
    tree_selector.move_selection_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, move_selection_one_up__does_not_pass_bounds) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    tree_selector.explore_children_of_selection();
    tree_selector.move_selection_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}