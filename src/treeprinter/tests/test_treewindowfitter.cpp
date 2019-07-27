#include <assert.h>
#include <utility>
#include "gtest/gtest.h"
#include "tree/tests/common.h"
#include "tree/tree.h"
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
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 3;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_the_same_size_as_window_returns_same_tree) {
    const int window_size = 7;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 3;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_bigger_than_window_but_smaller_without_leaves) {
    const int window_size = 4;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_one_line_bigger_than_window) {
    const int window_size = 6;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_tree_in_window_the_size_of_tree_without_leaves) {
    const int window_size = 3;
    auto tree = get_simple_tree();

    auto selected = tree.get_node("people");
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_subtree_with_one_node_in_large_enough_window) {
    const int window_size = 10;
    auto tree = get_tree_with_countries();

    auto selected = tree.get_node("jewish_majority");
    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, selected);
    auto expected = 2;

    ASSERT_EQ(actual, expected);
}

TEST(treewindowfitter, fitting_nodes_that_match_search_may_change_nr_levels_to_fit) {
    const int window_size = 4;
    auto tree = get_simple_tree();

    TreeWindowFitter<int> fitter;
    auto actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, tree.get_root());
    auto expected = 2;
    ASSERT_EQ(actual, expected);

    tree.search(std::string("Dumb"));

    actual = fitter.get_nr_levels_that_fit_in_window(tree, window_size, tree.get_root());
    expected = 3;
    ASSERT_EQ(actual, expected);
}
