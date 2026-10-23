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

TEST(treesearch, search_matches_names_not_paths) {
    treelib::Tree tree = get_example_tree();
    treelib::Search search(tree);

    // Under child1_tag, but not named so
    search.search("child1_tag/");
    ASSERT_EQ(search.get_nr_matching(), 0);

    // The root's name is part of every path
    search.search("root");
    ASSERT_EQ(search.get_nr_matching(), 1);
    ASSERT_TRUE(tree.get_node("root_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child1_id")->is_matching_search);
}

TEST(treesearch, lowercase_keyword_ignores_case) {
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    tree.create_node("README.md", "readme", "root", nullptr);
    tree.create_node("notes.txt", "notes", "root", nullptr);
    treelib::Search(tree).search("readme");
    ASSERT_TRUE(tree.get_node("readme")->is_matching_search);
    ASSERT_FALSE(tree.get_node("notes")->is_matching_search);
}

TEST(treesearch, keyword_with_uppercase_is_case_sensitive) {
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    tree.create_node("README.md", "upper", "root", nullptr);
    tree.create_node("readme.md", "lower", "root", nullptr);
    treelib::Search(tree).search("README");
    ASSERT_TRUE(tree.get_node("upper")->is_matching_search);
    ASSERT_FALSE(tree.get_node("lower")->is_matching_search);
}

TEST(treesearch, empty_keyword_matches_everything) {
    treelib::Tree tree = get_example_tree();
    treelib::Search search(tree);
    search.search("child1");
    search.search("");
    ASSERT_EQ(search.get_nr_matching(), 7);
    for (auto id : {"root_id", "child1_id", "child1.1_id", "child2_id", "child2.2_id"}) {
        ASSERT_TRUE(tree.get_node(id)->is_matching_search) << id;
    }
}

TEST(treesearch, nr_matching) {
    treelib::Tree tree = get_example_tree();
    treelib::Search search(tree);
    search.search("child2");
    // child2_tag, child2.1_tag and child2.2_tag
    ASSERT_EQ(search.get_nr_matching(), 3);
    search.search("no such node");
    ASSERT_EQ(search.get_nr_matching(), 0);
}

TEST(treesearch, new_search_replaces_the_previous_one) {
    treelib::Tree tree = get_example_tree();
    treelib::Search search(tree);
    search.search("child1.1");
    search.search("child2.2");
    ASSERT_FALSE(tree.get_node("child1.1_id")->is_matching_search);
    ASSERT_FALSE(tree.get_node("child1_id")->is_ancestor_of_matching_search);
    ASSERT_TRUE(tree.get_node("child2.2_id")->is_matching_search);
    ASSERT_TRUE(tree.get_node("child2_id")->is_ancestor_of_matching_search);
    ASSERT_TRUE(tree.get_node("root_id")->is_ancestor_of_matching_search);
}

TEST(treesearch, is_search_visible) {
    treelib::Tree tree = get_example_tree();
    treelib::Search(tree).search("child1.2");
    ASSERT_TRUE(tree.get_node("root_id")->is_search_visible());
    ASSERT_TRUE(tree.get_node("child1_id")->is_search_visible());
    ASSERT_TRUE(tree.get_node("child1.2_id")->is_search_visible());
    ASSERT_FALSE(tree.get_node("child1.1_id")->is_search_visible());
    ASSERT_FALSE(tree.get_node("child2_id")->is_search_visible());
}

TEST(treesearch, search_tree_without_nodes) {
    treelib::Tree tree;
    treelib::Search search(tree);
    search.search("anything");
    ASSERT_EQ(search.get_nr_matching(), 0);
}
