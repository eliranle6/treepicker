#include "tree/tree.h"
#include "picker/picker.h"

int main(void) {
    treelib::Tree tree;
    tree.create_node("People", "people", "", nullptr);
        tree.create_node("Dumb people", "dumb", "people", nullptr);
            tree.create_node("Rich dumb people", "rich-dumb", "dumb", nullptr);
            tree.create_node("Poor dumb people", "poor-dumb", "dumb", nullptr);
        tree.create_node("Smart people", "smart", "people", nullptr);
            tree.create_node("Rich smart people", "rich-smart", "smart", nullptr);
            tree.create_node("Poor smart people", "poor-smart", "smart", nullptr);

    picker::Picker treepicker(tree);
    treepicker.run();

    return 0;
}
