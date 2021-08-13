#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>
#include <syslog.h>

#include "tree/node.h"
#include "tree/tree.h"
#include "printnode.h"

namespace treelib {

class Tree;

class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node >;

    TreePrinter(Tree &tree);
    int print(std::ostream &out, bool filter_search_nodes = false,
              node_t selection = nullptr, int window_height = 100);
    void scan_nodes_to_print(std::ostream& out, bool filter_search_nodes,
              node_t selection, int window_height);
    std::shared_ptr<treelib::Node> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node> get_printed_node_before_selected(void);
    bool was_selection_printed(void);
    std::shared_ptr< Node > get_first_printed_node(void);
    std::shared_ptr< Node > get_last_printed_node(void);
    std::shared_ptr<treelib::Node> get_printed_subtree_root(void);

private:
    void init_pre_dfs_state(std::shared_ptr<treelib::Node> selection,
                            int window_height);
    void update_mid_dfs_state(std::shared_ptr<Node> node,
        std::shared_ptr<Node> selection,
        int depth,
        std::vector<PrintedNode>& result);
    void expand_dfs_to_children_of_node(std::shared_ptr<Node> node,
                                   std::shared_ptr<Node> selection,
                                   int window_height,
                                   int rel_depth);
    void update_depth_to_next_sibling_map(int previous_depth,
                                          int depth,
                                          std::shared_ptr<Node> previous_node,
        std::vector<PrintedNode>& nodes_to_print);
    void populate_stack_with_paginated_children_of_node(node_t node,
                                       node_t selection,
                                       int child_depth,
                                       int window_height);
    void populate_stack_with_all_children_of_node(std::shared_ptr<Node> node,
                                                  int children_depth);

    std::stack<std::pair<node_t, int>> _dfs_stack;
    bool should_node_be_skipped(bool filter_search_nodes,
                                std::shared_ptr<Node> node);
    void print_breadcrumbs(std::ostream &out);
    bool will_children_of_node_be_printed(int depth);

    Tree& _tree;
    std::shared_ptr< Node > _printed_node_before_selected;
    std::shared_ptr< Node > _next_printed_node_after_selected;
    std::shared_ptr< Node > _printed_subtree_root;
    std::shared_ptr< Node > _first_printed_node;
    int _printed_subtree_root_depth;
    int _nr_levels_that_fit_in_window;
    int _selection_depth;
    static constexpr std::size_t MAX_DEPTH = 512;

    // To be used during first iteration (DFS) in order to populate PrintedNode.next_sibling
    std::vector<int> _depth_to_prev_printed_node;
    // To be used during second iteration
    std::array<bool, MAX_DEPTH> depth_to_next_sibling;
    std::vector<PrintedNode> _nodes_to_print;

    std::shared_ptr<Node> _previously_printed_node;
    std::shared_ptr<Node> _first_sibling_in_pagination;
    std::shared_ptr<Node> _last_sibling_in_pagination;
    std::size_t _nr_items_removed_at_the_beginning;
    std::size_t _nr_items_removed_at_the_end;
    bool _was_previously_printed_node_selected;
    bool _has_paginated_in_current_print;
    bool _was_selection_printed;
    bool _will_children_of_node_be_printed;
};
} // namespace treelib

#endif
