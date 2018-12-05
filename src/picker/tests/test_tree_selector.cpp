#include "gtest/gtest.h"

#include "picker/tree_selector.h"
#include "tree/tests/common.h"


TEST(tree_selector, default_selection) {
    auto tree = get_simple_tree();
    picker::TreeSelector tree_selector(tree);
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

