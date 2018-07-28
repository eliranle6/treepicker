#include <assert.h>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>

#include "treelib.h"


// Not writing non-ASCII chars in code
//
// --
//
#define HORIZONTAL_TREE_LINE ("\xe2\x94\x80")
// |
// |
// |
#define VERTICAL_TREE_LINE ("\xe2\x94\x82")
// |
// |--
// |
#define MIDDLE_CHILD_CONNECTOR ("\xe2\x94\x9c")
// |
// |--
//
#define LAST_CHILD_CONNECTOR  ("\xe2\x94\x94")

static std::string convert_ascii_tree_to_unicode(std::string tree) {
    boost::replace_all(tree, "|", VERTICAL_TREE_LINE);
    boost::replace_all(tree, "+", MIDDLE_CHILD_CONNECTOR);
    boost::replace_all(tree, "-", HORIZONTAL_TREE_LINE);
    boost::replace_all(tree, "\\", LAST_CHILD_CONNECTOR);
    return tree;
}

treelib::Tree<int> get_simple_tree(void) {
    treelib::Tree<int> tree;
    tree.create_root_node("People", "people", 3);
    tree.create_node("Dumb people", "dumb", "people", 3);
    tree.create_node("Rich dumb people", "rich-dumb", "dumb", 6);
    tree.create_node("Poor dumb people", "poor-dumb", "dumb", 8);
    tree.create_node("Smart people", "smart", "people", 11);
    tree.create_node("Rich smart people", "rich-smart", "smart", 12);
    tree.create_node("Poor smart people", "poor-smart", "smart", 13);
    return tree;
}


void test_print_basic_tree() {
    // Store original cout buffer before mocking it
    auto *original_out_buffer = std::cout.rdbuf();
    // Mock the cout buffer
    std::ostringstream out;
    std::cout.rdbuf(out.rdbuf());

    std::string expected = convert_ascii_tree_to_unicode("People\n"
                                                         "+--Dumb people\n"
                                                         "|  +--Rich dumb people\n"
                                                         "|  \\--Poor dumb people\n"
                                                         "\\--Smart people\n"
                                                         "   +--Rich smart people\n"
                                                         "   \\--Poor smart people\n"
                                                        );
    // Print tree
    treelib::Tree<int> tree = get_simple_tree();
    out << tree;

    // Restore stdout stream buffer
    std::cout.rdbuf(original_out_buffer);

    // Test results
    std::string actual(out.str());
    assert(actual == expected);

}


int main(void) {
    test_print_basic_tree();
    std::cout << "great success" << std::endl;
}
