#ifndef __PICKER_H
#define __PICKER_H

#include <memory>
#include <vector>
#include "tree/tree.h"
#include "utils/guishell.h"
#include "picker/keyboardreactor.h"
#include "picker/tree_keyboard_selector.h"

namespace picker {

class Picker {
public:
    // With is_multi, the user picks several nodes
    Picker(treelib::Tree& tree, bool is_multi = false);

    // Let the user pick, and return what they picked: empty if they quit
    std::vector<std::shared_ptr<treelib::Node>> run(void);

private:
    TreeKeyboardSelector _tree_keyboard_selector;
    treelib::Tree& _tree;
    KeyboardReactor _keyboard_reactor;
};

} // namespace picker

#endif
