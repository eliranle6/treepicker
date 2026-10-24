#include "tree/tree.h"

namespace treelib {

Tree::CreateResult Tree::try_create_node(const std::string& tag, const std::string& identifier,
                                         const std::string& parent, node_t& node) {
    // Each map lookup compares strings, so look each id up once
    const bool is_root_node_creation = root == nullptr;
    std::shared_ptr< Node > parent_node;
    if (not is_root_node_creation) {
        parent_node = get_node(parent);
        if (parent_node == nullptr) {
            return CreateResult::NO_PARENT;
        }
    }
    auto inserted = node_map.emplace(identifier, nullptr);
    if (not inserted.second) {
        node = inserted.first->second;
        return CreateResult::EXISTS;
    }

    node = std::make_shared< Node >(tag, identifier, parent);
    if (is_root_node_creation) {
        root = node;
    } else {
        // Set as child of designated parent
        parent_node->add_child(node);
    }
    inserted.first->second = node;
    return CreateResult::CREATED;
}

std::shared_ptr< Node >Tree::create_node(const std::string& tag, const std::string& identifier,
                                         const std::string& parent) {
    node_t node;
    switch (try_create_node(tag, identifier, parent, node)) {
        case CreateResult::NO_PARENT:
            throw std::invalid_argument("Parent node does not exist");
        case CreateResult::EXISTS:
            throw std::invalid_argument("Duplicate node");
        case CreateResult::CREATED:
            break;
    }
    return node;
}

std::shared_ptr< Node > Tree::get_or_create_node(const std::string& tag,
                                                 const std::string& identifier,
                                                 const std::string& parent) {
    node_t node;
    const auto result = try_create_node(tag, identifier, parent, node);
    return result == CreateResult::NO_PARENT ? nullptr : node;
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
