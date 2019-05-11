#ifndef __PICKER_H
#define __PICKER_H

#include <assert.h>
#include <signal.h>
#include <thread>
#include <boost/asio.hpp>
#include "tree/tree.h"
#include "utils/guishell.h"
#include "tree_keyboard_selector.h"
#include "picker/keyboardreactor.h"

namespace picker {

template<typename T>
class Picker {
public:
    Picker(treelib::Tree<T>& tree);

    void run(void);

private:
    TreeKeyboardSelector<T> _tree_keyboard_selector;
    treelib::Tree<T>& _tree;
    KeyboardReactor<T> _keyboard_reactor;
};

template<typename T>
Picker<T>::Picker(treelib::Tree<T>& tree) :
    _tree_keyboard_selector(tree),
    _tree(tree),
    _keyboard_reactor([&](void) { _tree_keyboard_selector.initialize(); },
                      [&](char c) { return _tree_keyboard_selector.char_pressed(c); })
{}

template<typename T>
void Picker<T>::run(void) {
    _keyboard_reactor.run();
}

} // namespace picker

#endif
