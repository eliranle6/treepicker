#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>

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
    void print(std::ostream &out, Tree<T> &tree, bool filter_search_nodes = false,
               node_t selection = nullptr, int window_height = 100);
    std::shared_ptr<treelib::Node<T>> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node<T>> get_previously_printed_node_before_selected(void);

private:
    void print_node(node_t node, int depth, std::vector<bool> &depth_to_next_sibling_existance,
                    const Tree<T> &tree, bool is_last_child,
                    std::ostream &out, bool is_selection,
                    node_t printed_subtree_root);

    std::shared_ptr<treelib::Node<T>> choose_printed_tree_root(
        treelib::Tree<T> tree,
        std::shared_ptr<treelib::Node<T>> selection);
    Tree<T>& _tree;
    std::shared_ptr< Node<T> > previously_printed_node_before_selected;
    std::shared_ptr< Node<T> > next_printed_node_after_selected;
    std::shared_ptr< Node<T> > _printed_tree_root;
    bool was_previously_printed_node_selected;
};

template <typename T>
TreePrinter<T>::TreePrinter(Tree<T>& tree): _tree(tree) {
}

template <typename T>
void TreePrinter<T>::print(std::ostream &out, Tree<T> &tree,
                           bool filter_search_nodes,
                           node_t selection,
                           int window_height) {
    node_t root = tree.get_root();
    if (nullptr == root) {
        return;
    }
    next_printed_node_after_selected.reset();
    previously_printed_node_before_selected.reset();
    auto printed_subtree_root = choose_printed_tree_root(tree, selection);
    TreeWindowFitter<T> tree_window_fitter;
    int nr_levels_that_fit_in_window = tree_window_fitter.get_nr_levels_that_fit_in_window(
        tree,
        window_height,
        printed_subtree_root);
    const bool paginate_last_level = nr_levels_that_fit_in_window == 1;
    TreeAnalysisInfo info = analyze_tree_for_printing(root);
    std::stack< std::pair<node_t, int> > dfs_stack;
    int depth = 0;
    auto node = printed_subtree_root;
    dfs_stack.push(std::make_pair(node, depth));
    std::vector<bool> depth_to_next_sibling(info.m_max_depth);
    was_previously_printed_node_selected = false;
    std::shared_ptr<Node<T>> previously_printed_node;
    while (not dfs_stack.empty()) {
        // Store previous values before reassigning new ones
        auto previous_depth = depth;
        auto previous_node = node;

        // Extract node from DFS stack
        auto current_node_depth_pair = dfs_stack.top();

        // Not quite sure why, but structure bindings don't work here as the following line:
        // auto [node, depth] = current_node_depth_pair;
        // So std::get is used instad.
        node = std::get<0>(current_node_depth_pair);
        depth = std::get<1>(current_node_depth_pair);
        dfs_stack.pop();

        // Filter out nodes that don't match an ongoing search
        if (filter_search_nodes) {
            if (not node->is_ancestor_of_matching_search and not node->is_matching_search) {
                continue;
            }
        }

        // Maintain the depth-to-next-sibnling map
        const bool is_first_node_of_parent = previous_depth < depth;
        if (is_first_node_of_parent) {
            const bool next_sibling_of_parent_exists =
                info.m_node_to_next_sibling_existance[previous_node->identifier];
            depth_to_next_sibling[depth - 1] = next_sibling_of_parent_exists;
        }
        const auto is_last_child = info.m_node_to_next_sibling_existance[node->identifier];

        // Print node
        const bool is_selection = node.get() == selection.get();
        print_node(node, depth, depth_to_next_sibling, tree, is_last_child, out,
                   is_selection, printed_subtree_root);

        if (is_selection) {
            was_previously_printed_node_selected = true;
            previously_printed_node_before_selected = previously_printed_node;
        } else if (was_previously_printed_node_selected) {
            next_printed_node_after_selected = node;
            was_previously_printed_node_selected = false;
        } else {
        }
        previously_printed_node = node;

        auto child_depth = depth + 1;
        const int last_depth_that_fits_in_window = nr_levels_that_fit_in_window - 1;
        const bool next_depth_fits_in_window = child_depth <= last_depth_that_fits_in_window;
        const bool is_at_last_depth = depth == last_depth_that_fits_in_window;
        const bool paginate_next_depth = is_at_last_depth and paginate_last_level;

        if (next_depth_fits_in_window) {
        // Add children of current node to DFS stack. Iterate in reverse to cancel the stack's reverse effect
            for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
                auto current_node_depth_pair = std::make_pair(*iter, child_depth);
                dfs_stack.push(current_node_depth_pair);
            }
        } else if (paginate_next_depth) {
            size_t nr_children = node->children.size();
            int selection_idx = 0;
            for (auto iter = node->children.begin();
                 iter != node->children.end() and (*iter)->identifier != selection->identifier;
                 ++iter) {
                ++selection_idx;
            }
            int nr_items_removed_at_the_beginning = 0;
            int nr_items_removed_at_the_end = 0;
            std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
                paginate(nr_children, window_height - 1, selection_idx);
            int counter = 0;
            auto iter = node->children.rbegin();
            for (;iter != node->children.rend()
                 and counter < nr_items_removed_at_the_end;
                 ++iter, ++counter) {
            }
            int nr_items_to_print = nr_children - nr_items_removed_at_the_beginning
                - nr_items_removed_at_the_end;
            for (counter = 0;
                 counter < nr_items_to_print and iter != node->children.rend();
                 ++counter, ++iter) {
                auto current_node_depth_pair = std::make_pair(*iter, child_depth);
                dfs_stack.push(current_node_depth_pair);
            }
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
void TreePrinter<T>::print_node(node_t node, int depth, std::vector<bool> &depth_to_next_sibling,
                                const Tree<T> &tree, bool is_last_child,
                                std::ostream &out, bool is_selection,
                                node_t printed_subtree_root) {
    if (is_selection) {
        out << ">";
    } else {
        out << " ";
    }
    if (node->identifier != printed_subtree_root->identifier) {
        out << " ";
    }
    for (int depth_idx = 1; depth_idx < depth; ++depth_idx) {
        if (depth_to_next_sibling[depth_idx])
            out << VERTICAL_TREE_LINE << "   ";
        else
            out << "    ";
    }
    if (node->identifier != printed_subtree_root->identifier) {
        if (is_last_child)
            out << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            out << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }
    out << " " << node->tag << std::endl;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::choose_printed_tree_root(
        treelib::Tree<T> tree,
        std::shared_ptr<treelib::Node<T>> selection) {
    auto result = tree.get_root();
    if (selection != tree.get_root()) {
        result = tree.get_node(selection->parent);
    }
    return result;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_next_printed_node_after_selected(void) {
    return next_printed_node_after_selected;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_previously_printed_node_before_selected(void) {
    return previously_printed_node_before_selected;
}

} // namespace treelib
#endif
