#include "picker.h"
#include "tree/tree.h"

namespace picker {

Picker::Picker(treelib::Tree& tree, const PickerOptions& options) :
    _tree_keyboard_selector(tree, options),
    _tree(tree),
    _keyboard_reactor([&](void) { _tree_keyboard_selector.initialize(); },
                      [&](int c) { return _tree_keyboard_selector.char_pressed(c); })
{}

std::vector<std::shared_ptr<treelib::Node>> Picker::run(void) {
    _keyboard_reactor.run();
    return _tree_keyboard_selector.get_picked();
}

}
