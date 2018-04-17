#include "assert.h"

#include "treelib.h"
#include "treeprinter/pre_print_tree_analysis.h"


treelib::Tree<int> get_simple_tree(void) {
    treelib::Tree<int> tree;
    tree.create_root_node("root", "root", 3);
    tree.create_node("child1", "child1", "root", 3);
    tree.create_node("child1.1", "child1.1", "child1", 6);
    tree.create_node("child1.2", "child1.2", "child1", 8);
    tree.create_node("child2", "child2", "root", 11);
    tree.create_node("child2.1", "child2.1", "child2", 12);
    return tree;
}

void test_simple_tree_sibling_existance_map(void) {
    treelib::Tree<int> tree = get_simple_tree();
    treelib::Node<int> *root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_node_to_next_sibling_existance["child1"]);
    assert(info.m_node_to_next_sibling_existance["child1.1"]);
    assert(not info.m_node_to_next_sibling_existance["child1.2"]);
    assert(not info.m_node_to_next_sibling_existance["root"]);
    assert(not info.m_node_to_next_sibling_existance["child2"]);
    assert(not info.m_node_to_next_sibling_existance["child2.1"]);
}

void test_simple_tree_max_depth(void) {
    treelib::Tree<int> tree = get_simple_tree();
    treelib::Node<int> *root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_max_depth == 2);
}

void test_info_not_copied(void) {
    treelib::Tree<int> tree = get_simple_tree();
    treelib::Node<int> *root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(not info.m_copied);
}

int main(void) {
    test_simple_tree_sibling_existance_map();
    test_simple_tree_max_depth();
    test_info_not_copied();
}
