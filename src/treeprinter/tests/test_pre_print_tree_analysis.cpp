#include "assert.h"
#include "gtest/gtest.h"

#include "tree/tree.h"
#include "treeprinter/pre_print_tree_analysis.h"
#include "tree/tests/common.h"

TEST(pre_print_tree_analysis, simple_tree_sibling_existence_map) {
    treelib::Tree<int> tree = get_simple_tree_informative_nodes();
    auto root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_node_to_next_sibling_existence["child1"]);
    assert(info.m_node_to_next_sibling_existence["child1.1"]);
    assert(not info.m_node_to_next_sibling_existence["child1.2"]);
    assert(not info.m_node_to_next_sibling_existence["root"]);
    assert(not info.m_node_to_next_sibling_existence["child2"]);
    assert(not info.m_node_to_next_sibling_existence["child2.1"]);
}

TEST(pre_print_tree_analysis, test_simple_tree_max_depth) {
    treelib::Tree<int> tree = get_simple_tree_informative_nodes();
    auto root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_max_depth == 2);
}
