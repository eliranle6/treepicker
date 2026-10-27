#ifndef __NODE_H
#define __NODE_H

#include <memory>
#include <string>
#include <vector>

namespace treelib {

class Node
{
public:
    Node(std::string _tag, std::string _identifier, std::string parent);

    std::string tag;
    std::string identifier;
    std::string parent;

    // Whether an ongoing search shows this node: it matches, or a node under it does (or the
    // search hides nothing)
    bool is_search_visible;
    // Whether it matches an ongoing search's (non-empty) keyword, to be shown highlighted
    bool is_search_match;

    // Whether the user picked this node (when picking several). A node with children is picked
    // when all of them are, and partly picked when only some of them are (picked or partly).
    bool is_picked;
    bool is_partly_picked;

    // What the node is, if known: 'f' a file, 'd' a dir, 0 unknown (e.g. a tree of paths without
    // types)
    char kind;

    // Whether children holds all of the node's children. A tree that loads nodes lazily (see
    // Tree::load_children()) adds a node's children only when they're needed.
    bool are_children_loaded;

    std::vector< std::shared_ptr< Node > > children;
    void add_child(std::shared_ptr< Node > node);
};

}

#endif
