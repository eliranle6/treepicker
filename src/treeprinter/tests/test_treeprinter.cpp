#include <assert.h>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <gtest/gtest.h>

#include "tree/tree.h"
#include "treeprinter/treeprinter.h"
#include "common.h"

# if 0
static std::string convert_ascii_tree_to_unicode(std::string tree) {
    boost::replace_all(tree, "|", VERTICAL_TREE_LINE);
    boost::replace_all(tree, "+", MIDDLE_CHILD_CONNECTOR);
    boost::replace_all(tree, "-", HORIZONTAL_TREE_LINE);
    boost::replace_all(tree, "\\", LAST_CHILD_CONNECTOR);
    return tree;
}
#endif

TEST(treeprinter, print_basic_tree) {
    // // Store original cout buffer before mocking it
    std::ostringstream out;

    std::string expected = ("> People\n"
                            "  +--Dumb people\n"
                            "  |  +--Poor dumb people\n"
                            "  |  +--Rich dumb people\n"
                            "  +--Smart people\n"
                            "     +--Poor smart people\n"
                            "     +--Rich smart people\n"
                            );
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

    std::string expected = ("  People\n"
                            "  +--Dumb people\n"
                            "  |  +--Poor dumb people\n"
                            "> |  +--Rich dumb people\n"
                            "  +--Smart people\n"
                            "     +--Poor smart people\n"
                            "     +--Rich smart people\n"
                            );
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    treelib::TreePrinter<int> tree_printer;
    tree_printer.print(out, tree, true, tree.get_node("rich-dumb"));

    // Test results
    std::string actual = out.str();
    ASSERT_EQ(actual, expected);
}
