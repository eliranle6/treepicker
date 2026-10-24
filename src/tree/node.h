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

    bool is_matching_search;
    bool is_ancestor_of_matching_search;
    // Whether it matches an ongoing search's (non-empty) keyword, to be shown highlighted
    bool is_search_highlighted;

    // Whether the user picked this node (when picking several). A node with children is picked
    // when all of them are, and partly picked when only some of them are (picked or partly).
    bool is_picked;
    bool is_partly_picked;

    // What the node is, if known: 'f' a file, 'd' a dir, 0 unknown (e.g. a tree of paths without
    // types)
    char kind;

    std::vector< std::shared_ptr< Node > > children;
    void add_child(std::shared_ptr< Node > node);

    // Whether an ongoing search shows this node (it, or a node under it, matches)
    bool is_search_visible(void) const {
        return is_matching_search or is_ancestor_of_matching_search;
    }
};

}

#endif
