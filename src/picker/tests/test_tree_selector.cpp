#include "gtest/gtest.h"

#include "picker/tree_selector.h"
#include "tree/tests/common.h"


TEST(tree_selector, default_selection) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

TEST(tree_selector, default_selection_in_empty_tree) {
    treelib::Tree tree;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, explore_selection) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, explore_selection_in_empty_tree) {
    treelib::Tree tree;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, move_one_next) {
    // Setup
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
    tree_printer.preprocess(out, true, tree.get_root(), 100);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");

    // Run
    tree_printer.preprocess(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();

    // Validate
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-dumb");
}

TEST(tree_selector, move_one_prev) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_printer.preprocess(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();
    tree_printer.preprocess(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_prev();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, move_one_prev__does_not_pass_bounds) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_to_prev();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, move_one_up) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
}

TEST(tree_selector, move_one_up_does_not_pass_bounds) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_one_up();
    tree_selector.move_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
}

TEST(tree_selector, explore_selection_skips_non_maching_children) {
    auto tree = get_simple_tree();
    tree.get_node("dumb")->is_matching_search = false;
    tree.get_node("dumb")->is_ancestor_of_matching_search = false;
        tree.get_node("rich-dumb")->is_matching_search = false;
        tree.get_node("rich-dumb")->is_ancestor_of_matching_search = false;
        tree.get_node("poor-dumb")->is_matching_search = false;
        tree.get_node("poor-dumb")->is_ancestor_of_matching_search = false;

    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}

TEST(tree_selector, next_skips_non_matching_children) {
    // Setup
    auto tree = get_simple_tree();
    // Leave only root, dumb and empty
    tree.get_node("rich-dumb")->is_ancestor_of_matching_search = false;
    tree.get_node("poor-dumb")->is_ancestor_of_matching_search = false;
    tree.get_node("rich-dumb")->is_matching_search = false;
    tree.get_node("poor-dumb")->is_matching_search = false;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    // Go to first subnode of root, 'dumb'
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");

    // Move next, skipping subnodes of dumb
    std::ostringstream out;
    tree_printer.preprocess(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();

    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}
