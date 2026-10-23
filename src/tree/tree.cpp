#include "tree/tree.h"

namespace treelib {

std::shared_ptr< Node >Tree::create_node(const std::string& tag, const std::string& identifier,
                                         const std::string& parent, void* data) {
    // Each map lookup compares strings, so look each id up once
    const bool is_root_node_creation = root == nullptr;
    std::shared_ptr< Node > parent_node;
    if (not is_root_node_creation) {
        parent_node = get_node(parent);
        if (parent_node == nullptr) {
            throw std::invalid_argument("Parent node does not exist");
        }
    }
    auto inserted = node_map.emplace(identifier, nullptr);
    if (not inserted.second) {
        throw std::invalid_argument("Duplicate node");
    }

    auto nodeptr = std::make_shared< Node >(tag, identifier, data, parent);
    if (is_root_node_creation) {
        root = nodeptr;
        nodeptr->path = std::string("/") + tag;
    } else {
        // Set as child of designated parent
        nodeptr->path = parent_node->path + std::string("/") + tag;
        parent_node->add_child(nodeptr);
    }
    inserted.first->second = nodeptr;
    return nodeptr;
}

std::shared_ptr< Node > Tree::get_node(const std::string& identifier) {
    auto found = node_map.find(identifier);
    return found == node_map.end() ? nullptr : found->second;
}

const std::vector< std::shared_ptr< Node > > & Tree::children(const std::string& identifier) {
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
