#include <vector>
#include "tree/tree.h"
#include "tree/node.h"
#include "treewindowfitter.h"

namespace treelib {

int get_nr_levels_that_fit_in_window(
        Tree& tree,
        int window_size,
        std::shared_ptr<Node> printed_subtree_root) {
    using NodeP = std::shared_ptr<Node>;

    // Add levels (of nodes that an ongoing search doesn't hide) while all of their nodes fit,
    // one line each. Stop counting a level once it's too big, so a huge dir costs no more than
    // a small one.
    int nr_levels = 1;
    int nr_nodes = 1;
    std::vector<NodeP> level = {printed_subtree_root};
    while (true) {
        std::vector<NodeP> next_level;
        for (auto& node : level) {
            for (auto& child : node->children) {
                if (not child->is_search_visible) {
                    continue;
                }
                if (nr_nodes + static_cast<int>(next_level.size()) + 1 > window_size) {
                    return nr_levels;
                }
                next_level.push_back(child);
            }
        }
        if (next_level.empty()) {
            return nr_levels;
        }
        ++nr_levels;
        nr_nodes += next_level.size();
        level = std::move(next_level);
    }
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
    if (selection == tree.get_root() and printed_subtree_root != selection) {
        // E.g. a search hid the selection, so it moved up to the root: show from the root
        printed_subtree_root = selection;
        printed_subtree_root_depth = 0;
        nr_levels_that_fit_in_window = get_nr_levels_that_fit_in_window(
            tree,
            window_height,
            printed_subtree_root);
    } else if (should_move_printed_subtree_root) {
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
