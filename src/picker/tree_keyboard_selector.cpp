#include <iostream>
#include "tree_keyboard_selector.h"
#include "dirtree/dirtree.h"

namespace picker {

TreeKeyboardSelector::TreeKeyboardSelector(treelib::Tree& tree):
    _tree(tree),
    _out(guishell::GuiShell::get_instance()),
    _search_keyword(),
    _tree_printer(tree),
    _tree_selector(tree, _tree_printer),
    _mode(mode::MODE_NAVIGATION),
    _search(tree)
{
    curs_set(0);
}

bool TreeKeyboardSelector::char_pressed(char c)
{
    bool is_finished = false;

    if (_mode == mode::MODE_NAVIGATION) {
        syslog(LOG_NOTICE, "%d", c);
        switch(c) {
            case KEYCODE_UP:
                _tree_selector.move_to_prev();
                print_tree();
                break;
            case KEYCODE_DOWN:
                _tree_selector.move_to_next();
                print_tree();
                break;
            case KEYCODE_PAGE_DOWN:
                {
                int i = 0;
                for (; i < 10; ++i) {
                    _tree_selector.move_to_next();
                    print_tree();
                }
                if (i == 0) {
                    print_tree();
                }
                }
                break;
            case KEYCODE_PAGE_UP:
                {
                auto first_node = _tree_printer.get_first_printed_node();
                int i = 0;
                for (; i < 10 && _tree_selector.get_selection() != first_node; ++i) {
                    _tree_selector.move_to_prev();
                    print_tree();
                }
                if (i == 0) {
                    print_tree();
                }
                }
                break;
            case KEYCODE_RIGHT:
                _tree_selector.explore_children_of_selection();
                print_tree();
                break;
            case KEYCODE_LEFT:
                _tree_selector.move_one_up();
                print_tree();
                break;
            case KEYCODE_BACK:
                is_finished = true;
                print_tree();
                break;
            case KEYCODE_START_SEARCH:
                if (_mode == mode::MODE_NAVIGATION) {
                    _mode = mode::MODE_EDIT_SEARCH;
                    curs_set(1);
                    print_tree();
                }
                break;
            default:
                print_tree();
        }
    } else if (_mode == mode::MODE_EDIT_SEARCH) {
        switch(c) {
            case KEYCODE_BACKSPACE:
                if (_search_keyword.empty()) {
                    _mode = mode::MODE_NAVIGATION;
                    curs_set(0);
                } else {
                    _search_keyword = _search_keyword.substr(0, _search_keyword.size() - 1);
                    _search.search(_search_keyword);
                }
                print_tree();
                break;
            case KEYCODE_CTRL_W:
            case KEYCODE_CTRL_U:
                if (_mode == mode::MODE_EDIT_SEARCH) {
                    _search_keyword = "";
                }
                _search.search(_search_keyword);
                print_tree();
                break;
            case KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE:
                _mode = mode::MODE_NAVIGATION;
                curs_set(0);
                print_tree();
                break;
            default:
                if (_mode == mode::MODE_EDIT_SEARCH)
                    _search_keyword += c;
                else {
                }
                _search.search(_search_keyword);
                print_tree();
        }
    }

    return is_finished;
}

void TreeKeyboardSelector::print_tree(void) {
    _out.clear();
    const bool should_nodes_be_search_filtered = not _search_keyword.empty();

    // Get window height
    int window_height = 0, window_width = 0;
    getmaxyx(stdscr, window_height, window_width);

    // Determine whether to print search line
    const bool should_search_bar_be_displayed =
        should_nodes_be_search_filtered or _mode == mode::MODE_EDIT_SEARCH;
    int tree_max_height = window_height - 2;
    int tree_height = 0;

    // Print tree
    if (tree_max_height > 0) {
        tree_height = _tree_printer.print(_out, should_nodes_be_search_filtered,
                            _tree_selector.get_selection(), tree_max_height);
    }
    for (int i = 0; i < tree_max_height - tree_height; ++i) {
      _out << std::endl;
    }

    _out.refresh();

    // Print Stats line
    if (subby != nullptr) {
        delwin(subby);
    }
    subby = newwin(1, window_width - 1, window_height - 2, 0);
    wbkgd(subby,COLOR_PAIR(::guishell::BLACK_ON_WHITE));
    std::string stats_line = "Total: " + std::to_string(_tree.get_nr_nodes());
    if (should_search_bar_be_displayed) {
        stats_line += std::string(", Matching: ") + std::to_string(_search.get_nr_matching());
    }
    mvwprintw(subby, 0, 0, stats_line.c_str());
    wrefresh(subby);

    // Print Search line
    delwin(subby);
    subby = newwin(1, window_width - 1, window_height - 1, 0);
    // Print search line
    if (should_search_bar_be_displayed) {
        std::string search_line = std::string("/") + _search_keyword;
        mvwprintw(subby, 0, 0, search_line.c_str());
    }
    wrefresh(subby);
}


void TreeKeyboardSelector::initialize(void) {
    print_tree();
}

} // namespace picker
