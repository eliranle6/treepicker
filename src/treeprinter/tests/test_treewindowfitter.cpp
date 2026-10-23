#include <assert.h>
#include <utility>
#include "gtest/gtest.h"
#include "tree/tests/common.h"
#include "tree/tree.h"
#include "tree/search.h"
#include "treeprinter/treewindowfitter.h"
#include "tree/node.h"
#include "treeprinter/treeprinter.h"

/*
 * logic:
 *     root is parent of selected
 *     if selected level (node and siblings) requires pagination, paginate (no children)
 *     else:
 *         Traverse extra levels as long as they fit in the window
 */


TEST(treewindowfitter, fitting_tree_smaller_than_window_returns_same_tree) {
    const int window_size = 10;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 3;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_the_same_size_as_window_returns_same_tree) {
    const int window_size = 7;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 3;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_bigger_than_window_but_smaller_without_leaves) {
    const int window_size = 4;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_one_line_bigger_than_window) {
    const int window_size = 6;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_in_window_the_size_of_tree_without_leaves) {
    const int window_size = 3;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_subtree_with_one_node_in_large_enough_window) {
    const int window_size = 10;
    auto tree = get_tree_with_countries();

    auto selected = tree.get_node("jewish_majority");
    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_nodes_that_match_search_may_change_nr_levels_to_fit) {
    const int window_size = 4;
    auto tree = get_simple_tree();

    auto actual = get_nr_levels_that_fit_in_window(tree, window_size, tree.get_root());
    auto expected = 2;
    ASSERT_EQ(actual, expected);

    treelib::Search(tree).search(std::string("dumb"));

    actual = get_nr_levels_that_fit_in_window(tree, window_size, tree.get_root());
    expected = 3;
    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, level_that_doesnt_fit_is_left_out) {
    // The root and its 5 children are 6 lines, one too many for a window of 5
    auto tree = get_flat_tree(5);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 5, tree.get_root()), 1);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 6, tree.get_root()), 2);
}

TEST(treewindowfitter, chain_fits_one_level_per_line) {
    auto tree = get_chain_tree(50);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 10, tree.get_root()), 10);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 100, tree.get_root()), 51);
}

TEST(treewindowfitter, root_alone_always_fits) {
    auto tree = get_flat_tree(100);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 1, tree.get_root()), 1);
    ASSERT_EQ(get_nr_levels_that_fit_in_window(tree, 0, tree.get_root()), 1);
}

TEST(treewindowfitter, printed_root_moves_to_the_parent_of_a_selection_too_deep_to_print) {
    auto tree = get_chain_tree(50);
    auto selection = tree.get_node("d30");
    auto result = choose_printed_subtree_root(tree, selection, 30, 10, tree.get_root());
    ASSERT_EQ(std::get<0>(result), tree.get_node("d29"));
    ASSERT_EQ(std::get<1>(result), 29);
}

TEST(treewindowfitter, printed_root_moves_back_to_the_root_when_the_root_is_selected) {
    auto tree = get_chain_tree(50);
    auto result = choose_printed_subtree_root(tree, tree.get_root(), 0, 10, tree.get_node("d29"));
    ASSERT_EQ(std::get<0>(result), tree.get_root());
    ASSERT_EQ(std::get<1>(result), 0);
}
