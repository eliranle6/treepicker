#ifndef __TREESELECTOR_H
#define __TREESELECTOR_H

#include "tree/tree.h"

namespace picker {

template <typename T>
class TreeSelector
{
public:
    using node_t = typename treelib::Tree<T>::node_t;

    TreeSelector(const treelib::Tree<T>& tree);

    node_t get_selection(void);

    void explore_children_of_selection(void);

private:
    const treelib::Tree<T>& _tree;
    node_t _selection;
};

template<typename T>
TreeSelector<T>::TreeSelector(const treelib::Tree<T>& tree) :
    _tree(tree),
    _selection(tree.get_root())
{
}

template<typename T>
typename TreeSelector<T>::node_t TreeSelector<T>::get_selection(void) {
    return _selection;
}

template<typename T>
void TreeSelector<T>::explore_children_of_selection(void) {
    if (_selection != nullptr) {
        if (_selection->children.size() > 0) {
            _selection = _selection->children[0];
        }
    }
}

}
#endif
