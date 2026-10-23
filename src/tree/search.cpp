#include <algorithm>
#include <cctype>
#include <string>

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

    dfs_stack.push(root);
    while (not dfs_stack.empty()) {
        node_t node = dfs_stack.top();
        dfs_stack.pop();

        // Visit node. Match its name rather than its path: with paths, a dir that matched made
        // everything under it match too, and the root's name matched every node.
        bool found = keyword.empty() or does_text_match(node->tag, keyword, ignore_case);
        node->is_matching_search = found;
        node->is_search_highlighted = found and not keyword.empty();
        // Mark 'ancestor of matching' as false, by default, to be updated later. Not hiding
        // anything: mark every node, so all are shown.
        node->is_ancestor_of_matching_search = not hide_non_matching;

        if (found) {
            mark_ancestors(node->identifier);
            ++_nr_matching;
        }

        // Add children
        for (auto child : node->children) {
            dfs_stack.push(child);
        }
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
