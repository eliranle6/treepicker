#ifndef __TREELIB_H
#define __TREELIB_H

#include <assert.h>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <queue>
#include <iostream>
#include <stdexcept>
#include "node.h"
#include "search.h"

namespace treelib {

template <typename T>
class Tree
{
public:
    using node_t = std::shared_ptr< Node<T> >;

    // Today i learned: nullptr
    Tree(void) :
        _search(*this),
        root(nullptr)
    {}
    // Today i learned: shared_ptr
    node_t create_node(std::string tag, std::string identifier, std::string parent, T data);
    node_t get_node(std::string identifier);
    node_t get_root(void) const { return root; }
    const std::vector<node_t> & children(std::string identifier);

    // Print tree
    template <typename U>
    friend std::ostream& operator << (std::ostream &os, const Tree<U> &tree);

    void search(std::string pattern);
    int get_node_depth(std::shared_ptr<Node<T>> node);
    int get_nr_nodes(void) { return node_map.size(); };
    int get_nr_matching(void) { return _search.get_nr_matching(); };

    Search<T> _search;

protected:
    std::map<std::string, node_t > node_map;
    node_t root;
};

template <typename T>
std::shared_ptr< Node<T> >Tree<T>::create_node(std::string tag, std::string identifier,
                                               std::string parent, T data) {
    if (node_map.count(identifier)) {
        throw std::invalid_argument("Duplicate node");
    }
    const bool is_root_node_creation = root == nullptr;

    auto nodeptr = std::make_shared< Node<T> >(tag, identifier, data, parent);
    if (is_root_node_creation) {
    // Set as child of designated parent
        root = nodeptr;
        nodeptr->path = std::string("/") + tag;
    } else {
        auto parent_node = get_node(parent);
        if (parent_node == nullptr) {
            throw std::invalid_argument("Parent node does not exist");
        }
        nodeptr->path = parent_node->path + std::string("/") + tag;
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

template<typename T>
void Tree<T>::search(std::string pattern)
{
    _search.search(pattern);
}

template <typename T>
int Tree<T>::get_node_depth(std::shared_ptr<Node<T>> node) {
    int depth = 0;
    while (node != get_root()) {
        node = get_node(node->parent);
        assert (++depth <= 100);
    }
    return depth;
}

} // namespace treelib

#endif
