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
    node_t create_node(const std::string& tag, const std::string& identifier,
                       const std::string& parent);

    // Like create_node(), but if the id is already in the tree, returns that node instead of
    // throwing, and if the parent isn't, returns nullptr (adding nothing) instead of throwing
    node_t get_or_create_node(const std::string& tag, const std::string& identifier,
                              const std::string& parent);
    node_t get_node(const std::string& identifier);
    const node_t& get_root(void) const { return root; }

    // Print tree
    friend std::ostream& operator << (std::ostream &os, const Tree &tree);

    int get_node_depth(std::shared_ptr<Node> node);
    int get_nr_nodes(void) { return node_map.size(); };

protected:
    std::map<std::string, node_t > node_map;

private:
    enum class CreateResult { CREATED, EXISTS, NO_PARENT };
    CreateResult try_create_node(const std::string& tag, const std::string& identifier,
                                 const std::string& parent, node_t& node);
    node_t root;
};

} // namespace treelib

#endif
