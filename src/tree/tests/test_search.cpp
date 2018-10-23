#include "gtest/gtest.h"
#include "tree/tree.h"


treelib::Tree<int> get_example_tree(void) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
        tree.create_node("child1_tag", "child1_id", "root_id", 20);
            tree.create_node("child1.1_tag", "child1.1_id", "child1_id", 20);
            tree.create_node("child1.2_tag", "child1.2_id", "child1_id", 20);
        tree.create_node("child2_tag", "child2_id", "root_id", 20);
            tree.create_node("child2.1_tag", "child2.1_id", "child2_id", 20);
            tree.create_node("child2.2_tag", "child2.2_id", "child2_id", 20);
    return tree;
}

TEST(treesearch, simple_search_matching_nodes) {
    treelib::Tree<int> tree = get_example_tree();
    tree.search("child1");

    ASSERT_FALSE(tree.get_node("root_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_matching_search);
}

TEST(treesearch, simple_search_ancestors_of_matching_nodes) {
    treelib::Tree<int> tree = get_example_tree();
    tree.search("child1");
    ASSERT_TRUE(tree.get_node("root_id")->is_ancestor_of_matching_search);
    ASSERT_TRUE(tree.get_node("child1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_ancestor_of_matching_search);
}
