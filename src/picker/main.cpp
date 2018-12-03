#include "tree/tree.h"
#include "picker/picker.h"

int main(void) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
    tree.create_node("first", "first", "root_id", 10);
    tree.create_node("second", "second", "root_id", 10);
    tree.create_node("first_in_first", "first_in_first", "first", 10);

    picker::Picker treepicker(tree);
    treepicker.start();

    return 0;
}
