#ifndef __NODE_H
#define __NODE_H

#include <memory>

namespace treelib {

template <class T>
class Node
{
public:
    Node(std::string _tag, std::string _identifier, T _data, std::string parent);

    std::string tag;
    std::string identifier;
    std::string parent;
    std::string path;
    T data;

    bool is_matching_search;
    bool is_ancestor_of_matching_search;

    std::vector< std::shared_ptr< Node<T> > > children;
    void add_child(std::shared_ptr< Node<T> > node);

private:
    std::shared_ptr< Node<T> > parent_node;
};

template <class T>
Node<T>::Node(std::string _tag, std::string _identifier, T _data, std::string _parent):
        tag(_tag),
        identifier(_identifier),
        parent(_parent),
        data(_data),
        is_matching_search(true),
        is_ancestor_of_matching_search(true)
{
}

template <class T>
void Node<T>::add_child(std::shared_ptr< Node<T> > node) {
    // Find position to keep sorted order (todo: optimize this)
    auto position = children.begin();
    for (; position != children.end() && node->tag > (*position)->tag; ++position);

    children.insert(position, node);
    node->parent = identifier;
}

}

#endif
