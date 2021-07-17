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
    // Find position to keep sorted order (todo: optimize this)
    auto position = children.begin();
    for (; position != children.end() && node->tag > (*position)->tag; ++position);

    children.insert(position, node);
    node->parent = identifier;
}

}

