#ifndef __TREELIB_H
#define __TREELIB_H

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <iostream>
#include "node.h"

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
    std::vector< Node<T> * > & children(std::string identifier);
    void print(void);

private:
    std::map<std::string, Node<T> * > node_map;
    Node<T> *root;
};

template <typename T>
Node<T>* Tree<T>::create_root_node(std::string tag, std::string identifier, T data) {
    if (root != nullptr) {
        std::cout << "A root node already exists.";
        throw "A root node already exists.";
    }
    root = create_node(tag, identifier, "", data);
    return root;
}

template <typename T>
Node<T>* Tree<T>::create_node(std::string tag, std::string identifier, std::string parent, T data) {
    if (node_map.count(identifier)) {
        std::cout << "Duplicate node";
        throw "Duplicate node";
    }
    const bool does_root_node_exist = root != nullptr;
    if (not does_root_node_exist and parent.length()) {
        std::cout << "Root node cannot have a parent";
        throw "Root node cannot have a parent";
    }
    Node<T> * node = new Node<T>(tag, identifier, data, "");
    node_map[identifier] = node;

    if (parent.length()) {
        Node<T> *parent_node = get_node(parent);
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

template <typename T>
void Tree<T>::print(void) {
    if (nullptr == root) {
        return;
    }
    std::stack< Node<T>* > bfs_queue;
    bfs_queue.push(root);
    while (not bfs_queue.empty()) {
        Node<T> *node = bfs_queue.top();
        bfs_queue.pop();
        std::cout << node->tag << std::endl;
        for (auto child : node->children) {
                bfs_queue.push(child);
        }
    }
}

} // namespace treelib

#endif
