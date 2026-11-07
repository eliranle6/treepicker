#ifndef TREE_SERACH_H
#define TREE_SERACH_H

#include <functional>

#include "tree.h"

namespace treelib {

class Search
{
public:
    Search(Tree &tree) : _tree(tree), _nr_matching(_tree.get_nr_nodes()) {};

    // Mark the nodes matching keyword (and their ancestors), so the others are hidden. Without
    // hide_non_matching, every node stays shown, and the matching ones are only highlighted.
    void search(std::string keyword, bool hide_non_matching = true);

    std::size_t get_nr_matching(void) { return _nr_matching; };

    // Show only the nodes without children that filter passes (and the dirs they're in), and of
    // them, a search's matches; from the next search() on. nullptr: no filter.
    void set_filter(std::function<bool(const Node&)> filter) { _filter = filter; }
    bool has_filter(void) const { return static_cast<bool>(_filter); }

private:
    Tree &_tree;

    std::size_t _nr_matching;
    std::function<bool(const Node&)> _filter;

    using node_t = std::shared_ptr< Node >;

    // Show the node's ancestors, which hide_non_matching hid
    void show_ancestors(Node* node);
};

} // namespace treelib

#endif
