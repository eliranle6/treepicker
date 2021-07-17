//TODO generalize this module to a 'keyboard selector'
#ifndef TREE_KEYBOARD_SELECTOR_H__
#define TREE_KEYBOARD_SELECTOR_H__

#include <string>
#include "tree/tree.h"
#include "treeprinter/treeprinter.h"
#include "utils/guishell.h"
#include "tree_selector.h"
#include "tree/search.h"

namespace picker {

enum {
    KEYCODE_CTRL_C = 3,
    KEYCODE_CTRL_H = 8,
    KEYCODE_CTRL_J = 10,
    KEYCODE_CTRL_K = 11,
    KEYCODE_CTRL_L = 12,
    KEYCODE_CTRL_U = 21,
    KEYCODE_CTRL_W = 23,
    KEYCODE_BACKSPACE = 127,
    KEYCODE_SLASH = 47,
    KEYCODE_ENTER = 10,
    KEYCODE_H = 104,
    KEYCODE_J = 106,
    KEYCODE_K = 107,
    KEYCODE_L = 108
};

enum {
    KEYCODE_PREV = KEYCODE_K,
    KEYCODE_NEXT = KEYCODE_J,
    KEYCODE_RIGHT = KEYCODE_L,
    KEYCODE_LEFT = KEYCODE_H,
    KEYCODE_BACK = KEYCODE_CTRL_C,
    KEYCODE_START_SEARCH = KEYCODE_SLASH,
    KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE = KEYCODE_ENTER
};

enum mode {
    MODE_NAVIGATION,
    MODE_EDIT_SEARCH
};

class TreeKeyboardSelector
{
public:
    TreeKeyboardSelector(treelib::Tree& tree);

    bool char_pressed(char c);

    void print_tree(void);

    void initialize(void);

private:
    treelib::Tree& _tree;
    guishell::GuiShell& _out;
    std::string _search_keyword;
    treelib::TreePrinter _tree_printer;
    TreeSelector _tree_selector;
    mode _mode;
    WINDOW* subby;
    ::treelib::Search _search;
};

}

#endif
