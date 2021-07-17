#include "picker.h"
#include "tree/tree.h"

namespace picker {

Picker::Picker(treelib::Tree& tree) :
    _tree_keyboard_selector(tree),
    _tree(tree),
    _keyboard_reactor([&](void) { _tree_keyboard_selector.initialize(); },
                      [&](char c) { return _tree_keyboard_selector.char_pressed(c); })
{}

void Picker::run(void) {
    _keyboard_reactor.run();
}

}
