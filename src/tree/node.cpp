#include "tree/node.h"

namespace treelib {

Node::Node(std::string _tag, std::string _identifier, void* _data, std::string _parent):
        tag(_tag),
        identifier(_identifier),
        parent(_parent),
        data(_data),
        is_matching_search(true),
        is_ancestor_of_matching_search(true),
        is_search_highlighted(false),
        is_picked(false),
        is_partly_picked(false),
        kind(0)
{
}

void Node::add_child(std::shared_ptr< Node > node) {
    // Children keep the order they're added in: the tree doesn't know what it holds, so whoever
    // fills it decides the order (e.g. dirtree sorts by name)
    children.push_back(node);
    node->parent = identifier;
}

}

