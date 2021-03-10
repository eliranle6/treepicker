#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>

#include "utils/guishell.h"
#include "tree/node.h"
#include "tree/tree.h"
#include "pre_print_tree_analysis.h"
#include "paginator.h"
#include "treewindowfitter.h"

namespace treelib {

template<typename T>
class Tree;

template<typename T>
class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node<T> >;
    TreePrinter(Tree<T> &tree);
    int print(std::ostream &out, bool filter_search_nodes = false,
              node_t selection = nullptr, int window_height = 100);
    std::shared_ptr<treelib::Node<T>> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node<T>> get_printed_node_before_selected(void);

private:
    void print_node(node_t node,
                    node_t previous_node,
                    int depth,
                    int previous_depth,
                    std::ostream &out,
                    std::shared_ptr<treelib::Node<T>> selection
                    );

    void init_pre_dfs_state(std::shared_ptr<treelib::Node<T>> selection,
                            int window_height);
    void update_mid_dfs_state(std::shared_ptr<Node<T>> node,
                              std::shared_ptr<Node<T>> selection);
    void expand_dfs_to_children_of_node(std::shared_ptr<Node<T>> node,
                                   std::shared_ptr<Node<T>> selection,
                                   int window_height,
                                   int rel_depth);
    void update_depth_to_next_sibling_map(int previous_depth,
                                          int depth,
                                          std::shared_ptr<Node<T>> previous_node);
    void populate_stack_with_paginated_children_of_node(node_t node,
                                       node_t selection,
                                       int child_depth,
                                       int window_height);
    void populate_stack_with_all_children_of_node(std::shared_ptr<Node<T>> node,
                                                  int children_depth);

    std::stack<std::pair<node_t, int>> _dfs_stack;
    TreeWindowFitter<T> _tree_window_fitter;
    void choose_printed_tree_root(std::shared_ptr<treelib::Node<T>> selection,
                                  int window_height);
    bool should_node_be_skipped(bool filter_search_nodes,
                                std::shared_ptr<Node<T>> node);
    void print_breadcrumbs(std::ostream &out);
    Tree<T>& _tree;
    TreeAnalysisInfo info;
    std::shared_ptr< Node<T> > _printed_node_before_selected;
    std::shared_ptr< Node<T> > _next_printed_node_after_selected;
    std::shared_ptr< Node<T> > _printed_subtree_root;
    int _printed_subtree_root_depth;
    int _nr_levels_that_fit_in_window;
    static constexpr std::size_t MAX_DEPTH = 512;
    int _selection_depth;
    std::array<bool, MAX_DEPTH> depth_to_next_sibling;
    std::shared_ptr<Node<T>> _previously_printed_node;
    bool _was_previously_printed_node_selected;
    bool _has_paginated_in_current_print;
};

template <typename T>
TreePrinter<T>::TreePrinter(Tree<T>& tree): _tree(tree),
                                            info(analyze_tree_for_printing(_tree.get_root())),
                                            _printed_subtree_root(tree.get_root())
{}

template <typename T>
void TreePrinter<T>::init_pre_dfs_state(std::shared_ptr<treelib::Node<T>> selection,
                                        int window_height) {
    _next_printed_node_after_selected.reset();
    _printed_node_before_selected.reset();
    _previously_printed_node.reset();

    _selection_depth = _tree.get_node_depth(selection);

    // Determine the printed subtree root
    choose_printed_tree_root(selection, window_height);

    _was_previously_printed_node_selected = false;
     _has_paginated_in_current_print = false;
}

template <typename T>
bool TreePrinter<T>::should_node_be_skipped(
                           bool filter_search_nodes,
                           std::shared_ptr<Node<T>> node) {
    return (filter_search_nodes
        and not node->is_ancestor_of_matching_search
        and not node->is_matching_search);
}

template <typename T>
int TreePrinter<T>::print(std::ostream &out,
                          bool filter_search_nodes,
                          std::shared_ptr<Node<T>> selection,
                          int window_height) {
    if (nullptr == _tree.get_root()) {
        return 0;
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
        print_node(node, previous_node, rel_depth, previous_depth, out, selection);

        // Add children to DFS stack
        expand_dfs_to_children_of_node(node, selection, rel_depth, window_height);

        // Update state that requires update during iteration
        update_mid_dfs_state(node, selection);
    }

    return line_counter;
}

#include <syslog.h>
template <typename T>
void TreePrinter<T>::update_depth_to_next_sibling_map(int previous_depth,
                                                      int depth,
                                                      std::shared_ptr<Node<T>> previous_node) {
    // Maintain the depth-to-next-sibnling map
    const bool is_first_node_of_parent = previous_depth < depth;
    if (is_first_node_of_parent) {
        const bool next_sibling_of_parent_exists =
            info.m_node_to_next_sibling_existance[previous_node->identifier];
        depth_to_next_sibling[depth - 1] = next_sibling_of_parent_exists;
    }
}

template<typename T>
void TreePrinter<T>::expand_dfs_to_children_of_node(std::shared_ptr<Node<T>> node,
        std::shared_ptr<Node<T>> selection,
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
            syslog(LOG_NOTICE, "Paginating children of '%s'", node->tag.c_str());
            _has_paginated_in_current_print = true;
            populate_stack_with_paginated_children_of_node(node,
                                                        selection,
                                                        children_rel_depth,
                                                        window_height);
        }
    }
}

template <typename T>
void TreePrinter<T>::update_mid_dfs_state(std::shared_ptr<Node<T>> node,
                                          std::shared_ptr<Node<T>> selection) {

    // Update next printed node after selected
    const bool is_selection = node == selection;
    if (is_selection) {
        _was_previously_printed_node_selected = true;
        _printed_node_before_selected = _previously_printed_node;
    } else if (_was_previously_printed_node_selected) {
        _next_printed_node_after_selected = node;
        _was_previously_printed_node_selected = false;
    }

    // Update previously printed node
    _previously_printed_node = node;
}

