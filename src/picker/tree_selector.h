#ifndef __TREESELECTOR_H
#define __TREESELECTOR_H

#include "tree/tree.h"

namespace picker {

template <typename T>
class TreeSelector
{
public:
    using node_t = typename treelib::Tree<T>::node_t;

    TreeSelector(treelib::Tree<T>& tree);

    node_t get_selection(void);

    void explore_children_of_selection(void);
    void move_to_next(void);
    void move_to_prev(void);
    void move_one_up(void);

private:
    treelib::Tree<T>& _tree;
    node_t _selection;

    void _advance_selection_at_same_tree_level(int direction);
};

template<typename T>
TreeSelector<T>::TreeSelector(treelib::Tree<T>& tree) :
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
    if (nullptr == _selection) {
        return;
    }
    if (_selection->children.size() > 0) {
        _selection = _selection->children[0];
    }
}

template<typename T>
void TreeSelector<T>::move_one_up(void) {
    if (nullptr == _selection) {
        return;
    }
    auto parent = _tree.get_node(_selection->parent);
    if (parent != nullptr) {
        _selection = parent;
    }
}

template<typename T>
void TreeSelector<T>::move_to_next(void) {
    _advance_selection_at_same_tree_level(1);
}

template<typename T>
void TreeSelector<T>::move_to_prev(void) {
    _advance_selection_at_same_tree_level(-1);
}

template<typename T>
void TreeSelector<T>::_advance_selection_at_same_tree_level(int difference) {
    if (nullptr == _selection) {
        return;
    }
    if (_tree.get_root() == _selection) {
        return;
    }
    node_t parent = _tree.get_node(_selection->parent);
    if (parent.get() == nullptr) {
        return;
    }
    auto& children = parent->children;
    auto position = children.begin();
    // Find the index of the current selection in the array of paren'ts children
    // TODO: optimize this (to something better than O(n))
    for (; position != children.end() &&
        _selection->identifier != (*position)->identifier; ++position);
    assert(position != children.end());

    // Advance
    int direction = 0;
    if (difference > 0) {
        direction = 1;
    } else if (difference < 0) {
        direction = -1;
    }
    for (int count = 0; count < abs(difference); ++count) {
        auto target = position + direction;
        if (target == children.end()) {
            break;
        } else if (position == children.begin() and direction == -1) {
            break;
        } else {
            position = target;
        }
    }

    // Set selection
    _selection = *position;
}

}

#endif