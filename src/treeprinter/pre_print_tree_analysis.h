#ifndef __PRE_PRINT_TREE_ANALYSIS
#define __PRE_PRINT_TREE_ANALYSIS

#include "node.h"

namespace treelib {

template <typename T>
class Tree;

struct TreeAnalysisInfo {
    std::map<std::string, bool> m_node_to_next_sibling_existance;
    int m_max_depth;
    bool m_copied;

    TreeAnalysisInfo(int max_depth, std::map<std::string, bool> node_to_next_sibling_existance) :
        m_node_to_next_sibling_existance(node_to_next_sibling_existance),
        m_max_depth(max_depth),
        m_copied(false)
    {
    }
    TreeAnalysisInfo(TreeAnalysisInfo& info) {
        info.m_copied = true;
        m_copied = true;
    }
};

template <typename T>
TreeAnalysisInfo analyze_tree_for_printing(const Node<T> *root) {
    using node_depth_pair = std::pair<const Node<T>*, int>;
    std::queue< node_depth_pair > bfs_queue;
    auto rootpair = std::make_pair(root, 0);
    bfs_queue.push(rootpair);
    node_depth_pair previous_node_depth_pair = std::make_pair(nullptr, 0);
    std::map<std::string, bool> node_to_next_sibling;
    int max_depth = 0;
    while (not bfs_queue.empty()) {
        // Today i learned: Structured bindings (C++ 17)
        auto [node, depth] = bfs_queue.front();
        bfs_queue.pop();

        // Add all children of node to BFS queue
        for (auto child : node->children) {
            auto child_pair = std::make_pair(child, depth + 1);
            bfs_queue.push(child_pair);
        }

        // Store the current node and depth as previous, before reading them from the current node
        auto [previous_node, previous_depth] = previous_node_depth_pair;
        if (previous_node and previous_depth == depth and previous_node->parent == node->parent) {
            node_to_next_sibling[previous_node->identifier] = true;
        }
        previous_node_depth_pair = std::make_pair(node, depth);

        // Maintain `max_depth`
        if (depth > max_depth) {
            max_depth = depth;
        }
    }
    TreeAnalysisInfo result(max_depth, node_to_next_sibling);
    return result;
}

}

#endif
