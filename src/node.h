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
    T data;
    std::vector< std::shared_ptr< Node<T> > > children;
    void add_child(std::shared_ptr< Node<T> > node);

private:
    std::shared_ptr< Node<T> > parent_node;
};

template <class T>
Node<T>::Node(std::string _tag, std::string _identifier, T _data, std::string _parent) {
    tag = _tag;
    identifier = _identifier;
    data = _data;
    parent = _parent;
}

template <class T>
void Node<T>::add_child(std::shared_ptr< Node<T> > node) {
    children.insert(children.end(), node);
}

}

#endif
