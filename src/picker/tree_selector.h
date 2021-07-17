#ifndef __TREESELECTOR_H
#define __TREESELECTOR_H

#include "tree/tree.h"
#include "treeprinter/treeprinter.h"

namespace picker {

class TreeSelector
{
public:
    using NodePtr = typename treelib::Tree::node_t;

    TreeSelector(treelib::Tree& tree, treelib::TreePrinter& tree_printer);

    NodePtr get_selection(void);

    void explore_children_of_selection(void);
    void move_to_next(void);
    void move_to_prev(void);
    void move_one_up(void);

private:
    treelib::Tree& _tree;
    treelib::TreePrinter& _tree_printer;
    NodePtr _selection;

    void _advance_selection_at_same_tree_level(int direction);
    void select_node(std::shared_ptr<treelib::Node> node);
};

}

#endif
