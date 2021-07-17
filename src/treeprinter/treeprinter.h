#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>
#include <syslog.h>

#include "utils/guishell.h"
#include "tree/node.h"
#include "tree/tree.h"
#include "pre_print_tree_analysis.h"
#include "treewindowfitter.h"

namespace treelib {

class Tree;

class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node >;

    struct PrintedNode {
        PrintedNode(
            std::shared_ptr<Node> _node,
            std::shared_ptr<Node> _previous_node,
            int _depth,
            int _previous_depth):
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

    TreePrinter(Tree &tree);
    int print(std::ostream &out, bool filter_search_nodes = false,
              node_t selection = nullptr, int window_height = 100);
    std::vector<PrintedNode> preprocess(std::ostream& out, bool filter_search_nodes,
              node_t selection, int window_height);
    std::shared_ptr<treelib::Node> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node> get_printed_node_before_selected(void);
    bool was_selection_printed(void);
    std::shared_ptr< Node > get_first_printed_node(void);

private:
    void print_node(std::ostream &out, PrintedNode &printed_node,
        std::shared_ptr<treelib::Node> selection);

    void init_pre_dfs_state(std::shared_ptr<treelib::Node> selection,
                            int window_height);
    void update_mid_dfs_state(std::shared_ptr<Node> node,
                              std::shared_ptr<Node> selection);
    void expand_dfs_to_children_of_node(std::shared_ptr<Node> node,
                                   std::shared_ptr<Node> selection,
                                   int window_height,
                                   int rel_depth);
    void update_depth_to_next_sibling_map(int previous_depth,
                                          int depth,
                                          std::shared_ptr<Node> previous_node);
    void populate_stack_with_paginated_children_of_node(node_t node,
                                       node_t selection,
                                       int child_depth,
                                       int window_height);
    void populate_stack_with_all_children_of_node(std::shared_ptr<Node> node,
                                                  int children_depth);

    std::stack<std::pair<node_t, int>> _dfs_stack;
    TreeWindowFitter _tree_window_fitter;
    void choose_printed_tree_root(std::shared_ptr<treelib::Node> selection,
                                  int window_height);
    bool should_node_be_skipped(bool filter_search_nodes,
                                std::shared_ptr<Node> node);
    void print_breadcrumbs(std::ostream &out);

    Tree& _tree;
    TreeAnalysisInfo info;
    std::shared_ptr< Node > _printed_node_before_selected;
    std::shared_ptr< Node > _next_printed_node_after_selected;
    std::shared_ptr< Node > _printed_subtree_root;
    std::shared_ptr< Node > _first_printed_node;
    int _printed_subtree_root_depth;
    int _nr_levels_that_fit_in_window;
    static constexpr std::size_t MAX_DEPTH = 512;
    int _selection_depth;
    std::array<bool, MAX_DEPTH> depth_to_next_sibling;
    std::shared_ptr<Node> _previously_printed_node;
    bool _was_previously_printed_node_selected;
    bool _has_paginated_in_current_print;
    bool _was_selection_printed;
};
} // namespace treelib

#endif
