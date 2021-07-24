#include "treeprinter.h"
#include "paginator.h"
#include "printnode.h"
#include "treewindowfitter.h"

namespace treelib {

TreePrinter::TreePrinter(Tree& tree): _tree(tree),
                                            info(analyze_tree_for_printing(_tree.get_root())),
                                            _printed_subtree_root(tree.get_root())
{}

void TreePrinter::init_pre_dfs_state(std::shared_ptr<treelib::Node> selection,
                                        int window_height) {
    _next_printed_node_after_selected.reset();
    _printed_node_before_selected.reset();
    _previously_printed_node.reset();

    _selection_depth = _tree.get_node_depth(selection);

    // Determine the printed subtree root
    auto printed_subtree_root_info = 
        choose_printed_subtree_root(_tree,
            selection,
            _selection_depth,
            window_height,
            _printed_subtree_root);
    _printed_subtree_root = std::get<0>(printed_subtree_root_info);
    _printed_subtree_root_depth = std::get<1>(printed_subtree_root_info);
    _nr_levels_that_fit_in_window = std::get<2>(printed_subtree_root_info);

    _was_previously_printed_node_selected = false;
    _has_paginated_in_current_print = false;
    _was_selection_printed = false;
    _first_printed_node = nullptr;
}

bool TreePrinter::should_node_be_skipped(
                           bool filter_search_nodes,
                           std::shared_ptr<Node> node) {
    return (filter_search_nodes
        and not node->is_ancestor_of_matching_search
        and not node->is_matching_search);
}

int TreePrinter::print(std::ostream &out,
                          bool filter_search_nodes,
                          std::shared_ptr<Node> selection,
                          int window_height) {
    std::vector<PrintedNode> nodes_to_print = scan_nodes_to_print(out,
        filter_search_nodes,
        selection,
        window_height);

    for (auto& printed_node: nodes_to_print) {
        // Before printing, maintain the array of depth to -
        // [whether a next (=not printed yet) sibling exists]
        // in order to know whether to print connecting lines to next sibling of that
        // depth (node depth corresponds to indentation level), if siblint such exists
        update_depth_to_next_sibling_map(
				printed_node.previous_depth,
				printed_node.depth,
                printed_node.previous_node);

        print_node(out, printed_node, selection, _printed_subtree_root, info, _tree,
			&depth_to_next_sibling[0]);
        if (_first_printed_node == nullptr) {
            _first_printed_node = printed_node.node;
        }
    }

    return nodes_to_print.size();
}

std::vector<PrintedNode> TreePrinter::scan_nodes_to_print(std::ostream &out,
                          bool filter_search_nodes,
                          std::shared_ptr<Node> selection,
                          int window_height) {
    assert(selection);

    std::vector<PrintedNode> result;
    if (nullptr == _tree.get_root()) {
        return result;
    }

    init_pre_dfs_state(selection, window_height);

    // Prepare variables for DFS scan for printing
    int rel_depth = 0;
    auto node = _printed_subtree_root;
    _dfs_stack.push(std::make_pair(node, rel_depth));

    // DFS scan
    int line_counter = 0;
    while (not _dfs_stack.empty()) {
        // Store previous values before reassigning new ones
        auto previous_depth = rel_depth;
        auto previous_node = node;

        // Extract node from DFS stack
        node = std::get<0>(_dfs_stack.top());
        rel_depth = std::get<1>(_dfs_stack.top());
        _dfs_stack.pop();

        // Skip nodes that don't match an ongoing search
        if (should_node_be_skipped(filter_search_nodes, node)) {
            continue;
        }

        // Print node
        ++line_counter;
        result.emplace_back(node, previous_node, rel_depth, previous_depth);

        // Add children to DFS stack
        expand_dfs_to_children_of_node(node, selection, rel_depth, window_height);

        // Update state that requires update during iteration
        update_mid_dfs_state(node, selection);
    }

    return result;
}

void TreePrinter::update_depth_to_next_sibling_map(int previous_depth,
                                                      int depth,
                                                      std::shared_ptr<Node> previous_node) {
    // Maintain the depth-to-next-sibnling map
    const bool is_first_node_of_parent = previous_depth < depth;
    if (is_first_node_of_parent) {
        const bool next_sibling_of_parent_exists =
            info.m_node_to_next_sibling_existence[previous_node->identifier];
        depth_to_next_sibling[depth - 1] = next_sibling_of_parent_exists;
    }
}

void TreePrinter::expand_dfs_to_children_of_node(std::shared_ptr<Node> node,
        std::shared_ptr<Node> selection,
        int rel_depth,
        int window_height) {
    // Determine whether printed subtree fits window
    int children_rel_depth = rel_depth + 1;
    const int highest_rel_depth_that_fits_window = _nr_levels_that_fit_in_window - 1;
    const bool printed_subtree_fits_window =
        children_rel_depth <= highest_rel_depth_that_fits_window;

    if (printed_subtree_fits_window) {
        // Print whole subtree
        populate_stack_with_all_children_of_node(node, children_rel_depth);
    } else if (not _has_paginated_in_current_print) {
        // Determine whether should paginate printed subtree
        int selection_rel_depth = _selection_depth - _printed_subtree_root_depth;
        bool should_paginate =
            (// Selection is in the deepest printed level (which is to paginate)
             // and so are the node's children that are to print
             selection_rel_depth == highest_rel_depth_that_fits_window + 1
             and children_rel_depth == highest_rel_depth_that_fits_window + 1)
            or node == _printed_subtree_root;

        // Print as paginated
        if (should_paginate) {
            _has_paginated_in_current_print = true;
            populate_stack_with_paginated_children_of_node(node,
                                                        selection,
                                                        children_rel_depth,
                                                        window_height);
        }
    }
}

void TreePrinter::update_mid_dfs_state(std::shared_ptr<Node> node,
                                          std::shared_ptr<Node> selection) {

    // Update next printed node after selected
    const bool is_selection = node == selection;
    if (is_selection) {
        _was_previously_printed_node_selected = true;
        _printed_node_before_selected = _previously_printed_node;
        _was_selection_printed = true;
    } else if (_was_previously_printed_node_selected) {
        _next_printed_node_after_selected = node;
        _was_previously_printed_node_selected = false;
    }

    // Update previously printed node
    _previously_printed_node = node;
}

void TreePrinter::populate_stack_with_all_children_of_node(std::shared_ptr<Node> node,
                                                              int children_depth) {
    // Add children of current node to DFS stack.
    // Iterate in reverse to cancel the stack's reverse effect
    for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
        auto current_node_depth_pair = std::make_pair(*iter, children_depth);
        _dfs_stack.push(current_node_depth_pair);
    }
}

