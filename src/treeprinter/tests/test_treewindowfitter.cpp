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

template<typename T>
bool are_two_trees_equal(treelib::Tree<T> lhs, treelib::Tree<T> rhs) {
    typedef typename treelib::Tree<T>::node_t Node;
    typedef typename std::pair<Node, Node> NodePair;

    std::stack<NodePair> dfs_stack;

    dfs_stack.push(std::make_pair(lhs.get_root(), rhs.get_root()));
    while (not dfs_stack.empty()) {
        // Get current pair
        auto pair = dfs_stack.top();
        dfs_stack.pop();
        auto lhs_node = std::get<0>(pair);
        auto rhs_node = std::get<1>(pair);

        if (rhs_node != lhs_node) {
            return false;
        }

        // Compare number of direct child nodes
        auto &lhs_children = lhs_node->children;
        auto &rhs_children = rhs_node->children;
        if (lhs_children.size() != rhs_children.size()) {
            return false;
        }
        // Add children to stack
        for (size_t node_idx = 0; node_idx < lhs_children.size(); ++node_idx) {
            auto child_pair = std::make_pair(lhs_children[node_idx], rhs_children[node_idx]);
            dfs_stack.push(child_pair);
        }
    }

    return true;
}

template<typename T>
int get_tree_print_size(treelib::Tree<T> tree) {
    std::ostringstream out;
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, false, tree.get_root());
    std::string tree_str = out.str();
    return std::count(tree_str.begin(), tree_str.end(), '\n');
}

TEST(treewindowfitter, fitting_tree_smaller_than_window_returns_same_tree) {
    const int window_size = 10;
    auto tree = get_simple_tree();
    // Validate the test logic, not testing the unit test itself
    assert(window_size > get_tree_print_size(tree));

    auto selected = tree.get_node("dumb");
    auto actual = fit_tree_to_window(tree, window_size, selected);
    auto expected = tree;

    ASSERT_TRUE(are_two_trees_equal(actual, tree));
}

TEST(treewindowfitter, fitting_tree_the_same_size_as_window_returns_same_tree) {
    const int window_size = 7;
    auto tree = get_simple_tree();
    // Validate the test logic, not testing the unit test itself
    ASSERT_EQ(window_size, get_tree_print_size(tree));

    auto selected = tree.get_node("dumb");
    auto actual = fit_tree_to_window(tree, window_size, selected);
    auto expected = tree;

    ASSERT_TRUE(are_two_trees_equal(actual, tree));
}

TEST(treewindowfitter, parent_of_selected_when_not_root_is_root_of_printed_tree) {
    auto tree = get_simple_tree();
    auto selected = tree.get_node("rich-dumb");
    auto window_size = 3;
    auto fitted_tree = fit_tree_to_window(tree, window_size, selected);
    auto actual = fitted_tree.get_root();
    auto expected = tree.get_node("dumb");
    ASSERT_EQ(actual, expected);
}
