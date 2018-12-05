#ifndef __COMMON_H
#define __COMMON_H

#include "tree/tree.h"

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

treelib::Tree<int> get_simple_tree_informative_nodes(void) {
    treelib::Tree<int> tree;
    tree.create_node("root", "root", "", 3);
    tree.create_node("child1", "child1", "root", 3);
    tree.create_node("child1.1", "child1.1", "child1", 6);
    tree.create_node("child1.2", "child1.2", "child1", 8);
    tree.create_node("child2", "child2", "root", 11);
    tree.create_node("child2.1", "child2.1", "child2", 12);
    return tree;
}

#endif
