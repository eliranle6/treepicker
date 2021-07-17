#ifndef __PRE_PRINT_TREE_ANALYSIS
#define __PRE_PRINT_TREE_ANALYSIS

#include <map>
#include "tree/node.h"

namespace treelib {

class Tree;

struct TreeAnalysisInfo {
    std::map<std::string, bool> m_node_to_next_sibling_existence;
    int m_max_depth;

    TreeAnalysisInfo(int max_depth, std::map<std::string, bool> node_to_next_sibling_existence) :
        m_node_to_next_sibling_existence(node_to_next_sibling_existence),
        m_max_depth(max_depth)
    {
    }
};

TreeAnalysisInfo analyze_tree_for_printing(std::shared_ptr< Node > root);

}

#endif
