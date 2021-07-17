#include "gtest/gtest.h"
#include "tree/tree.h"
#include "tree/search.h"


static auto TEN = reinterpret_cast<void*>(10); // Not allowed with constexpr
static auto TWENTY = reinterpret_cast<void*>(20); // Not allowed with constexpr

treelib::Tree get_example_tree(void) {
    treelib::Tree tree;
    tree.create_node("root_tag", "root_id", "", TEN);
        tree.create_node("child1_tag", "child1_id", "root_id", TWENTY);
            tree.create_node("child1.1_tag", "child1.1_id", "child1_id", TWENTY);
            tree.create_node("child1.2_tag", "child1.2_id", "child1_id", TWENTY);
        tree.create_node("child2_tag", "child2_id", "root_id", TWENTY);
            tree.create_node("child2.1_tag", "child2.1_id", "child2_id", TWENTY);
            tree.create_node("child2.2_tag", "child2.2_id", "child2_id", TWENTY);
    return tree;
}

TEST(treesearch, simple_search_matching_nodes) {
    treelib::Tree tree = get_example_tree();
    treelib::Search(tree).search("child1");

    ASSERT_FALSE(tree.get_node("root_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_matching_search);
}

TEST(treesearch, simple_search_ancestors_of_matching_nodes) {
    treelib::Tree tree = get_example_tree();
    treelib::Search(tree).search("child1");
    ASSERT_TRUE(tree.get_node("root_id")->is_ancestor_of_matching_search);
    ASSERT_TRUE(tree.get_node("child1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_ancestor_of_matching_search);
}

TEST(treesearch, search_by_path) {
    treelib::Tree tree = get_example_tree();
    treelib::Search(tree).search("child1_tag/");
    ASSERT_FALSE(tree.get_node("root_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.1_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child1.2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_matching_search);

    ASSERT_TRUE(tree.get_node("root_id")->is_ancestor_of_matching_search);
    ASSERT_TRUE(tree.get_node("child1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child1.2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.1_id")->is_ancestor_of_matching_search);
    ASSERT_FALSE(tree.get_node("child2.2_id")->is_ancestor_of_matching_search);
}
