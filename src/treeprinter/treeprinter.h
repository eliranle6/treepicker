#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>

#include "tree/node.h"
#include "tree/tree.h"
#include "printnode.h"

namespace treelib {

class Tree;

// Prints the part of the tree that fits in the window around the selection. It lays out the lines
// to show first (layout()), then draws them (print()), and says what was laid out, so the
// selection can move through what's shown.
class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node >;

    TreePrinter(Tree &tree);
    // Lay out and draw; returns the number of lines drawn. window_width (0: no limit) keeps the
    // breadcrumbs from being cut by the screen's edge.
    int print(std::ostream &out, bool filter_search_nodes, node_t selection, int window_height,
              int window_width = 0);
    // Lay out without drawing. filter_search_nodes hides what a search doesn't show.
    void layout(bool filter_search_nodes, node_t selection, int window_height);

    // What the last layout showed
    std::shared_ptr<treelib::Node> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node> get_printed_node_before_selected(void);
    bool was_selection_printed(void);
    std::shared_ptr< Node > get_first_printed_node(void);
    std::shared_ptr< Node > get_last_printed_node(void);
    std::shared_ptr<treelib::Node> get_printed_subtree_root(void);

private:
    // Add node's shown children, and theirs, down to max_depth
    void add_descendants(const std::shared_ptr<Node>& node, int depth, int max_depth);
    // Add the root's shown children, as many as fit around the selection, with a "... N more"
    // line for those that don't
    void add_paginated_children(const std::shared_ptr<Node>& root, node_t selection,
                                int window_height);
    // The node's children that are shown (pointers, since copying shared_ptrs of 100k children
    // took ms)
    std::vector<const std::shared_ptr<Node>*> get_shown_children(const std::shared_ptr<Node>& node);
    bool is_shown(const std::shared_ptr<Node>& node);

    Tree& _tree;
    std::shared_ptr< Node > _printed_subtree_root;
    bool _filter_search_nodes;
    // The lines to draw, from the top: the printed subtree root first
    std::vector<PrintedLine> _lines;
    std::shared_ptr< Node > _printed_node_before_selected;
    std::shared_ptr< Node > _next_printed_node_after_selected;
    bool _was_selection_printed;
};
} // namespace treelib

#endif
