#include <assert.h>
#include <boost/algorithm/string/replace.hpp>

#include <treelib.h>


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


void test_print_basic_tree() {
    treelib::Tree<int> tree;
    tree.create_root_node("People", "people", 3);
    tree.create_node("Dumb people", "dumb", "people", 3);
    tree.create_node("Rich dumb people", "rich-dumb", "dumb", 6);
    tree.create_node("Poor dumb people", "poor-dumb", "dumb", 8);
    tree.create_node("Smart people", "smart", "people", 11);
    tree.create_node("Rich smart people", "rich-smart", "smart", 12);
    tree.create_node("Poor smart people", "poor-smart", "smart", 13);

    std::string actual = "";
    std::string expected = convert_ascii_tree_to_unicode("People\n"
                                                         "+-- Dumb people\n"
                                                         "|  +-- Rich dumb people\n"
                                                         "|  \\-- Poor dumb people\n"
                                                         "\\-- Smart people\n"
                                                         "   +-- Rich smart people\n"
                                                         "   \\-- Poor smart people\n"
                                                        );
    tree.print();
    std::cout << expected << std::endl;
    // assert(actual == expected);
}


int main(void) {
    test_print_basic_tree();
    std::cout << "great success" << std::endl;
}
