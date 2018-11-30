#include <iostream>
#include "menu.h"

enum {
    KEYCODE_CTRL_C = 3,
    KEYCODE_CTRL_J = 10,
    KEYCODE_CTRL_K = 11,
    KEYCODE_CTRL_U = 21,
    KEYCODE_CTRL_W = 23,
    KEYCODE_BACKSPACE = 127
};

enum {
    KEYCODE_DOWN = KEYCODE_CTRL_J,
    KEYCODE_UP = KEYCODE_CTRL_K,
    KEYCODE_BACK = KEYCODE_CTRL_C,
};

Menu::Menu(treelib::Tree<int> tree):
    _tree(tree),
    _out(guishell::GuiShell::get_instance()),
    _search_keyword()
{
}

void Menu::char_pressed(char c)
{
    // _tree.get_root()->tag += std::to_string((int)c);
    switch(c) {
        case KEYCODE_DOWN:
            break;
        case KEYCODE_UP:
            break;
        case KEYCODE_BACKSPACE:
            if (not _search_keyword.empty()) {
                _search_keyword = _search_keyword.substr(0, _search_keyword.size() - 1);
            }
            break;
        case KEYCODE_CTRL_W:
        case KEYCODE_CTRL_U:
                _search_keyword = "";
                break;
        default:
            _search_keyword += c;
    };
    _tree.search(_search_keyword);
    _print_tree();
}

void Menu::_print_tree(void) {
    _out.clear();
    _out << _tree;
    if (not _search_keyword.empty()) {
        _out << std::endl << "Search: " + _search_keyword;
    }
    _out.refresh();
}
