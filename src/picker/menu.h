#ifndef MENU_H__
#define MENU_H__

#include "tree/tree.h"
#include "guishell/guishell.h"

namespace picker {

template<typename T>
class Menu
{
public:
    Menu(treelib::Tree<T>& tree);

    bool char_pressed(char c);

    void _print_tree(void);

private:
    treelib::Tree<T>& _tree;
    guishell::GuiShell& _out;
    std::string _search_keyword;
};

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

template<typename T>
Menu<T>::Menu(treelib::Tree<T>& tree):
    _tree(tree),
    _out(guishell::GuiShell::get_instance()),
    _search_keyword()
{
}

template<typename T>
bool Menu<T>::char_pressed(char c)
{
    bool is_finished = false;

    // _tree.get_root()->tag += std::to_string((int)c);
    switch(c) {
        case KEYCODE_DOWN:
            break;
        case KEYCODE_UP:
            break;
        case KEYCODE_BACK:
            is_finished = true;
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
    _tree.is_there_an_ongoing_search = not _search_keyword.empty();
    if (_tree.is_there_an_ongoing_search) {
        _tree.search(_search_keyword);
    }
    _print_tree();

    return is_finished;
}

template<typename T>
void Menu<T>::_print_tree(void) {
    _out.clear();
    _out << _tree;
    if (_tree.is_there_an_ongoing_search) {
        _out << std::endl << "Search: " + _search_keyword;
    }
    _out.refresh();
}

} // namespace picker

#endif
