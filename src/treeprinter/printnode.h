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
        bool* depth_to_next_sibling,
        bool is_last_in_pagination,
        int window_width = 0
        );

// The path from the root down to printed_subtree_root, like "root > dir > ". If it's wider than
// max_width (0: no limit), it starts with "... > " instead of the dirs closest to the root.
void _print_breadcrumbs(std::ostream &out, const std::shared_ptr<Node> printed_subtree_root, Tree& tree,
                        int max_width = 0);

void print_top_scroll_indicator(std::ostream &out, std::size_t nr_hidden_items);

}


#endif
