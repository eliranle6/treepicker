#include <algorithm>
#include <node.h>

namespace treelib {

Node::Node(std::string _tag, std::string _identifier, void* _data, std::string _parent):
        tag(_tag),
        identifier(_identifier),
        parent(_parent),
        data(_data),
        is_matching_search(true),
        is_ancestor_of_matching_search(true)
{
}

void Node::add_child(std::shared_ptr< Node > node) {
    // Keep children sorted by tag. Children often arrive already sorted (dirtree sorts them), so
    // check the end first, to keep building a big flat dir linear rather than quadratic.
    if (children.empty() or children.back()->tag <= node->tag) {
        children.push_back(node);
    } else {
        auto position = std::upper_bound(children.begin(), children.end(), node,
            [](const std::shared_ptr< Node >& a, const std::shared_ptr< Node >& b) {
                return a->tag < b->tag;
            });
        children.insert(position, node);
    }
    node->parent = identifier;
}

}

