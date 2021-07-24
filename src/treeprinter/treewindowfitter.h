#ifndef __TREEWINDOWFITTER_H
#define __TREEWINDOWFITTER_H

#include "tree/tree.h"
#include "tree/node.h"

namespace treelib {

    int get_nr_levels_that_fit_in_window(Tree& tree, int window_size, std::shared_ptr<Node> selected);

    std::tuple<std::shared_ptr<Node>, int, int> choose_printed_subtree_root(
            Tree& tree,
            std::shared_ptr<Node> selection,
            int selection_depth,
            int window_height,
            std::shared_ptr<Node> current_printed_subtree_root
            );

}

#endif
