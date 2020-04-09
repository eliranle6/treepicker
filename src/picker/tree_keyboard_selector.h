#ifndef MENU_H__
#define MENU_H__

#include "tree/tree.h"
#include "treeprinter/treeprinter.h"
#include "utils/guishell.h"
#include "tree_selector.h"

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
    KEYCODE_ENTER = 13,
    KEYCODE_J = 106,
    KEYCODE_K = 107
};

enum {
    KEYCODE_DOWN = KEYCODE_CTRL_J,
    KEYCODE_UP = KEYCODE_CTRL_H,
    KEYCODE_PREV = KEYCODE_CTRL_K,
    KEYCODE_RIGHT = KEYCODE_CTRL_L,
    KEYCODE_BACK = KEYCODE_CTRL_C,
    KEYCODE_START_SEARCH = KEYCODE_SLASH,
    KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE = KEYCODE_ENTER
};

enum mode {
    MODE_NAVIGATION,
    MODE_EDIT_SEARCH
};

template<typename T>
class TreeKeyboardSelector
{
public:
    TreeKeyboardSelector(treelib::Tree<T>& tree);

    bool char_pressed(char c);

    void print_tree(void);

    void initialize(void);

private:
    treelib::Tree<T>& _tree;
    guishell::GuiShell& _out;
    std::string _search_keyword;
    treelib::TreePrinter<T> _tree_printer;
    TreeSelector<T> _tree_selector;
    mode _mode;
};


template<typename T>
TreeKeyboardSelector<T>::TreeKeyboardSelector(treelib::Tree<T>& tree):
    _tree(tree),
    _out(guishell::GuiShell::get_instance()),
    _search_keyword(),
    _tree_printer(tree),
    _tree_selector(tree, _tree_printer),
    _mode(mode::MODE_NAVIGATION)
{
}

template<typename T>
bool TreeKeyboardSelector<T>::char_pressed(char c)
{
    bool is_finished = false;

    switch(c) {
        case KEYCODE_DOWN:
            _tree_selector.move_to_next();
            break;
        case KEYCODE_J:
            _tree_selector.move_to_next_printed_node();
            break;
        case KEYCODE_K:
            _tree_selector.move_to_previously_printed_node();
            break;
        case KEYCODE_UP:
            _tree_selector.move_one_up();
            break;
        case KEYCODE_RIGHT:
            _tree_selector.explore_children_of_selection();
            break;
        case KEYCODE_PREV:
            _tree_selector.move_to_prev();
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
            if (_mode == mode::MODE_EDIT_SEARCH) {
                _search_keyword = "";
            }
            break;
        case KEYCODE_START_SEARCH:
            if (_mode == mode::MODE_NAVIGATION) {
                _search_keyword = "";
                _mode = mode::MODE_EDIT_SEARCH;
            }
            break;
        case KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE:
            _mode = mode::MODE_NAVIGATION;
            break;
        default:
            if (_mode == mode::MODE_EDIT_SEARCH)
                _search_keyword += c;
            else {
            }
    };
    _tree.search(_search_keyword);
    print_tree();

    return is_finished;
}

template<typename T>
void TreeKeyboardSelector<T>::print_tree(void) {
    _out.clear();
    const bool should_nodes_be_search_filtered = not _search_keyword.empty();

    // Get window height
    int window_height = 0;
    getmaxyx(stdscr, window_height, std::ignore);

    // Print
    _tree_printer.print(_out, should_nodes_be_search_filtered,
                        _tree_selector.get_selection(), window_height);
    const bool should_search_bar_be_displayed =
        should_nodes_be_search_filtered or _mode == mode::MODE_EDIT_SEARCH;
    if (should_search_bar_be_displayed) {
        _out << std::endl << "Search: " + _search_keyword;
    }
    _out.refresh();
}


template<typename T>
void TreeKeyboardSelector<T>::initialize(void) {
    print_tree();
}

} // namespace picker

#endif
