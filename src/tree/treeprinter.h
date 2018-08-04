#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>

#include "node.h"
#include "tree.h"
#include "pre_print_tree_analysis.h"

namespace treelib {

template<typename T>
class Tree;

template<typename T>
class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node<T> >;
    void print(std::ostream &buf, const Tree<T> &tree);

private:
    void print_node(node_t node, int depth, std::vector<bool> &depth_to_next_sibling_existance,
                    const Tree<T> &tree, bool is_last_child);
};

template <typename T>
void TreePrinter<T>::print(std::ostream &buf, const Tree<T> &tree) {
    node_t root = tree.get_root();
    if (nullptr == root) {
        return;
    }
    TreeAnalysisInfo info = analyze_tree_for_printing(root);
    std::stack< std::pair<node_t, int> > dfs_stack;
    int depth = 0;
    auto node = root;
    dfs_stack.push(std::make_pair(node, depth));
    std::vector<bool> depth_to_next_sibling(info.m_max_depth);
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

        // Maintain the depth-to-next-sibnling map
        const bool is_first_node_of_parent = previous_depth < depth;
        if (is_first_node_of_parent) {
            const bool next_sibling_of_parent_exists =
                info.m_node_to_next_sibling_existance[previous_node->identifier];
            depth_to_next_sibling[depth - 1] = next_sibling_of_parent_exists;
        }
        const auto is_last_child = info.m_node_to_next_sibling_existance[node->identifier];

        // Print node
        print_node(node, depth, depth_to_next_sibling, tree, is_last_child);

        // Add children of current node to DFS stack. Iterate in reverse to cancel the stack's reverse effect
        for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
        //for (auto child : node->children) {
            auto current_node_depth_pair = std::make_pair(*iter, depth + 1);
            dfs_stack.push(current_node_depth_pair);
        }
    }
}
//
// Not writing non-ASCII chars in code
//
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
                                const Tree<T> &tree, bool is_last_child) {
    for (int depth_idx = 1; depth_idx < depth; ++depth_idx) {
        if (depth_to_next_sibling[depth_idx])
            std::cout << VERTICAL_TREE_LINE << "  ";
        else
            std::cout << "   ";
    }
    if (node->identifier != tree.get_root()->identifier) {
        if (is_last_child)
            std::cout << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            std::cout << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }
    std::cout << node->tag << std::endl;
}

} // namespace treelib
#endif
