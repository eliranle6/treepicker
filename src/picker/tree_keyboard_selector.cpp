#include <algorithm>
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

bool TreeKeyboardSelector::char_pressed(int c)
{
    bool is_finished = false;

    if (_mode == mode::MODE_NAVIGATION) {
        switch(c) {
            case KEYCODE_UP:
            case KEY_UP:
                _tree_selector.move_to_prev();
                print_tree();
                break;
            case KEYCODE_DOWN:
            case KEY_DOWN:
                _tree_selector.move_to_next();
                print_tree();
                break;
            case KEYCODE_PAGE_DOWN:
            case KEY_NPAGE:
                // Moving relies on the tree's layout (what's printed after the selection), so
                // lay it out again after each move, but draw only once at the end
                for (int i = 0; i < PAGE_SIZE; ++i) {
                    _tree_selector.move_to_next();
                    layout_tree();
                }
                print_tree();
                break;
            case KEYCODE_PAGE_UP:
            case KEY_PPAGE:
                {
                auto first_node = _tree_printer.get_first_printed_node();
                for (int i = 0; i < PAGE_SIZE && _tree_selector.get_selection() != first_node; ++i) {
                    _tree_selector.move_to_prev();
                    layout_tree();
                }
                print_tree();
                }
                break;
            case KEYCODE_RIGHT:
            case KEY_RIGHT:
                _tree_selector.explore_children_of_selection();
                print_tree();
                break;
            case KEYCODE_LEFT:
            case KEY_LEFT:
                _tree_selector.move_one_up();
                print_tree();
                break;
            case KEYCODE_BACK:
            case KEYCODE_QUIT:
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
            case KEY_BACKSPACE:
                if (_search_keyword.empty()) {
                    _mode = mode::MODE_NAVIGATION;
                    curs_set(0);
                } else {
                    _search_keyword = _search_keyword.substr(0, _search_keyword.size() - 1);
                    search();
                }
                print_tree();
                break;
            case KEYCODE_CTRL_W:
            case KEYCODE_CTRL_U:
                if (_mode == mode::MODE_EDIT_SEARCH) {
                    _search_keyword = "";
                }
                search();
                print_tree();
                break;
            case KEYCODE_CANCEL_SEARCH:
                // Like in vim: drop the keyword (unlike Enter, which keeps filtering by it)
                _search_keyword = "";
                search();
                _mode = mode::MODE_NAVIGATION;
                curs_set(0);
                print_tree();
                break;
            case KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE:
            case KEY_ENTER:
                _mode = mode::MODE_NAVIGATION;
                curs_set(0);
                print_tree();
                break;
            default:
                // Only text goes into the keyword: no control keys, and no curses codes
                // (e.g. KEY_RESIZE), which are bigger than a byte. Bytes of UTF-8 characters
                // (128-255) are text too.
                const bool is_text = c >= ' ' and c <= 0xff and c != KEYCODE_BACKSPACE;
                if (is_text) {
                    _search_keyword += static_cast<char>(c);
                    search();
                }
                print_tree();
        }
    }

    return is_finished;
}

// Print text on a row of the screen, over whatever was there, cut at the screen's edge. With
// fill, the rest of the row is filled with spaces (to show the row's background color).
static void print_line(int row, const std::string& text, int window_width, bool fill) {
    // The last column stays empty, like in NCursesSink::write
    const int width = std::max(0, window_width - 1);
    move(row, 0);
    clrtoeol();
    // Not printw: the text may hold a file name, which may contain '%'
    const std::size_t nr_bytes = guishell::nr_bytes_that_fit(text.data(), text.size(), width);
    addnstr(text.data(), nr_bytes);
    if (fill) {
        for (int i = guishell::display_width(text.substr(0, nr_bytes)); i < width; ++i) {
            addch(' ');
        }
    }
}

void TreeKeyboardSelector::print_tree(void) {
    _out.clear();
    const int window_width = getmaxx(stdscr);
    const Layout layout = get_layout();

    // Print tree
    if (layout.tree_height > 0) {
        // The screen's last column stays empty (see NCursesSink::write)
        _tree_printer.print(_out, not _search_keyword.empty(), _tree_selector.get_selection(),
                            layout.tree_height, window_width - 1);
    }
    _out.flush();

    // Print stats line
    if (layout.stats_row >= 0) {
        std::string stats_line = "Total: " + std::to_string(_tree.get_nr_nodes());
        if (is_search_line_shown()) {
            stats_line += std::string(", Matching: ") + std::to_string(_search.get_nr_matching());
        }
        stats_line += std::string(", selected: ") +
            guishell::printable(_tree_selector.get_selection()->tag);
        attron(COLOR_PAIR(::guishell::BLACK_ON_WHITE));
        print_line(layout.stats_row, stats_line, window_width, true);
        attroff(COLOR_PAIR(::guishell::BLACK_ON_WHITE));
    }

    // Print search line (last, so that the cursor stays at its end while editing it)
    if (layout.search_row >= 0) {
        print_line(layout.search_row, "/" + _search_keyword, window_width, false);
    }
    refresh();
}

bool TreeKeyboardSelector::is_search_line_shown(void) {
    return not _search_keyword.empty() or _mode == mode::MODE_EDIT_SEARCH;
}

TreeKeyboardSelector::Layout TreeKeyboardSelector::get_layout(void) {
    Layout layout;
    int nr_rows_left = getmaxy(stdscr);

    // The search line, at the bottom, only while there's a search
    layout.search_row = -1;
    if (is_search_line_shown() and nr_rows_left > 0) {
        layout.search_row = --nr_rows_left;
    }

    // The stats line above it, unless the tree would then be too short to show the selection
    layout.stats_row = -1;
    if (nr_rows_left - 1 >= MIN_TREE_HEIGHT) {
        layout.stats_row = --nr_rows_left;
    }

    layout.tree_height = nr_rows_left;
    return layout;
}

void TreeKeyboardSelector::layout_tree(void) {
    const int tree_max_height = get_layout().tree_height;
    if (tree_max_height > 0) {
        _tree_printer.scan_nodes_to_print(_out, not _search_keyword.empty(),
                                          _tree_selector.get_selection(), tree_max_height);
    }
}

void TreeKeyboardSelector::search(void) {
    _search.search(_search_keyword);
    // The new filter may hide the selected node
    _tree_selector.select_visible_ancestor();
}

void TreeKeyboardSelector::initialize(void) {
    print_tree();
}

} // namespace picker
