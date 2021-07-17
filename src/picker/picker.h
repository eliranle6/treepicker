#ifndef __PICKER_H
#define __PICKER_H

#include <assert.h>
#include <signal.h>
#include <thread>
#include <boost/asio.hpp>
#include "tree/tree.h"
#include "utils/guishell.h"
#include "picker/keyboardreactor.h"
#include "picker/tree_keyboard_selector.h"

namespace picker {

class Picker {
public:
    Picker(treelib::Tree& tree);

    void run(void);

private:
    TreeKeyboardSelector _tree_keyboard_selector;
    treelib::Tree _tree;
    KeyboardReactor _keyboard_reactor;
};

} // namespace picker

#endif
