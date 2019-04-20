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
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, true, tree.get_root());

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
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, true, tree.get_node("dumb"));

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
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, true, tree.get_root(), window_height);

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}

TEST(treeprinter, print_tree_in_window_the_size_of_tree_with_leaves_omitted_omits_levels) {
    // Store original cout buffer before mocking it
    std::ostringstream out;

    constexpr static int window_height = 3;
    std::string expected = "> People\n"
                           "  +-- Dumb people\n"
                           "  +-- Smart people\n"
                           ;
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, true, tree.get_root(), window_height);

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}
