#include <algorithm>
#include <iostream>
#include <sstream>
#include "tree_keyboard_selector.h"
#include "picker/keys_help.h"
#include "pathtree/pathtree.h"

namespace picker {

TreeKeyboardSelector::TreeKeyboardSelector(treelib::Tree& tree, const PickerOptions& options):
    _tree(tree),
    _out(guishell::GuiShell::get_instance()),
    _search_keyword(),
    _applied_keyword(),
    _is_search_on_enter(false),
    _is_filtering(true),
    _tree_printer(tree),
    _tree_selector(tree, _tree_printer),
    _mode(mode::MODE_NAVIGATION),
    _search(tree),
    _options(options),
    _are_files_known(false)
{
    // Once, rather than on each redraw
    std::vector<treelib::Node*> dfs_stack;
    if (tree.get_root() != nullptr) {
        dfs_stack.push_back(tree.get_root().get());
    }
    while (not dfs_stack.empty() and not _are_files_known) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        _are_files_known = node->kind == 'f';
        for (auto& child : node->children) {
            dfs_stack.push_back(child.get());
        }
    }
    if (options.is_multi) {
        for (auto& id : options.initially_picked_ids) {
            _tree_selector.set_picked(_tree.get_node(id), true);
        }
        if (not options.initially_picked_ids.empty()) {
            _tree_selector.update_picked_dirs();
        }
    }
    if (options.is_first_child_selected_first) {
        _tree_selector.explore_children_of_selection();
    }
    if (not options.header.empty()) {
        std::istringstream header(options.header);
        for (std::string line; std::getline(header, line);) {
            _header_lines.push_back(line);
        }
    }
    curs_set(0);
}

bool TreeKeyboardSelector::char_pressed(int c)
{
    bool is_finished = false;

    // The help stays until a key is pressed, which only takes it away (a resize redraws it)
    if (_mode == mode::MODE_HELP) {
        if (c != KEY_RESIZE) {
            _mode = mode::MODE_NAVIGATION;
        }
        print_tree();
        return false;
    }

    if (_mode == mode::MODE_NAVIGATION) {
        for (auto& command : get_key_commands()) {
            for (auto& binding : command.bindings) {
                if (binding.first == c) {
                    is_finished = (this->*binding.second)();
                    break;
                }
            }
        }
        if (not is_finished) {
            print_tree();
        }
    } else if (_mode == mode::MODE_EDIT_SEARCH) {
        switch(c) {
            case KEYCODE_BACKSPACE:
            case KEY_BACKSPACE:
                if (_search_keyword.empty()) {
                    stop_editing_keyword();
                } else {
                    _search_keyword.pop_back();
                    keyword_changed();
                }
                print_tree();
                break;
            case KEYCODE_CTRL_W:
            case KEYCODE_CTRL_U:
                _search_keyword = "";
                keyword_changed();
                print_tree();
                break;
            case KEYCODE_ESCAPE:
                // Like in vim: drop the keyword (unlike Enter, which keeps filtering by it)
                _search_keyword = "";
                search();
                stop_editing_keyword();
                print_tree();
                break;
            case KEYCODE_ENTER:
            case KEY_ENTER:
                if (_is_search_on_enter) {
                    search();
                }
                stop_editing_keyword();
                print_tree();
                break;
            default:
                // Only text goes into the keyword: no control keys, and no curses codes
                // (e.g. KEY_RESIZE), which are bigger than a byte. Bytes of UTF-8 characters
                // (128-255) are text too.
                const bool is_text = c >= ' ' and c <= 0xff and c != KEYCODE_BACKSPACE;
                if (is_text) {
                    _search_keyword += static_cast<char>(c);
                    keyword_changed();
                }
                print_tree();
        }
    }

    return is_finished;
}

const std::vector<TreeKeyboardSelector::KeyCommand>& TreeKeyboardSelector::get_key_commands(void) {
    using T = TreeKeyboardSelector;
    // A function's static, so it's there for whoever needs it while statics are initialized
    // (cli's usage text)
    static const std::vector<KeyCommand> commands = {
        {"h/j/k/l, arrows", "move", {
            {KEYCODE_K, &T::move_to_prev}, {KEY_UP, &T::move_to_prev},
            {KEYCODE_J, &T::move_to_next}, {KEY_DOWN, &T::move_to_next},
            {KEYCODE_H, &T::move_one_up}, {KEY_LEFT, &T::move_one_up},
            {KEYCODE_L, &T::explore_children}, {KEY_RIGHT, &T::explore_children}}},
        {"g/G, Home/End", "first/last sibling", {
            {KEYCODE_LOWER_G, &T::move_to_first_sibling}, {KEY_HOME, &T::move_to_first_sibling},
            {KEYCODE_UPPER_G, &T::move_to_last_sibling}, {KEY_END, &T::move_to_last_sibling}}},
        {"Ctrl-D/Ctrl-U", "page down/up", {
            {KEYCODE_CTRL_D, &T::page_down}, {KEY_NPAGE, &T::page_down},
            {KEYCODE_CTRL_U, &T::page_up}, {KEY_PPAGE, &T::page_up}}},
        {"/", "search as you type (Enter keeps the filter, Esc drops it)", {
            {KEYCODE_SLASH, &T::start_search}}},
        {"Ctrl-/", "search when Enter is pressed", {
            {KEYCODE_CTRL_SLASH, &T::start_search_on_enter}}},
        {"Esc", "drop the search filter", {
            {KEYCODE_ESCAPE, &T::drop_search}}},
        {"f", "switch between showing only the matches and the\n"
              "whole tree with the matches in yellow", {
            {KEYCODE_F, &T::toggle_filter}}},
        {"Space", "pick or unpick (with -m)", {
            {KEYCODE_SPACE, &T::toggle_pick}}},
        {"Enter", "pick (with -m: finish)", {
            {KEYCODE_ENTER, &T::pick}, {KEY_ENTER, &T::pick}}},
        {"?", "show these keys", {
            {KEYCODE_QUESTION_MARK, &T::show_help}}},
        {"q, Ctrl-C", "quit", {
            {KEYCODE_Q, &T::quit}, {KEYCODE_CTRL_C, &T::quit}}},
    };
    return commands;
}

