#include "tree/node.h"

namespace treelib {

Node::Node(std::string _tag, std::string _identifier, std::string _parent):
        tag(_tag),
        identifier(_identifier),
        parent(_parent),
        is_search_visible(true),
        is_search_match(false),
        is_picked(false),
        is_partly_picked(false),
        kind(0),
        is_disabled(false),
        are_children_loaded(true)
{
}

void Node::add_child(std::shared_ptr< Node > node) {
    // Children keep the order they're added in: the tree doesn't know what it holds, so whoever
    // fills it decides the order (e.g. pathtree sorts by name)
    children.push_back(node);
    node->parent = identifier;
}

}