template <typename T>
void TreePrinter<T>::populate_stack_with_all_children_of_node(std::shared_ptr<Node<T>> node,
                                                              int children_depth) {
    // Add children of current node to DFS stack.
    // Iterate in reverse to cancel the stack's reverse effect
    for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
        auto current_node_depth_pair = std::make_pair(*iter, children_depth);
        _dfs_stack.push(current_node_depth_pair);
    }
}

template <typename T>
void TreePrinter<T>::populate_stack_with_paginated_children_of_node(
        std::shared_ptr<Node<T>> node,
        std::shared_ptr<Node<T>> selection,
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
    syslog(LOG_NOTICE, "selected child idx %lu", selection_idx_in_level);

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
// --
//
#define HORIZONTAL_TREE_LINE ("\xe2\x94\x80")
// |
// |
// |
#define VERTICAL_TREE_LINE ("\xe2\x94\x82")
// |
// |--
// |
#define MIDDLE_CHILD_CONNECTOR ("\xe2\x94\x9c")
// |
// |--
//
#define LAST_CHILD_CONNECTOR  ("\xe2\x94\x94")

template <typename T>
void TreePrinter<T>::print_node(node_t node,
                                node_t previous_node,
                                int depth,
                                int previous_depth,
                                std::ostream &out,
                                std::shared_ptr<treelib::Node<T>> selection
                                ) {

    // Before printing, maintain the array of depth to -
    // [whether a next (=not printed yet) sibling exists]
    // in order to know whether to print connecting lines to next sibling of that
    // depth (node depth corresponds to indentation level), if siblint such exists
    update_depth_to_next_sibling_map(previous_depth, depth, previous_node);

    // Print selection marker if node is selected node
    const bool is_selection = node == selection;
    if (is_selection) {
        out << ">";
        out << guishell::Color(guishell::BLACK_ON_BLUE);
    } else {
        out << " ";
    }
    if (node->identifier != _printed_subtree_root->identifier) {
        out << " ";
    }

    // Print connecting lines for each depth (node depth is manifested by the indentation
    // level
    for (int depth_idx = 1; depth_idx < depth; ++depth_idx) {
        if (depth_to_next_sibling[depth_idx])
            out << VERTICAL_TREE_LINE << "   ";
        else
            out << "    ";
    }
    // Connecting lines special case - if node is last node in level
    if (node->identifier != _printed_subtree_root->identifier) {
        const auto is_last_child = info.m_node_to_next_sibling_existance[node->identifier];
        if (is_last_child)
            out << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            out << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }

    // If printing root, print breadcrumbs up to node before printed subtree root
    if (node == _printed_subtree_root) {
        print_breadcrumbs(out);
    } else {
        // Print node text
        out << " ";
        if (node->children.size()) {
            auto color = is_selection ? guishell::SELECTED_DIRECTORY :
                guishell::NON_SELECTED_DIRECTORY;
            out << guishell::Color(color) << guishell::Bold() <<
                node->tag << guishell::Unbold() << guishell::Color(guishell::DEFAULT);
        } else {
            if (is_selection) {
                out << guishell::Color(guishell::WHITE_ON_BLUE);
            }
            out << node->tag;
        }
        out << std::endl;
    }

    if (is_selection) {
        out << guishell::Color(guishell::DEFAULT);
    }
}

template<typename T>
void TreePrinter<T>::print_breadcrumbs(std::ostream &out) {
    std::string breadcrumbs;
    std::shared_ptr<Node<T>> node = _printed_subtree_root;
    std::string node_id = node->identifier;
    syslog(LOG_NOTICE, "parent of subtree root (%s): %s", _printed_subtree_root->tag.c_str(),
           node_id.c_str());

    int counter = 0;
    while (++counter <= 22) {
        auto node = _tree.get_node(node_id);
        breadcrumbs = node->tag + std::string(" > ") + breadcrumbs;
        node_id = node->parent;
        syslog(LOG_NOTICE, "parent of %s is %s", node->tag.c_str(), node->parent.c_str());
        if (node == _tree.get_root()) {
            break;
        }
    }

    out << breadcrumbs << std::endl;
}

template<typename T>
void TreePrinter<T>::choose_printed_tree_root(std::shared_ptr<treelib::Node<T>> selection,
                                              int window_height) {
    // Update state regarding printed subtree root
    _nr_levels_that_fit_in_window = _tree_window_fitter.get_nr_levels_that_fit_in_window(
        _tree,
        window_height,
        _printed_subtree_root);
    _printed_subtree_root_depth = _tree.get_node_depth(_printed_subtree_root);

    // Adjust printed subtree root to make sure selection is visible
    // Check if selection is within visible (printed) range. If not,
    // change printed subtree root to parent of selection
    int max_visible_depth = _printed_subtree_root_depth + _nr_levels_that_fit_in_window - 1;
    if (_selection_depth > max_visible_depth
        or _selection_depth <= _printed_subtree_root_depth) {
        _printed_subtree_root = _tree.get_node(selection->parent);
        _printed_subtree_root_depth = _tree.get_node_depth(_printed_subtree_root);
        _nr_levels_that_fit_in_window = _tree_window_fitter.get_nr_levels_that_fit_in_window(
            _tree,
            window_height,
            _printed_subtree_root);
    }
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_next_printed_node_after_selected(void) {
    return _next_printed_node_after_selected;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_printed_node_before_selected(void) {
    return _printed_node_before_selected;
}

} // namespace treelib
#endif