bool TreeKeyboardSelector::move_to_prev(void) {
    _tree_selector.move_to_prev();
    return false;
}

bool TreeKeyboardSelector::move_to_next(void) {
    _tree_selector.move_to_next();
    return false;
}

bool TreeKeyboardSelector::page_down(void) {
    // Moving relies on the tree's layout (what's printed after the selection), so lay it out
    // again after each move, but draw only once at the end
    for (int i = 0; i < PAGE_SIZE; ++i) {
        _tree_selector.move_to_next();
        layout_tree();
    }
    return false;
}

bool TreeKeyboardSelector::page_up(void) {
    auto first_node = _tree_printer.get_first_printed_node();
    for (int i = 0; i < PAGE_SIZE && _tree_selector.get_selection() != first_node; ++i) {
        _tree_selector.move_to_prev();
        layout_tree();
    }
    return false;
}

bool TreeKeyboardSelector::move_to_first_sibling(void) {
    _tree_selector.move_to_first_sibling();
    return false;
}

bool TreeKeyboardSelector::move_to_last_sibling(void) {
    _tree_selector.move_to_last_sibling();
    return false;
}

bool TreeKeyboardSelector::explore_children(void) {
    _tree_selector.explore_children_of_selection();
    return false;
}

bool TreeKeyboardSelector::move_one_up(void) {
    _tree_selector.move_one_up();
    return false;
}

bool TreeKeyboardSelector::pick(void) {
    if (not _options.is_multi) {
        if (_options.is_root_pickable or _tree_selector.get_selection() != _tree.get_root()) {
            _picked = {_tree_selector.get_selection()};
            return true;
        }
    } else if (_tree_selector.get_nr_picked() > 0) {
        _picked = _tree_selector.get_picked_nodes();
        return true;
    }
    return false;
}

bool TreeKeyboardSelector::toggle_pick(void) {
    if (_options.is_multi) {
        _tree_selector.toggle_pick_of_selection();
    }
    return false;
}

bool TreeKeyboardSelector::quit(void) {
    _picked.clear();
    return true;
}

bool TreeKeyboardSelector::drop_search(void) {
    // Like while editing the keyword: drop it, unfiltering the tree
    if (not _applied_keyword.empty()) {
        _search_keyword = "";
        search();
    }
    return false;
}

bool TreeKeyboardSelector::show_help(void) {
    _mode = mode::MODE_HELP;
    return false;
}

bool TreeKeyboardSelector::toggle_filter(void) {
    _is_filtering = not _is_filtering;
    if (not _applied_keyword.empty()) {
        search();
    }
    return false;
}

bool TreeKeyboardSelector::start_search(void) {
    _mode = mode::MODE_EDIT_SEARCH;
    curs_set(1);
    return false;
}

