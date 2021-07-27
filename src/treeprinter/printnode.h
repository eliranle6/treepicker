#ifndef PRINTNODE_H__
#define PRINTNODE_H__

#include <memory>
#include <iostream>

#include "tree/node.h"

namespace treelib {

struct PrintedNode {
    PrintedNode(
        std::shared_ptr<Node> _node,
        int _depth,
        bool _are_children_printed
        ):
        node(_node),
        depth(_depth),
		next_sibling_exists(false),
        are_children_printed(_are_children_printed)
        {}

    std::shared_ptr<Node> node;
    int depth;
	bool next_sibling_exists;
    bool are_children_printed;
};

void print_node(
        std::ostream &out,
        PrintedNode& printed_node,
        const std::shared_ptr<Node> selection,
        const std::shared_ptr<Node> printed_subtree_root,
        Tree& tree,
		bool* depth_to_next_sibling
        );

void _print_breadcrumbs(std::ostream &out, const std::shared_ptr<Node> printed_subtree_root, Tree& tree);

}


#endif
