#ifndef __NODE_H
#define __NODE_H

#include <memory>
#include <vector>

namespace treelib {

class Node
{
public:
    Node(std::string _tag, std::string _identifier, void* _data, std::string parent);

    std::string tag;
    std::string identifier;
    std::string parent;
    std::string path;
    void* data;

    bool is_matching_search;
    bool is_ancestor_of_matching_search;

    std::vector< std::shared_ptr< Node > > children;
    void add_child(std::shared_ptr< Node > node);

private:
    std::shared_ptr< Node > parent_node;
};

}

#endif
