#ifndef TREE_SERACH_H
#define TREE_SERACH_H

#include "tree.h"

namespace treelib {

template<typename T>
class Tree;

template<typename T>
class Search
{
public:
    Search(Tree<T> &tree) : _tree(tree) {};

    void search(std::string &keyword);

private:
    Tree<T> &_tree;

    void mark_ancestors(std::string &identifier);

    using node_t = std::shared_ptr< Node<T> >;
};

template<typename T>
void Search<T>::search(std::string &keyword) {
    // Currently,  assume that there are no cycles.
    // Also, not using concurrency yet.
    std::stack< node_t > dfs_stack;

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
        bool found = node->tag.find(keyword) != std::string::npos;
        node->is_matching_search = found;
        node->is_ancestor_of_matching_search = false;

        if (found) {
            mark_ancestors(node->identifier);
        }

        // Add children
        for (auto child : node->children) {
            dfs_stack.push(child);
        }

        ++cycle_count;
    }
}

template<typename T>
void Search<T>::mark_ancestors(std::string &identifier) {
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

} // namespace treelib

#endif
