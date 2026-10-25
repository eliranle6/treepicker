#ifndef PRINTNODE_H__
#define PRINTNODE_H__

#include <memory>
#include <iostream>
#include <vector>

#include "tree/node.h"

namespace treelib {

// A line of the printed tree: a node, or a "... N more" line standing for siblings that don't fit
struct PrintedLine {
    // nullptr for a "... N more" line
    std::shared_ptr<Node> node;
    // Under the printed subtree root, which is 0
    int depth;
    // Whether no line of its siblings (or "... N more") comes after it, so it's drawn with "└──"
    bool is_last_sibling;
    // Whether its children have lines too (else, if it has any, it shows "(...)")
    bool are_children_printed;
    // For a "... N more" line: N
    std::size_t nr_hidden;
};

// has_next_sibling[depth]: whether the line last drawn at that depth (this one's ancestor) has
// siblings after it, so a vertical line goes down past this one
void print_node(
        std::ostream &out,
        const PrintedLine& line,
        const std::shared_ptr<Node> selection,
        const std::shared_ptr<Node> printed_subtree_root,
        Tree& tree,
        const std::vector<bool>& has_next_sibling,
        int window_width = 0
        );

// The path from the root down to printed_subtree_root, like "root > dir > ". If it's wider than
// max_width (0: no limit), it starts with "... > " instead of the dirs closest to the root.
void _print_breadcrumbs(std::ostream &out, const std::shared_ptr<Node> printed_subtree_root, Tree& tree,
                        int max_width = 0);

// The line standing for paginated siblings that don't fit, at their depth: "├── ... 12 more" above
// the printed ones, or "└── ... 12 more" below them (they're the last of their siblings)
void print_scroll_indicator(std::ostream &out, std::size_t nr_hidden_items, int depth,
                            const std::vector<bool>& has_next_sibling,
                            bool is_after_printed_siblings);

}


#endif
