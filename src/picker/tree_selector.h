#ifndef __TREESELECTOR_H
#define __TREESELECTOR_H

#include <vector>
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

    // Select the first/last of the selection's siblings (that an ongoing search doesn't hide)
    void move_to_first_sibling(void);
    void move_to_last_sibling(void);

    // If an ongoing search hides the selection, select its closest visible ancestor
    void select_visible_ancestor(void);

    // Pick the selection and everything under it, or, if it's picked, unpick them all. The nodes
    // above it become picked if all under them is, and partly picked if some is.
    void toggle_pick_of_selection(void);

    // Pick (or unpick) the node alone, without the nodes under it or above it; then call
    // update_picked_dirs()
    void set_picked(NodePtr node, bool is_picked);

    // Make each node with children picked if all of them are, and partly picked if some are
    void update_picked_dirs(void);

    std::size_t get_nr_picked(void) const { return _nr_picked; }

    // The picked nodes, in the tree's order (each node before the nodes under it)
    std::vector<NodePtr> get_picked_nodes(void);

private:
    treelib::Tree& _tree;
    treelib::TreePrinter& _tree_printer;
    // Update node's picked state from its children's
    void update_from_children(treelib::Node* node);

    NodePtr _selection;
    std::size_t _nr_picked;
};

}

#endif
