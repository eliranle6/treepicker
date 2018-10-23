#ifndef TREE_SERACH_H
#define TREE_SERACH_H

#include "tree.h"

namespace treelib {

template<typename T>
class Tree;

template<typename T>
class Search
{
public:
    Search(Tree<T> &tree) : _tree(tree) {};

    void search(std::string &keyword);

private:
    Tree<T> &_tree;
};

template<typename T>
void Search<T>::search(std::string &keyword) {

}

} // namespace treelib

#endif
