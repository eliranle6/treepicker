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

namespace treelib {

class Tree
{
public:
    using node_t = std::shared_ptr< Node >;

    // Today i learned: nullptr
    Tree(void) :
        root(nullptr)
    {}
    // Today i learned: shared_ptr
    node_t create_node(std::string tag, std::string identifier, std::string parent, void* data);
    node_t get_node(std::string identifier);
    node_t get_root(void) const { return root; }
    const std::vector<node_t> & children(std::string identifier);

    // Print tree
    friend std::ostream& operator << (std::ostream &os, const Tree &tree);

    int get_node_depth(std::shared_ptr<Node> node);
    int get_nr_nodes(void) { return node_map.size(); };

protected:
    std::map<std::string, node_t > node_map;
    node_t root;
};

} // namespace treelib

#endif
