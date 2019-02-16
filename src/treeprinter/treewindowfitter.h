#ifndef __TREEWINDOWFITTER_H
#define __TREEWINDOWFITTER_H

#include "tree/tree.h"
#include "tree/node.h"

template<typename T>
class TreeWindowFitter
{
public:
    treelib::Tree<T> fit_tree_to_window(treelib::Tree<T> tree,
                                        int window_size,
                                        std::shared_ptr<treelib::Node<T>> selected);

private:
    std::shared_ptr<treelib::Node<T>>
            choose_printed_tree_root(treelib::Tree<T> tree,
                                    int window_size,
                                    std::shared_ptr<treelib::Node<T>> selected);
};

template<typename T>
treelib::Tree<T> TreeWindowFitter<T>::fit_tree_to_window(treelib::Tree<T> tree,
        int window_size,
        std::shared_ptr<treelib::Node<T>> selected) {
    auto root = tree.get_root();
    auto printed_tree_root = choose_printed_tree_root(tree, window_size, selected);

    // TODO: Uncomment the line below once get_subtree_from_node is implemented, instead of
    // the other line (which uses the same tree)
    // auto printed_tree = get_subtree_from_node(printed_tree_root);
    auto printed_tree = tree;
    return printed_tree;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreeWindowFitter<T>::choose_printed_tree_root(
        treelib::Tree<T> tree,
        int window_size,
        std::shared_ptr<treelib::Node<T>> selected) {
    auto root = tree.get_root();

    std::shared_ptr<treelib::Node<T>> result;
    if (selected == root) {
        result = root;
    } else if (selected == nullptr) {
        result = nullptr;
    } else if (root == nullptr) {
        result = nullptr;
    } else {
        result = tree.get_node(selected->parent);
    }
    return result;
}

#endif
