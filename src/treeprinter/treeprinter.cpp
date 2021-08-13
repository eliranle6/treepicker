#include "treeprinter.h"
#include "paginator.h"
#include "printnode.h"
#include "treewindowfitter.h"

namespace treelib {

TreePrinter::TreePrinter(Tree& tree): _tree(tree),
                                            _printed_subtree_root(tree.get_root())
{}

void TreePrinter::init_pre_dfs_state(std::shared_ptr<treelib::Node> selection,
                                        int window_height) {
    _next_printed_node_after_selected.reset();
    _printed_node_before_selected.reset();
    _previously_printed_node.reset();
    _last_sibling_in_pagination.reset();
    _first_sibling_in_pagination.reset();

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
    _depth_to_prev_printed_node.clear();
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
    scan_nodes_to_print(out, filter_search_nodes, selection, window_height);

    for (auto& printed_node: _nodes_to_print) {
        // Before printing, maintain the array of depth to -
        // [whether a next (=not printed yet) sibling exists]
        // in order to know whether to print connecting lines to next sibling of that
        // depth (node depth corresponds to indentation level), if siblint such exists
        const bool is_first_in_pagination = printed_node.node == _first_sibling_in_pagination;
        const bool is_last_in_pagination = printed_node.node == _last_sibling_in_pagination;
        if (is_first_in_pagination) {
            print_top_scroll_indicator(out, _nr_items_removed_at_the_beginning);
        }
        depth_to_next_sibling[printed_node.depth] = printed_node.next_sibling_exists;
        print_node(out, printed_node, selection, _printed_subtree_root, _tree,
            &depth_to_next_sibling[0], is_last_in_pagination);
        if (is_last_in_pagination) {
            print_top_scroll_indicator(out, _nr_items_removed_at_the_end);
        }

    }

    return _nodes_to_print.size();
}

void TreePrinter::scan_nodes_to_print(std::ostream &out,
                          bool filter_search_nodes,
                          std::shared_ptr<Node> selection,
                          int window_height) {
    assert(selection);

    _nodes_to_print.clear();
    if (nullptr == _tree.get_root()) {
        return;
    }

    init_pre_dfs_state(selection, window_height);

    // Prepare variables for DFS scan for printing
    int depth = 0;
    auto node = _printed_subtree_root;
    _dfs_stack.push(std::make_pair(node, depth));

    // DFS scan
    while (not _dfs_stack.empty()) {
        // Extract node from DFS stack
        node = std::get<0>(_dfs_stack.top());
        depth = std::get<1>(_dfs_stack.top());
        _dfs_stack.pop();

        // Skip nodes that don't match an ongoing search
        if (should_node_be_skipped(filter_search_nodes, node)) {
            continue;
        }

        // Print ('visit') node
        _nodes_to_print.emplace_back(node, depth, will_children_of_node_be_printed(depth));

        // Add children to DFS stack
        expand_dfs_to_children_of_node(node, selection, depth, window_height);

        // Update state that requires update during iteration
        update_mid_dfs_state(node, selection, depth, _nodes_to_print);
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
        std::shared_ptr<Node> selection,
        int depth,
        std::vector<PrintedNode>& result) {
    // Set the 'whether a next sibling exists' for previous sibling,
    // if such exists (=current node is not a first sibling), to true.
    // Current node is first sibling iff its depth is bigger than
    // previously iterated-on node (since it's DFS)
    const bool is_root = depth == 0;
    if (not is_root) {
        assert(result.size() >= 2); // root and at least another node
        auto prev_node_idx = result.size() - 2;
        auto previous_depth = result[prev_node_idx].depth;
        const bool is_first_sibling = depth == previous_depth + 1;
        if (not is_first_sibling) {
            assert(depth <= previous_depth);
            auto& prev_node = result[_depth_to_prev_printed_node[depth]];
            prev_node.next_sibling_exists = true;
        }
    }
    // Maintain the depth to previously-printed-node vector
    // (which is used for above logic of setting the 'whether a next
    // sibling exists')
    unsigned int required_size = depth + 1;
    _depth_to_prev_printed_node.resize(required_size);
    _depth_to_prev_printed_node[depth] = result.size() - 1;

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
    std::tie(_nr_items_removed_at_the_beginning, _nr_items_removed_at_the_end) =
        paginate(nr_children, window_height - 1, selection_idx_in_level);

    // Skip invisible children
    std::size_t counter = 0;
    auto iter = node->children.rbegin();
    for (;iter != node->children.rend() and counter < _nr_items_removed_at_the_end;
             ++iter, ++counter);

    // Iterate on visible children and push them to stack
    std::size_t nr_items_to_print = nr_children - _nr_items_removed_at_the_beginning
        - _nr_items_removed_at_the_end;
    for (counter = 0; counter < nr_items_to_print and iter != node->children.rend();
             ++counter, ++iter) {

        // Update the first/last sibling pointers, in order for the print loop to
        // know when to add '(... x more)' rows at the beginning and/or end.
        const bool is_last_sibling = counter == 0;
        if (is_last_sibling and _nr_items_removed_at_the_end > 0) {
            _last_sibling_in_pagination = *iter;
        } else if (_nr_items_removed_at_the_beginning) {
            const bool is_first_sibling = counter == nr_items_to_print - 1;
            if (is_first_sibling and not is_last_sibling) {
                _first_sibling_in_pagination = *iter;
            }
        }

        _dfs_stack.push(std::make_pair(*iter, child_depth));
    }

    // Update the next printed (_next_printed_node_after_selected), in case
    // selected node is the last printed node (in which case it won't be
    // updated in update_mid_dfs_state)
    const bool is_selecting_last_item = selection_idx_in_level >=
        _nr_items_removed_at_the_beginning + nr_items_to_print - 1;
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
    std::shared_ptr<treelib::Node> result;
    if (_nodes_to_print.size()) {
        result = _nodes_to_print[0].node;
    }
    return result;
}

std::shared_ptr<treelib::Node> TreePrinter::get_last_printed_node(void) {
    std::shared_ptr<treelib::Node> result;
    if (_nodes_to_print.size()) {
        result = _nodes_to_print[_nodes_to_print.size() - 1].node;
    }
    return result;
}

bool TreePrinter::will_children_of_node_be_printed(int depth) {
    const int highest_rel_depth_that_fits_window = _nr_levels_that_fit_in_window - 1;
    return depth < highest_rel_depth_that_fits_window;
}

std::shared_ptr<treelib::Node> TreePrinter::get_printed_subtree_root(void) {
    return _printed_subtree_root;
}

}
