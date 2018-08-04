#ifndef __TREELIB_H
#define __TREELIB_H

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <queue>
#include <iostream>
#include <stdexcept>
#include "node.h"
#include "treeprinter.h"

namespace treelib {

template <typename T>
class Tree
{
public:
    // Today i learned: nullptr
    Tree(void) : root(nullptr) {}
    // Today i learned: shared_ptr
    std::shared_ptr< Node<T> > create_node(std::string tag, std::string identifier, std::string parent, T data);
    std::shared_ptr< Node<T> > get_node(std::string identifier);
    std::shared_ptr< Node<T> > get_root(void) const { return root; }
    const std::vector< std::shared_ptr < Node<T> > > & children(std::string identifier);

    // Print tree
    template <typename U>
    friend std::ostream& operator << (std::ostream &os, const Tree<U> &tree);

protected:
    std::map<std::string, std::shared_ptr< Node<T> > > node_map;
    std::shared_ptr< Node<T> > root;
};

template <typename T>
std::shared_ptr< Node<T> >Tree<T>::create_node(std::string tag, std::string identifier,
                                               std::string parent, T data) {
    if (node_map.count(identifier)) {
        std::cout << "Duplicate node";
        throw std::invalid_argument("Duplicate node");
    }
    const bool is_root_node_creation = root == nullptr;

    auto nodeptr = std::make_shared< Node<T> >(tag, identifier, data, parent);
    if (is_root_node_creation) {
    // Set as child of designated parent
        root = nodeptr;
    } else {
        auto parent_node = get_node(parent);
        if (parent_node == nullptr) {
            throw std::invalid_argument("Parent node does not exist");
        }
        parent_node->add_child(nodeptr);
    }
    node_map[identifier] =  nodeptr;
    return node_map[identifier];
}

template <typename T>
std::shared_ptr< Node<T> > Tree<T>::get_node(std::string identifier) {
    if (node_map.count(identifier))
        return node_map[identifier];
    return nullptr;
}

template <typename T>
const std::vector< std::shared_ptr< Node<T> > > & Tree<T>::children(std::string identifier) {
    return node_map[identifier]->children;
}

template <typename T>
std::ostream& operator << (std::ostream &os, const Tree<T> &tree) {
    TreePrinter<T> tree_printer;
    tree_printer.print(os, tree);
    return os;
}

} // namespace treelib

#endif
