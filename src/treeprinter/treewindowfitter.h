#ifndef __TREEWINDOWFITTER_H
#define __TREEWINDOWFITTER_H

#include "tree/tree.h"
#include "tree/node.h"

class TreeWindowFitter
{
public:
    int get_nr_levels_that_fit_in_window(treelib::Tree tree,
                                         int window_size,
                                         std::shared_ptr<treelib::Node> selected);
};

#endif
