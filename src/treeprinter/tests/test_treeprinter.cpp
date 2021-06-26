#include <assert.h>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <gtest/gtest.h>

#include "tree/tree.h"
#include "tree/tests/common.h"
#include "treeprinter/treeprinter.h"


static constexpr char SELECTED_ROW_FIRST_CHAR = '\x1B';


TEST(treeprinter, print_basic_tree) {
    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);

    // Run
    auto selection = tree.get_root();
    tree_printer.print(out, true, selection);

    // Validate
    auto tree_lines = split_string_by_newline(out.str());
    std::string actual = out.str();
    ASSERT_TRUE(does_string_contain_substring(tree_lines[0], "People"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[1], "Dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[2], "Poor dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[3], "Rich dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[4], "Smart"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[5], "Poor smart"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[6], "Rich smart"));
}

TEST(treeprinter, test_different_selection) {
    // Choose 'dumb' intead of root should move the '>' char to the beginning of dumb
    //  People
    //> +-- Dumb people
    //  |   +-- Poor dumb people
    //  |   +-- Rich dumb people
    //  +-- Smart people
    //      +-- Poor smart people
    //      +-- Rich smart people

    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);

    // Run
    // Select 'dumb' instead of root
    auto selection = tree.get_node("dumb");
    tree_printer.print(out, true, selection);

    // Validate
    std::string actual = out.str();
    auto tree_lines = split_string_by_newline(out.str());
    auto dumb_line = tree_lines[1];

    ASSERT_EQ(dumb_line[1], SELECTED_ROW_FIRST_CHAR);
    ASSERT_NE(dumb_line[0], SELECTED_ROW_FIRST_CHAR);
}

TEST(treeprinter, printing_tree_smaller_than_window_omits_higher_levels) {
    // Printed the following tree with window:
    // -> People
    // +-- Dumb people
    // |   +-- Poor dumb people
    // |   +-- Rich dumb people
    // +-- Smart people
    //     +-- Poor smart people
    //     +-- Rich smart people
    //
    // with a window size of 3 will yield only these rows:
    // -> People
    // +-- Dumb people
    // +-- Smart people

    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    constexpr static int window_height = 3;
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer(tree);

    // Run
    tree_printer.print(out, true, tree.get_root(), window_height);

    // Validate
    auto tree_lines = split_string_by_newline(out.str());

    ASSERT_TRUE(does_string_contain_substring(tree_lines[0], "People"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[1], "Dumb people"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[2], "Smart people"));
}

TEST(treeprinter, get_next_printed_node) {
    // -> People
    // +-- Dumb people
    // |   +-- Poor dumb people
    // |   +-- Rich dumb people
    // +-- Smart people
    //     +-- Poor smart people
    //     +-- Rich smart people
    //
    // Selected is People, so next printed node should be 'Dumb people'

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
    ASSERT_EQ(expected, actual);
}
