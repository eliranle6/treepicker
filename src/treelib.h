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
#include "treeprinter/treeprinter.h"

namespace treelib {

template <typename T>
class Tree
{
public:
    // Today i learned: nullptr
    Tree(void) : root(nullptr) {}
    Node<T>* create_root_node(std::string tag, std::string identifier, T data);
    Node<T>* create_node(std::string tag, std::string identifier, std::string parent, T data);
    Node<T>* get_node(std::string identifier);
    Node<T>* get_root(void) const { return root; }
    std::vector< Node<T> * > & children(std::string identifier);

    // Print tree
    template<T>
    friend std::ostream& operator << (std::ostream &os, const Tree<T> &tree);

private:

    std::map<std::string, Node<T> * > node_map;
    Node<T> *root;
};

template <typename T>
Node<T>* Tree<T>::create_root_node(std::string tag, std::string identifier, T data) {
    if (root != nullptr) {
        std::cout << "A root node already exists.";
        throw std::invalid_argument("A root node already exists.");
    }
    root = create_node(tag, identifier, "", data);
    return root;
}

template <typename T>
Node<T>* Tree<T>::create_node(std::string tag, std::string identifier, std::string parent, T data) {
    if (node_map.count(identifier)) {
        std::cout << "Duplicate node";
        throw std::invalid_argument("Duplicate node");
    }
    const bool does_root_node_exist = root != nullptr;
    if (not does_root_node_exist and parent.length()) {
        std::cout << "Root node cannot have a parent";
        throw std::invalid_argument("Root node cannot have a parent");
    }
    auto node = new Node<T>(tag, identifier, data, parent);
    node_map[identifier] = node;

    if (parent.length()) {
        auto *parent_node = get_node(parent);
        parent_node->add_child(node);
    }
    return node;
}

template <typename T>
Node<T>* Tree<T>::get_node(std::string identifier) {
    if (node_map.count(identifier))
        return node_map[identifier];
    return nullptr;
}

template <typename T>
std::vector< Node<T> * > & Tree<T>::children(std::string identifier) {
    return node_map[identifier]->children;
}

std::ostream& operator << (std::ostream &os, const Tree<int> &tree) {
    TreePrinter<int> tree_printer;
    tree_printer.print(os, tree);
    return os;
}

} // namespace treelib

#endif