bool TreeKeyboardSelector::start_search_on_enter(void) {
    _is_search_on_enter = true;
    return start_search();
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

void TreeKeyboardSelector::print_help(void) {
    const int window_height = getmaxy(stdscr);
    const int window_width = getmaxx(stdscr);
    std::vector<std::string> lines = {"Keys (press any key to go back):", ""};
    std::istringstream keys(keys_help());
    for (std::string line; std::getline(keys, line);) {
        lines.push_back(line);
    }
    for (int row = 0; row < window_height and row < static_cast<int>(lines.size()); ++row) {
        print_line(row, lines[row], window_width, false);
    }
    refresh();
}

void TreeKeyboardSelector::print_tree(void) {
    _out.clear();
    if (_mode == mode::MODE_HELP) {
        print_help();
        return;
    }
    const int window_width = getmaxx(stdscr);
    const Layout layout = get_layout();

    // Print header
    for (int row = 0; row < layout.nr_header_rows; ++row) {
        print_line(row, guishell::printable(_header_lines[row]), window_width, false);
    }

    // Print tree, below it
    move(layout.nr_header_rows, 0);
    if (layout.tree_height > 0) {
        // The screen's last column stays empty (see NCursesSink::write)
        _tree_printer.print(_out, not _applied_keyword.empty(), _tree_selector.get_selection(),
                            layout.tree_height, window_width - 1);
    }
    _out.flush();

    // Print stats line
    if (layout.stats_row >= 0) {
        std::string stats_line = "Total: " + std::to_string(_tree.get_nr_nodes());
        if (not _applied_keyword.empty()) {
            stats_line += std::string(", Matching: ") + std::to_string(_search.get_nr_matching());
            if (not _is_filtering) {
                stats_line += " (highlighted)";
            }
        }
        if (_options.is_multi) {
            // Files, when the tree knows which nodes are files (else nodes, dirs too)
            stats_line += ", picked: " + (_are_files_known ?
                std::to_string(_tree_selector.get_nr_picked_files()) + " files" :
                std::to_string(_tree_selector.get_nr_picked()));
        }
        stats_line += std::string(", selected: ") +
            guishell::printable(_tree_selector.get_selection()->tag);
        // Where it is in its dir, which a long one shows only part of
        const auto place = _tree_selector.get_place_among_siblings(not _applied_keyword.empty());
        if (place.second > 1) {
            stats_line += " (" + std::to_string(place.first) + "/" +
                std::to_string(place.second) + ")";
        }
        // Room for the label, which is drawn over it
        const std::string label = _options.label.empty() ? "" :
            " " + guishell::printable(_options.label) + " ";
        if (not label.empty()) {
            stats_line = std::string(guishell::display_width(label) + 1, ' ') + stats_line;
        }
        // With a reminder of the help key at the right end, which the stats give way to
        static const std::string HELP_HINT = "?: help";
        const int width = window_width - 1;
        const int stats_width = width - guishell::display_width(HELP_HINT) - 1;
        if (stats_width > 0) {
            stats_line.resize(guishell::nr_bytes_that_fit(stats_line.data(), stats_line.size(),
                                                          stats_width));
            stats_line += std::string(stats_width + 1 - guishell::display_width(stats_line), ' ') +
                HELP_HINT;
        }
        attron(COLOR_PAIR(guishell::color_pair(guishell::BLACK_ON_WHITE)));
        print_line(layout.stats_row, stats_line, window_width, true);
        attroff(COLOR_PAIR(guishell::color_pair(guishell::BLACK_ON_WHITE)));
        if (not label.empty()) {
            const int pair = guishell::label_color_pair(_options.label_color);
            attron(COLOR_PAIR(pair));
            attron(A_BOLD);
            const int width = std::max(0, window_width - 1);
            mvaddnstr(layout.stats_row, 0, label.data(),
                      guishell::nr_bytes_that_fit(label.data(), label.size(), width));
            attroff(A_BOLD);
            attroff(COLOR_PAIR(pair));
        }
    }

    // Print search line (last, so that the cursor stays at its end while editing it)
    if (layout.search_row >= 0) {
        const bool is_waiting_for_enter = _mode == mode::MODE_EDIT_SEARCH and _is_search_on_enter;
        // With the hint after the keyword, the cursor wouldn't be at the keyword's end
        if (is_waiting_for_enter) {
            print_line(layout.search_row, "/" + _search_keyword + "    (Enter: search)",
                       window_width, false);
            move(layout.search_row, std::min(guishell::display_width("/" + _search_keyword),
                                             std::max(0, window_width - 2)));
        } else {
            print_line(layout.search_row, "/" + _search_keyword, window_width, false);
        }
    }
    refresh();
}

bool TreeKeyboardSelector::is_search_line_shown(void) {
    return not _applied_keyword.empty() or _mode == mode::MODE_EDIT_SEARCH;
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

    // The header on top, as much of it as leaves the tree enough room
    layout.nr_header_rows = std::min(static_cast<int>(_header_lines.size()),
                                     std::max(0, nr_rows_left - MIN_TREE_HEIGHT));
    nr_rows_left -= layout.nr_header_rows;

    layout.tree_height = nr_rows_left;
    return layout;
}

void TreeKeyboardSelector::layout_tree(void) {
    const int tree_max_height = get_layout().tree_height;
    if (tree_max_height > 0) {
        _tree_printer.layout(not _applied_keyword.empty(), _tree_selector.get_selection(),
                             tree_max_height);
    }
}

void TreeKeyboardSelector::keyword_changed(void) {
    if (not _is_search_on_enter) {
        search();
    }
}

void TreeKeyboardSelector::stop_editing_keyword(void) {
    // A keyword that waited for Enter, and didn't get it, isn't the one searched for
    _search_keyword = _applied_keyword;
    _is_search_on_enter = false;
    _mode = mode::MODE_NAVIGATION;
    curs_set(0);
}

void TreeKeyboardSelector::search(void) {
    _applied_keyword = _search_keyword;
    _search.search(_applied_keyword, _is_filtering);
    // The new filter may hide the selected node
    _tree_selector.select_visible_ancestor();
}

void TreeKeyboardSelector::initialize(void) {
    print_tree();
}

} // namespace picker