void TreePrinter::populate_stack_with_paginated_children_of_node(
        std::shared_ptr<Node> node,
        std::shared_ptr<Node> selection,
        int child_depth,
        int window_height) {
    std::size_t nr_children = node->children.size();

    // Find index of selected child
    std::size_t selection_idx_in_level = 0;
    for (auto iter = node->children.begin();
             iter != node->children.end() and (*iter)->identifier != selection->identifier;
             ++iter, ++selection_idx_in_level);
    if (selection_idx_in_level == node->children.size()) {
        selection_idx_in_level = 0;
    }

    // Calculate number of children to hide from start and from end
    std::size_t nr_items_removed_at_the_beginning = 0;
    std::size_t nr_items_removed_at_the_end = 0;
    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_children, window_height - 1, selection_idx_in_level);

    // Skip invisible children
    std::size_t counter = 0;
    auto iter = node->children.rbegin();
    for (;iter != node->children.rend() and counter < nr_items_removed_at_the_end;
             ++iter, ++counter);

    // Iterate on visible children and push them to stack
    std::size_t nr_items_to_print = nr_children - nr_items_removed_at_the_beginning
        - nr_items_removed_at_the_end;
    for (counter = 0; counter < nr_items_to_print and iter != node->children.rend();
             ++counter, ++iter) {
        _dfs_stack.push(std::make_pair(*iter, child_depth));
    }

    // Update the next printed (_next_printed_node_after_selected), in case
    // selected node is the last printed node (in which case it won't be
    // updated in update_mid_dfs_state)
    const bool is_selecting_last_item = selection_idx_in_level >=
        nr_items_removed_at_the_beginning + nr_items_to_print - 1;
    if (is_selecting_last_item) {
        std::size_t next_item_idx = selection_idx_in_level + 1;
        const bool next_item_exists = next_item_idx < nr_children;
        if (next_item_exists) {
            _next_printed_node_after_selected = node->children[selection_idx_in_level + 1];
        }
    }
}

std::shared_ptr<treelib::Node> TreePrinter::get_next_printed_node_after_selected(void) {
    return _next_printed_node_after_selected;
}

std::shared_ptr<treelib::Node> TreePrinter::get_printed_node_before_selected(void) {
    return _printed_node_before_selected;
}

bool TreePrinter::was_selection_printed(void) {
    return _was_selection_printed;
}

std::shared_ptr<treelib::Node> TreePrinter::get_first_printed_node(void) {
    return _first_printed_node;
}

}
