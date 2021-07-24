#ifndef PRINTNODE_H__
#define PRINTNODE_H__

#include <memory>
#include <iostream>

#include "tree/node.h"
#include "pre_print_tree_analysis.h"

namespace treelib {

struct PrintedNode {
    PrintedNode(
        std::shared_ptr<Node> _node,
        std::shared_ptr<Node> _previous_node,
        int _depth,
        int _previous_depth
        ):
        node(_node),
        previous_node(_previous_node),
        depth(_depth),
        previous_depth(_previous_depth)
        {}

    std::shared_ptr<Node> node;
    std::shared_ptr<Node> previous_node;
    int depth;
    int previous_depth;
};

void print_node(
        std::ostream &out,
        PrintedNode& printed_node,
        const std::shared_ptr<Node> selection,
        const std::shared_ptr<Node> printed_subtree_root,
        TreeAnalysisInfo& info,
        Tree& tree,
		bool* depth_to_next_sibling
        );

void _print_breadcrumbs(std::ostream &out, const std::shared_ptr<Node> printed_subtree_root, Tree& tree);

}


#endif
