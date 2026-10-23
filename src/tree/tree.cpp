#include "tree/tree.h"

namespace treelib {

std::shared_ptr< Node >Tree::create_node(std::string tag, std::string identifier,
                                               std::string parent, void* data) {
    if (node_map.count(identifier)) {
        throw std::invalid_argument("Duplicate node");
    }
    const bool is_root_node_creation = root == nullptr;

    auto nodeptr = std::make_shared< Node >(tag, identifier, data, parent);
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

std::shared_ptr< Node > Tree::get_node(std::string identifier) {
    if (node_map.count(identifier))
        return node_map[identifier];
    return nullptr;
}

const std::vector< std::shared_ptr< Node > > & Tree::children(std::string identifier) {
    // Not node_map[identifier]: for an unknown id it would add an empty node pointer, and then
    // dereference it
    auto node = get_node(identifier);
    if (node == nullptr) {
        throw std::invalid_argument("No such node: " + identifier);
    }
    return node->children;
}

int Tree::get_node_depth(std::shared_ptr<Node> node) {
    int depth = 0;
    // No depth limit, and no assert() around the ++: with NDEBUG the increment would vanish too
    while (node != get_root()) {
        node = get_node(node->parent);
        ++depth;
    }
    return depth;
}

}
