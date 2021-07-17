#include <string>

#include "tree/search.h"
#include "tree/tree.h"

namespace treelib {

void Search::search(std::string keyword) {
    // Currently,  assume that there are no cycles.
    // Also, not using concurrency yet.
    std::stack< node_t > dfs_stack;
    _nr_matching = 0;

    // Validate that the tree is not empty
    auto root = _tree.get_root();
    if (root == nullptr) {
        return;
    }

    dfs_stack.push(root);

    // Use a simple cycle counter to prevent infinite loops (TODO: implement a better
    // cycle detection with a set or something)
    int cycle_count = 0;
    static const int cycle_count_limit = 999999;
    while (not dfs_stack.empty() and cycle_count < cycle_count_limit) {
        node_t node = dfs_stack.top();
        dfs_stack.pop();

        // Visit node
        bool found = keyword.empty() or node->path.find(keyword) != std::string::npos;
        node->is_matching_search = found;
        // Mark 'ancestor of matching' as false, by default, to be updated later
        node->is_ancestor_of_matching_search = false;

        if (found) {
            mark_ancestors(node->identifier);
            ++_nr_matching;
        }

        // Add children
        for (auto child : node->children) {
            dfs_stack.push(child);
        }

        ++cycle_count;
    }
}

void Search::mark_ancestors(std::string &identifier) {
    const node_t root = _tree.get_root();

    node_t current = _tree.get_node(identifier);
    while (current != root) {
        current = _tree.get_node(current->parent);

        if (current->is_ancestor_of_matching_search) {
            break;
        }
        current->is_ancestor_of_matching_search = true;
    };
}

}
