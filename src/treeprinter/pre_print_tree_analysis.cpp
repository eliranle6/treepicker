#include <assert.h>
#include <map>
#include <queue>

#include "pre_print_tree_analysis.h"

namespace treelib {

TreeAnalysisInfo analyze_tree_for_printing(std::shared_ptr< Node > root) {
    std::map<std::string, bool> node_to_next_sibling;

    if (not root) {
        return TreeAnalysisInfo(0, node_to_next_sibling);
    }

    using node_depth_pair = typename std::pair<std::shared_ptr< Node >, int>;
    std::queue< node_depth_pair > bfs_queue;
    auto rootpair = std::make_pair(root, 0);
    bfs_queue.push(rootpair);
    std::shared_ptr< Node > previous_node = nullptr;
    int previous_depth = 0;
    int max_depth = 0;
    assert(root);
    while (not bfs_queue.empty()) {
        // Today i learned: Structured bindings (C++ 17)
        auto [node, depth] = bfs_queue.front();
        bfs_queue.pop();

        // Add all children of node to BFS queue
        for (auto iter = node->children.begin(); iter != node->children.end(); ++iter) {
            auto child_pair = std::make_pair(*iter, depth + 1);
            bfs_queue.push(child_pair);
        }

        // Store the current node and depth as previous, before reading them from the current node
        if (previous_node and previous_depth == depth and previous_node->parent == node->parent) {
            node_to_next_sibling[previous_node->identifier] = true;
        }

        previous_node = node;
        previous_depth = depth;

        // Maintain `max_depth`
        if (depth > max_depth) {
            max_depth = depth;
        }
    }
    return TreeAnalysisInfo(max_depth, node_to_next_sibling);
}

}
