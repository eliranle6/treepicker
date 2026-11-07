#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "tree/search.h"
#include "tree/tree.h"

namespace treelib {

// Smartcase, like vim: ignore case unless the keyword has an uppercase letter (ASCII only)
static bool does_text_match(const std::string& text, const std::string& keyword,
                            bool ignore_case) {
    if (not ignore_case) {
        return text.find(keyword) != std::string::npos;
    }
    auto equal_ignoring_case = [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) ==
               std::tolower(static_cast<unsigned char>(b));
    };
    return std::search(text.begin(), text.end(), keyword.begin(), keyword.end(),
                       equal_ignoring_case) != text.end();
}

void Search::search(std::string keyword, bool hide_non_matching) {
    // Currently, assume that there are no cycles (create_node can't make any).
    // Also, not using concurrency yet.
    std::stack< node_t > dfs_stack;
    _nr_matching = 0;

    // Validate that the tree is not empty
    auto root = _tree.get_root();
    if (root == nullptr) {
        return;
    }

    const bool ignore_case = std::none_of(keyword.begin(), keyword.end(),
        [](char c) { return std::isupper(static_cast<unsigned char>(c)); });

    // The nodes in a DFS's preorder (each before the nodes under it)
    std::vector<Node*> preorder;
    dfs_stack.push(root);
    while (not dfs_stack.empty()) {
        node_t node = dfs_stack.top();
        dfs_stack.pop();
        preorder.push_back(node.get());
        for (auto& child : node->children) {
            dfs_stack.push(child);
        }
    }
    // With a filter, what it passes: the nodes without children it passes, and the dirs with one
    // of them under them (worked out children first, kept in is_search_visible for their parents)
    std::vector<bool> passes(preorder.size(), true);
    if (_filter) {
        for (std::size_t i = preorder.size(); i-- > 0;) {
            Node* node = preorder[i];
            bool does_pass = node->children.empty() and _filter(*node);
            for (auto& child : node->children) {
                does_pass = does_pass or child->is_search_visible;
            }
            node->is_search_visible = does_pass;
            passes[i] = does_pass;
        }
    }

    for (std::size_t i = 0; i < preorder.size(); ++i) {
        Node* node = preorder[i];
        // Visit node. Match its name rather than its path: with paths, a dir that matched made
        // everything under it match too, and the root's name matched every node.
        const bool is_match = passes[i] and not keyword.empty() and
            does_text_match(node->tag, keyword, ignore_case);
        node->is_search_match = is_match;
        // Hidden until it or a node under it matches (found after it: its children come later),
        // unless nothing is hidden
        node->is_search_visible = passes[i] and (keyword.empty() or not hide_non_matching);
        if (is_match) {
            node->is_search_visible = true;
            show_ancestors(node);
        }
        if (is_match or (keyword.empty() and passes[i])) {
            ++_nr_matching;
        }
    }
    // The root is always shown
    root->is_search_visible = true;
}

void Search::show_ancestors(Node* node) {
    const Node* root = _tree.get_root().get();
    for (Node* current = node; current != root;) {
        current = _tree.get_node(current->parent).get();
        // Then so are its ancestors
        if (current->is_search_visible) {
            break;
        }
        current->is_search_visible = true;
    }
}

}
