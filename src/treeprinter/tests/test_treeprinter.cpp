#include <assert.h>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <gtest/gtest.h>

#include "tree/tree.h"
#include "tree/tests/common.h"
#include "treeprinter/treeprinter.h"


TEST(treeprinter, print_basic_tree) {
    // // Store original cout buffer before mocking it
    std::ostringstream out;

    std::string expected = "> People\n"
                           "  +-- Dumb people\n"
                           "  |   +-- Poor dumb people\n"
                           "  |   +-- Rich dumb people\n"
                           "  +-- Smart people\n"
                           "      +-- Poor smart people\n"
                           "      +-- Rich smart people\n"
                           ;
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);
    tree_printer.print(out, true);

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}

TEST(treeprinter, test_different_selection) {
    // // Store original cout buffer before mocking it
    std::ostringstream out;

    std::string expected = "  People\n"
                           "> +-- Dumb people\n"
                           "  |   +-- Poor dumb people\n"
                           "  |   +-- Rich dumb people\n"
                           "  +-- Smart people\n"
                           "      +-- Poor smart people\n"
                           "      +-- Rich smart people\n"
                           ;
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);
    tree_printer.print(out, true, tree.get_node("dumb"));

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}

TEST(treeprinter, print_tree_in_window_smaller_than_tree_omits_levels) {
    // Store original cout buffer before mocking it
    std::ostringstream out;

    constexpr static int window_height = 6;
    std::string expected = "> People\n"
                           "  +-- Dumb people\n"
                           "  +-- Smart people\n"
                           ;
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);
    tree_printer.print(out, true, tree.get_root(), window_height);

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}

TEST(treeprinter, print_tree_in_window_the_size_of_tree_with_leaves_omitted_omits_trees) {
    // Store original cout buffer before mocking it
    std::ostringstream out;

    constexpr static int window_height = 3;
    std::string expected = "> People\n"
                           "  +-- Dumb people\n"
                           "  +-- Smart people\n"
                           ;
    // Print tree
    treelib::Tree<int> tree;
    tree.create_node("People", "people", "", 3);
        tree.create_node("Dumb people", "dumb", "people", 3);
        tree.create_node("Smart people", "smart", "people", 11);
    treelib::TreePrinter<int> tree_printer(tree);
    tree_printer.print(out, true, tree.get_root(), window_height);

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}

TEST(treeprinter, get_next_printed_node) {
    treelib::Tree<int> tree = get_simple_tree();
    std::ostringstream out;
    treelib::TreePrinter<int> tree_printer(tree);
    constexpr static int window_height = 6;
    tree_printer.print(out, true, tree.get_root(), window_height);

    auto expected = tree.get_node("dumb");
    auto actual = tree_printer.get_next_printed_node_after_selected();

    ASSERT_EQ(expected, actual);
}

TEST(treeprinter, get_previously_printed_node) {
    treelib::Tree<int> tree = get_simple_tree();
    std::ostringstream out;
    treelib::TreePrinter<int> tree_printer(tree);
    constexpr static int window_height = 6;
    tree_printer.print(out, true, tree.get_node("poor-smart"), window_height);

    auto actual = tree_printer.get_printed_node_before_selected();
    auto expected = tree.get_node("smart");
    std::cout << "actual: " << actual->tag << std::endl;
    std::cout << "expected: " << expected->tag << std::endl;
    ASSERT_EQ(expected, actual);
}
