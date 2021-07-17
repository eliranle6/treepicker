#ifndef TREE_SERACH_H
#define TREE_SERACH_H

#include "tree.h"

namespace treelib {

class Search
{
public:
    Search(Tree &tree) : _tree(tree) {};

    void search(std::string keyword);

    std::size_t get_nr_matching(void) { return _nr_matching; };

private:
    Tree &_tree;

    std::size_t _nr_matching;

    void mark_ancestors(std::string &identifier);

    using node_t = std::shared_ptr< Node >;
};

} // namespace treelib

#endif
