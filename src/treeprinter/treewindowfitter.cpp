#include "tree/tree.h"
#include "tree/node.h"
#include "treewindowfitter.h"

namespace treelib {

int get_nr_levels_that_fit_in_window(
        Tree& tree,
        int window_size,
        std::shared_ptr<Node> printed_subtree_root) {
    using NodeP = std::shared_ptr<Node>;

    using NodeDepth = std::pair<NodeP, int>;
    std::queue<NodeDepth> bfs_queue;
    bfs_queue.push(std::make_pair(printed_subtree_root, 0));
    int nr_nodes_to_print_until_previous_level = 0;
    int nr_nodes_in_current_depth = 0;
    int nr_depths_to_print = 0;
    while(not bfs_queue.empty() and
          nr_nodes_to_print_until_previous_level + nr_nodes_in_current_depth <= window_size + 1) {

        auto node_depth_pair = bfs_queue.front();
        auto node = std::get<0>(node_depth_pair);
        auto depth = std::get<1>(node_depth_pair);
        bfs_queue.pop();
        for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
            if ((*iter)->is_matching_search or (*iter)->is_ancestor_of_matching_search) {
                auto child_node_depth_pair = std::make_pair(*iter, depth + 1);
                bfs_queue.push(child_node_depth_pair);
            }
        }

        // Add the previous fully-scanned level as a printed
        ++nr_nodes_in_current_depth;
        int previous_depth = nr_depths_to_print;
        const bool first_in_new_depth = previous_depth != depth;
        if (first_in_new_depth or bfs_queue.empty()) {
            if (nr_nodes_to_print_until_previous_level <= window_size) {
                ++nr_depths_to_print;
                nr_nodes_to_print_until_previous_level += nr_nodes_in_current_depth;
            }
            nr_nodes_in_current_depth = 1;
            if (bfs_queue.empty() and first_in_new_depth) {
                if (nr_nodes_to_print_until_previous_level + 1 <= window_size) {
                    ++nr_depths_to_print;
                }
            }
        }
    }
    return nr_depths_to_print;
}


std::tuple<std::shared_ptr<Node>, int, int> choose_printed_subtree_root(
        Tree& tree,
        std::shared_ptr<Node> selection,
        int selection_depth,
        int window_height,
        std::shared_ptr<Node> current_printed_subtree_root
        ) {
    // Update state regarding printed subtree root
    auto printed_subtree_root = current_printed_subtree_root;
    auto nr_levels_that_fit_in_window = get_nr_levels_that_fit_in_window(
        tree,
        window_height,
        printed_subtree_root);
    auto printed_subtree_root_depth = tree.get_node_depth(printed_subtree_root);

    // Set printed subtree root as the parent of selection, if selection is not the root node, and
    // if one or more of the following conditions satisfies:
    // 1. Selection is too deep to print, given the current printed subtree root
    // 2. Selection is above (closer to root) the printed subtree root, and therefore is invisible
    // 3. Selection is the printed subtree root (not allowed, unless selection is the root node).
    int max_visible_depth = printed_subtree_root_depth + nr_levels_that_fit_in_window - 1;
    const bool is_selection_too_deep_to_print = selection_depth > max_visible_depth;
    const bool is_selection_above_printed_root = selection_depth < printed_subtree_root_depth;
    const bool is_selection_printed_subtree_root = selection_depth == printed_subtree_root_depth;
    const bool should_move_printed_subtree_root = selection != tree.get_root() and (
        is_selection_too_deep_to_print or is_selection_above_printed_root
        or is_selection_printed_subtree_root);
    if (should_move_printed_subtree_root) {
        printed_subtree_root = tree.get_node(selection->parent);
        printed_subtree_root_depth = tree.get_node_depth(printed_subtree_root);
        nr_levels_that_fit_in_window = get_nr_levels_that_fit_in_window(
            tree,
            window_height,
            printed_subtree_root);
    }

    return std::make_tuple(
			printed_subtree_root,
			printed_subtree_root_depth,
			nr_levels_that_fit_in_window);
}

}
