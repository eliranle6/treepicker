#include <assert.h>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <gtest/gtest.h>

#include "tree/tree.h"

# if 0
static std::string convert_ascii_tree_to_unicode(std::string tree) {
    boost::replace_all(tree, "|", VERTICAL_TREE_LINE);
    boost::replace_all(tree, "+", MIDDLE_CHILD_CONNECTOR);
    boost::replace_all(tree, "-", HORIZONTAL_TREE_LINE);
    boost::replace_all(tree, "\\", LAST_CHILD_CONNECTOR);
    return tree;
}
#endif

treelib::Tree<int> get_simple_tree(void) {
    treelib::Tree<int> tree;
    tree.create_node("People", "people", "", 3);
    tree.create_node("Dumb people", "dumb", "people", 3);
    tree.create_node("Rich dumb people", "rich-dumb", "dumb", 6);
    tree.create_node("Poor dumb people", "poor-dumb", "dumb", 8);
    tree.create_node("Smart people", "smart", "people", 11);
    tree.create_node("Rich smart people", "rich-smart", "smart", 12);
    tree.create_node("Poor smart people", "poor-smart", "smart", 13);
    return tree;
}

TEST(treeprinter, print_basic_tree) {
    // // Store original cout buffer before mocking it
    std::ostringstream out;

    std::string expected = ("People\n"
                             "+--Dumb people\n"
                             "|  +--Poor dumb people\n"
                             "|  +--Rich dumb people\n"
                             "+--Smart people\n"
                             "   +--Poor smart people\n"
                             "   +--Rich smart people\n"
                            );
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    out << tree;

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}
