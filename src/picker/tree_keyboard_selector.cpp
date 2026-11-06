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
    _is_preview_shown(true),
    _tree_printer(tree),
    _tree_selector(tree, _tree_printer),
    _mode(mode::MODE_NAVIGATION),
    _search(tree),
    _options(options),
    _are_files_known(false),
    _expected_key(0),
    _range_start(nullptr)
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
    if (options.side_tree != nullptr and options.side_tree->get_root() != nullptr) {
        _side_printer.reset(new treelib::TreePrinter(*options.side_tree));
        _side_selector.reset(new TreeSelector(*options.side_tree, *_side_printer));
        if (not options.side_start_path.empty()) {
            const auto node = closest_in_side(options.side_start_path);
            if (node != nullptr) {
                _side_selector->select(node);
            }
            _is_side_focused = true;
        }
    }
    if (options.is_first_child_selected_first) {
        _tree_selector.explore_children_of_selection();
    }
    if (not options.selected_id.empty()) {
        _tree_selector.select(_tree.get_node(options.selected_id));
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

    // Moving between the windows, as vim's Ctrl-W: its key waits for the next one, which is only that
    if (_mode == mode::MODE_NAVIGATION and _is_window_key_pending) {
        _is_window_key_pending = false;
        if (c == KEYCODE_L or c == KEYCODE_CTRL_L or c == KEY_RIGHT) {
            focus_side();
        } else if (c == KEYCODE_H or c == KEYCODE_CTRL_H or c == KEY_LEFT) {
            _is_side_focused = false;
        } else if (c == KEYCODE_LOWER_W or c == KEYCODE_CTRL_W) {
            if (_is_side_focused) {
                _is_side_focused = false;
            } else {
                focus_side();
            }
        }
        print_tree();
        return false;
    }
    if (_mode == mode::MODE_NAVIGATION and c == KEYCODE_CTRL_W) {
        _is_window_key_pending = true;
        return false;
    }

    if (_mode == mode::MODE_NAVIGATION and _is_side_focused) {
        return side_char_pressed(c);
    }
    if (_mode == mode::MODE_NAVIGATION) {
        load_around_selection();
        if (c > 0 and c <= 0xff and _options.expect_keys.find(static_cast<char>(c)) != std::string::npos) {
            _expected_key = c;
            _picked = _options.is_multi ? _tree_selector.get_picked_nodes() :
                std::vector<std::shared_ptr<treelib::Node>>();
            return true;
        }
        // A key that's expected twice in a row: the first waits for the second; any other key
        // drops it
        const bool is_twice_key = c > 0 and c <= 0xff and
            _options.expect_twice_keys.find(static_cast<char>(c)) != std::string::npos;
        if (is_twice_key and _pending_twice_key != c) {
            _pending_twice_key = c;
            return false;
        }
        const int pending_twice_key = _pending_twice_key;
        _pending_twice_key = 0;
        if (is_twice_key and pending_twice_key == c) {
            end_range(true);
            _expected_key = c;
            _picked = _options.is_multi ? _tree_selector.get_picked_nodes() :
                std::vector<std::shared_ptr<treelib::Node>>();
            return true;
        }
        if (c > 0 and c <= 0xff and _notify and
                _options.notify_keys.find(static_cast<char>(c)) != std::string::npos) {
            _notify(c);
            print_tree();
            return false;
        }
        // Picking a range: Space and Enter end it (keeping it), as V does; Esc drops it
        if (_range_start != nullptr and (c == KEYCODE_SPACE or c == KEYCODE_ENTER or c == KEY_ENTER or
                                         c == KEYCODE_ESCAPE)) {
            end_range(c != KEYCODE_ESCAPE);
            if (c != KEYCODE_ENTER and c != KEY_ENTER) {
                print_tree();
                return false;
            }
        }
        for (auto& command : get_key_commands()) {
            for (auto& binding : command.bindings) {
                if (binding.first == c) {
                    is_finished = (this->*binding.second)();
                    break;
                }
            }
        }
        // The range follows the selection
        if (_range_start != nullptr and not is_finished) {
            layout_tree();
            pick_range();
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
        {"J/K", "next/previous node of the whole tree, collapsed dirs too", {
            {KEYCODE_UPPER_J, &T::move_to_next_in_tree}, {KEYCODE_UPPER_K, &T::move_to_prev_in_tree}}},
        {"g/G", "first/last line on the screen", {
            {KEYCODE_LOWER_G, &T::move_to_first_shown}, {KEYCODE_UPPER_G, &T::move_to_last_shown}}},
        {"Home/End", "first/last sibling", {
            {KEY_HOME, &T::move_to_first_sibling}, {KEY_END, &T::move_to_last_sibling}}},
        {"Ctrl-J/Ctrl-K", "next/previous sibling", {
            {KEYCODE_CTRL_J, &T::move_to_next_sibling}, {KEYCODE_CTRL_K, &T::move_to_prev_sibling}}},
        {"Ctrl-D/Ctrl-U", "page down/up", {
            {KEYCODE_CTRL_D, &T::page_down}, {KEY_NPAGE, &T::page_down},
            {KEYCODE_CTRL_U, &T::page_up}, {KEY_PPAGE, &T::page_up}}},
        {"/", "search as you type (Enter keeps the filter, Esc drops it)", {
            {KEYCODE_SLASH, &T::start_search}}},
        {"Ctrl-/", "search when Enter is pressed", {
            {KEYCODE_CTRL_SLASH, &T::start_search_on_enter}}},
        {"Esc, Ctrl-[", "drop the search filter (without one, with -m: unpick all)", {
            {KEYCODE_ESCAPE, &T::drop_search}}},
        {"f", "switch between showing only the matches and the\n"
              "whole tree with the matches in yellow", {
            {KEYCODE_F, &T::toggle_filter}}},
        {"p", "show or hide the preview column (with --preview)", {
            {KEYCODE_P, &T::toggle_preview}}},
        {"Ctrl-L/Ctrl-H", "move in the side tree (with --side), and back", {
            {KEYCODE_CTRL_L, &T::focus_side}}},
        {"Ctrl-W l/h", "the same, as vim's (Ctrl-W w switches)", {
            {KEYCODE_CTRL_W, &T::start_window_key}}},
        {"V", "pick a range (with -m): moving picks the lines from here\n"
              "to the selection (V, Space or Enter: done; Esc: unpick them)", {
            {KEYCODE_UPPER_V, &T::toggle_range}}},
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

void TreeKeyboardSelector::move_skipping(void (TreeSelector::*first)(void), void (TreeSelector::*then)(void)) {
    const auto start = _tree_selector.get_selection();
    (_tree_selector.*first)();
    for (auto last = start; _tree_selector.get_selection() != nullptr and _tree_selector.get_selection()->is_skipped;) {
        if (_tree_selector.get_selection() == last) {
            // Nowhere else to go: back where it was
            _tree_selector.select(start);
            return;
        }
        last = _tree_selector.get_selection();
        // Moving by what's printed needs the layout around the new selection
        layout_tree();
        (_tree_selector.*then)();
    }
}

bool TreeKeyboardSelector::move_to_prev(void) {
    move_skipping(&TreeSelector::move_to_prev, &TreeSelector::move_to_prev);
    return false;
}

bool TreeKeyboardSelector::move_to_next(void) {
    move_skipping(&TreeSelector::move_to_next, &TreeSelector::move_to_next);
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
    move_skipping(&TreeSelector::move_to_first_sibling, &TreeSelector::move_to_next_sibling);
    return false;
}

bool TreeKeyboardSelector::move_to_last_sibling(void) {
    move_skipping(&TreeSelector::move_to_last_sibling, &TreeSelector::move_to_prev_sibling);
    return false;
}

bool TreeKeyboardSelector::move_to_next_sibling(void) {
    move_skipping(&TreeSelector::move_to_next_sibling, &TreeSelector::move_to_next_sibling);
    return false;
}

bool TreeKeyboardSelector::move_to_prev_sibling(void) {
    move_skipping(&TreeSelector::move_to_prev_sibling, &TreeSelector::move_to_prev_sibling);
    return false;
}

bool TreeKeyboardSelector::move_to_next_in_tree(void) {
    const auto nr_nodes = _tree.get_nr_nodes();
    move_skipping(&TreeSelector::move_to_next_in_tree, &TreeSelector::move_to_next_in_tree);
    search_again_if_loaded(nr_nodes);
    return false;
}

bool TreeKeyboardSelector::move_to_prev_in_tree(void) {
    const auto nr_nodes = _tree.get_nr_nodes();
    move_skipping(&TreeSelector::move_to_prev_in_tree, &TreeSelector::move_to_prev_in_tree);
    search_again_if_loaded(nr_nodes);
    return false;
}

void TreeKeyboardSelector::search_again_if_loaded(int nr_nodes_before) {
    // Moving through a lazy tree may have loaded dirs that a search doesn't know of yet
    if (_tree.get_nr_nodes() != nr_nodes_before and not _applied_keyword.empty()) {
        _search.search(_applied_keyword, _is_filtering);
        _tree_selector.select_visible_ancestor();
    }
}

bool TreeKeyboardSelector::move_to_first_shown(void) {
    layout_tree();
    auto node = _tree_printer.get_top_printed_node();
    if (node != nullptr) {
        _tree_selector.select(node);
    }
    return false;
}

bool TreeKeyboardSelector::move_to_last_shown(void) {
    layout_tree();
    auto node = _tree_printer.get_last_printed_node();
    if (node != nullptr) {
        _tree_selector.select(node);
    }
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
        auto selection = _tree_selector.get_selection();
        if ((_options.is_root_pickable or selection != _tree.get_root()) and
                not selection->is_disabled) {
            _picked = {_tree_selector.get_selection()};
            return true;
        }
    } else {
        auto selection = _tree_selector.get_selection();
        if (_tree_selector.get_nr_picked() == 0 and _options.is_selection_picked_if_none and
                not selection->is_disabled and (_options.is_root_pickable or selection != _tree.get_root())) {
            // As Space would pick it
            toggle_pick();
        }
        if (_tree_selector.get_nr_picked() > 0) {
            _picked = _tree_selector.get_picked_nodes();
            return true;
        }
    }
    return false;
}

bool TreeKeyboardSelector::toggle_pick(void) {
    if (_options.is_multi and not _tree_selector.get_selection()->is_disabled) {
        // While a search hides what doesn't match, only what it shows
        _tree_selector.toggle_pick_of_selection(not _applied_keyword.empty() and _is_filtering);
    }
    return false;
}

bool TreeKeyboardSelector::focus_side(void) {
    if (get_layout().side_height > 0) {
        end_range(true);
        _is_side_focused = true;
    }
    return false;
}

bool TreeKeyboardSelector::start_window_key(void) {
    _is_window_key_pending = true;
    return false;
}

bool TreeKeyboardSelector::finish_in_side(int key) {
    _expected_key = _options.side_key;
    _side_pressed_key = key;
    _picked = _options.is_multi ? _tree_selector.get_picked_nodes() :
        std::vector<std::shared_ptr<treelib::Node>>();
    return true;
}

bool TreeKeyboardSelector::side_char_pressed(int c) {
    const int height = get_layout().side_height;
    if (height <= 0) {
        // No room for it anymore (e.g. a resize): back to the tree
        _is_side_focused = false;
        print_tree();
        return false;
    }
    if (_options.side_key != 0 and c > 0 and c <= 0xff and
            _options.side_expect_keys.find(static_cast<char>(c)) != std::string::npos) {
        return finish_in_side(c);
    }
    // Moving relies on what's printed (as in the tree), so it's laid out first
    _side_printer->layout(false, _side_selector->get_selection(), height);
    switch (c) {
        case KEYCODE_CTRL_H:
        case KEY_BACKSPACE:
        case KEYCODE_BACKSPACE:
        case KEYCODE_ESCAPE:
            _is_side_focused = false;
            break;
        case KEYCODE_J: case KEY_DOWN: _side_selector->move_to_next(); break;
        case KEYCODE_K: case KEY_UP: _side_selector->move_to_prev(); break;
        case KEYCODE_H: case KEY_LEFT: _side_selector->move_one_up(); break;
        case KEYCODE_L: case KEY_RIGHT: _side_selector->explore_children_of_selection(); break;
        case KEYCODE_UPPER_J: _side_selector->move_to_next_in_tree(); break;
        case KEYCODE_UPPER_K: _side_selector->move_to_prev_in_tree(); break;
        case KEYCODE_CTRL_J: _side_selector->move_to_next_sibling(); break;
        case KEYCODE_CTRL_K: _side_selector->move_to_prev_sibling(); break;
        case KEY_HOME: _side_selector->move_to_first_sibling(); break;
        case KEY_END: _side_selector->move_to_last_sibling(); break;
        case KEYCODE_ENTER:
        case KEY_ENTER:
            if (_options.side_key != 0) {
                return finish_in_side(0);
            }
            break;
        case KEYCODE_QUESTION_MARK:
            show_help();
            break;
        case KEYCODE_Q:
        case KEYCODE_CTRL_C:
            return quit();
        default:
            break;
    }
    print_tree();
    return false;
}

std::shared_ptr<treelib::Node> TreeKeyboardSelector::closest_in_side(const std::string& path) {
    auto& side_tree = *_options.side_tree;
    std::string id = path == "." ? "" : path;
    auto node = id.empty() ? side_tree.get_root() : side_tree.get_node(id);
    while (node == nullptr and not id.empty()) {
        const auto slash = id.rfind('/');
        id = slash == std::string::npos ? "" : id.substr(0, slash);
        node = id.empty() ? side_tree.get_root() : side_tree.get_node(id);
    }
    return node;
}

void TreeKeyboardSelector::apply_side_selection(const std::string& path) {
    if (_side_selector == nullptr or _is_side_focused) {
        return;
    }
    const auto node = closest_in_side(path);
    if (node == nullptr or node == _side_selector->get_selection()) {
        return;
    }
    _side_selector->select(node);
    if (_mode != mode::MODE_HELP) {
        print_tree();
    }
}

void TreeKeyboardSelector::print_side(const Layout& layout) {
    const int column = layout.preview_column + 2;
    const int width = std::max(0, getmaxx(stdscr) - 1 - column);
    // Its title: its root's name, and whether the user is moving in it (and how to, if not)
    std::string title = (_is_side_focused ? "\u25b6 " : "") + guishell::printable(_options.side_tree->get_root()->tag) +
        (_is_side_focused ? "  (Ctrl-W h: back)" : "  (Ctrl-W l: move here)");
    title.resize(guishell::nr_bytes_that_fit(title.data(), title.size(), width));
    attron(A_BOLD);
    if (_is_side_focused) {
        attron(A_REVERSE);
    }
    mvaddstr(layout.side_title_row, column, title.c_str());
    attroff(A_REVERSE);
    attroff(A_BOLD);
    move(layout.side_row, column);
    _out.set_line_origin(column);
    _side_printer->set_selection_highlighted(_is_side_focused);
    _side_printer->print(_out, false, _side_selector->get_selection(), layout.side_height, width);
    _out.flush();
    _out.set_line_origin(0);
}

bool TreeKeyboardSelector::toggle_range(void) {
    if (not _options.is_multi) {
        return false;
    }
    if (_range_start != nullptr) {
        end_range(true);
        return false;
    }
    _range_start = _tree_selector.get_selection();
    pick_range();
    return false;
}

void TreeKeyboardSelector::end_range(bool keep) {
    if (not keep) {
        for (auto& node : _range_picked) {
            _tree_selector.set_picked(node, false);
        }
        _tree_selector.update_picked_dirs();
    }
    _range_start = nullptr;
    _range_picked.clear();
}

void TreeKeyboardSelector::pick_range(void) {
    for (auto& node : _range_picked) {
        _tree_selector.set_picked(node, false);
    }
    _range_picked.clear();
    // The nodes shown (that a search doesn't hide), in the tree's order, each with its depth
    std::vector<std::pair<std::shared_ptr<treelib::Node>, int>> preorder;
    std::vector<std::pair<const std::shared_ptr<treelib::Node>*, int>> dfs_stack = {{&_tree.get_root(), 0}};
    while (not dfs_stack.empty()) {
        auto top = dfs_stack.back();
        dfs_stack.pop_back();
        const auto& node = *top.first;
        if (not node->is_search_visible) {
            continue;
        }
        preorder.emplace_back(node, top.second);
        for (auto child = node->children.rbegin(); child != node->children.rend(); ++child) {
            dfs_stack.emplace_back(&*child, top.second + 1);
        }
    }
    std::size_t first = preorder.size(), last = 0;
    for (std::size_t i = 0; i < preorder.size(); ++i) {
        if (preorder[i].first == _range_start or preorder[i].first == _tree_selector.get_selection()) {
            first = std::min(first, i);
            last = i;
        }
    }
    if (first == preorder.size()) {
        // The range's start is gone (e.g. hidden by a search)
        _tree_selector.update_picked_dirs();
        return;
    }
    // With what's under each node of it: under the last, the nodes after it that are deeper
    for (std::size_t i = first; i <= last; ++i) {
        while (last + 1 < preorder.size() and preorder[last + 1].second > preorder[i].second) {
            ++last;
        }
    }
    for (std::size_t i = first; i <= last; ++i) {
        auto& node = preorder[i].first;
        if (not node->is_picked and not node->is_disabled) {
            _tree_selector.set_picked(node, true);
            _range_picked.push_back(node);
        }
    }
    _tree_selector.update_picked_dirs();
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
    } else if (_options.is_multi) {
        // Without a search to drop, unpick all
        for (auto& node : _tree_selector.get_picked_nodes()) {
            _tree_selector.set_picked(node, false);
        }
        _tree_selector.update_picked_dirs();
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

bool TreeKeyboardSelector::toggle_preview(void) {
    _is_preview_shown = not _is_preview_shown;
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
    // The caller's: what its keys do here, and what else it says
    if (not _options.key_hints.empty() or not _options.help_lines.empty()) {
        lines.push_back("");
        lines.push_back("Here:");
        for (auto& hint : _options.key_hints) {
            lines.push_back("  " + hint.key + std::string(std::max<int>(1, 17 - hint.key.size()), ' ') +
                            hint.text);
        }
        for (auto& line : _options.help_lines) {
            lines.push_back(line);
        }
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
    load_around_selection();
    const int window_width = getmaxx(stdscr);
    const Layout layout = get_layout();
    // No room for the side tree (too small a window, from the start or after a resize): back to the
    // tree, so its selection shows, and the next key goes to it rather than taking the user back
    if (_is_side_focused and layout.side_height <= 0) {
        _is_side_focused = false;
    }

    // Print header
    for (int row = 0; row < layout.nr_header_rows; ++row) {
        print_line(row, guishell::printable(_header_lines[row]), window_width, false);
    }

    // Print tree, below it: up to the screen's last column, which stays empty (see
    // NCursesSink::write), or with a preview, up to a space before its column
    if (layout.tree_row > layout.nr_header_rows) {
        print_line(layout.nr_header_rows, "", window_width, false);
    }
    move(layout.tree_row, 0);
    const int tree_width = layout.preview_column >= 0 ? layout.preview_column - 1 :
        window_width - 1;
    if (layout.tree_height > 0) {
        _tree_printer.set_selection_highlighted(not _is_side_focused);
        _out.set_line_width(tree_width);
        _tree_printer.print(_out, not _applied_keyword.empty(), _tree_selector.get_selection(),
                            layout.tree_height, tree_width);
        _out.set_line_width(0);
    }
    _out.flush();
    if (layout.nr_preview_rows > 0) {
        print_preview_below(layout, window_width);
    }
    if (layout.preview_column >= 0) {
        print_preview(layout);
    }
    if (layout.side_height > 0) {
        print_side(layout);
    }

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
        if (_range_start != nullptr) {
            stats_line += " (picking a range: V done, Esc undo)";
        }
        stats_line += std::string(", selected: ") +
            guishell::printable(_tree_selector.get_selection()->tag);
        // Where it is in its dir, which a long one shows only part of
        const auto place = _tree_selector.get_place_among_siblings(not _applied_keyword.empty());
        if (place.second > 1) {
            stats_line += " (" + std::to_string(place.first) + "/" +
                std::to_string(place.second) + ")";
        }
        // Room for the labels, which are drawn over it, each followed by a space
        // The caller's, then what it says it's doing
        std::vector<Label> shown_labels = _options.labels;
        for (auto& label : parse_status(_status)) {
            shown_labels.push_back(label);
        }
        std::vector<std::string> labels;
        int labels_width = 0;
        for (auto& label : shown_labels) {
            labels.push_back(" " + guishell::printable(label.text) + " ");
            labels_width += guishell::display_width(labels.back()) + 1;
        }
        stats_line = std::string(labels_width, ' ') + stats_line;
        attron(COLOR_PAIR(guishell::color_pair(guishell::BLACK_ON_WHITE)));
        print_line(layout.stats_row, stats_line, window_width, true);
        attroff(COLOR_PAIR(guishell::color_pair(guishell::BLACK_ON_WHITE)));
        int column = 0;
        for (std::size_t i = 0; i < labels.size(); ++i) {
            const int pair = guishell::label_color_pair(shown_labels[i].color);
            attron(COLOR_PAIR(pair));
            attron(A_BOLD);
            const int width = std::max(0, window_width - 1 - column);
            mvaddnstr(layout.stats_row, column, labels[i].data(),
                      guishell::nr_bytes_that_fit(labels[i].data(), labels[i].size(), width));
            attroff(A_BOLD);
            attroff(COLOR_PAIR(pair));
            column += guishell::display_width(labels[i]) + 1;
        }
    }

    // The bottom row: the keys, or while there's a search, the search line (last, so that the
    // cursor stays at its end while editing it)
    if (layout.search_row >= 0 and not is_search_line_shown()) {
        print_key_hints(layout.search_row, window_width);
    } else if (layout.search_row >= 0) {
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

void TreeKeyboardSelector::print_key_hints(int row, int window_width) {
    move(row, 0);
    clrtoeol();
    const int width = std::max(0, window_width - 1);
    int column = 0;
    for (auto& hint : key_hints(_options, _tree_selector.get_nr_picked() == 0, _is_side_focused)) {
        // The key in reverse video, then what it does, two spaces apart from the next
        const std::string key = " " + guishell::printable(hint.key) + " ";
        const std::string text = " " + guishell::printable(hint.text) + "  ";
        if (column + guishell::display_width(key) > width) {
            break;
        }
        attron(A_REVERSE);
        mvaddstr(row, column, key.c_str());
        attroff(A_REVERSE);
        column += guishell::display_width(key);
        const int left = width - column;
        mvaddnstr(row, column, text.data(), guishell::nr_bytes_that_fit(text.data(), text.size(), left));
        column += std::min(left, guishell::display_width(text));
    }
}

const std::vector<std::string>& TreeKeyboardSelector::selected_preview(void) {
    const auto preview = _previews.find(_tree_selector.get_selection()->identifier);
    return preview == _previews.end() ? _default_preview : preview->second;
}

void TreeKeyboardSelector::print_preview_below(const Layout& layout, int window_width) {
    const auto& lines = selected_preview();
    const int width = std::max(0, window_width - 1);
    for (int i = 0; i < layout.nr_preview_rows; ++i) {
        const int row = layout.preview_row + i;
        move(row, 0);
        clrtoeol();
        if (i >= static_cast<int>(lines.size())) {
            continue;
        }
        int column = 0;
        for (auto& segment : parse_segments(lines[i])) {
            std::string text = guishell::printable(segment.text);
            text.resize(guishell::nr_bytes_that_fit(text.data(), text.size(), std::max(0, width - column)));
            const int pair = segment.color.empty() ? -1 : guishell::label_color_pair(segment.color);
            if (pair >= 0) {
                attron(COLOR_PAIR(pair));
                attron(A_BOLD);
            }
            mvaddstr(row, column, text.c_str());
            if (pair >= 0) {
                attroff(A_BOLD);
                attroff(COLOR_PAIR(pair));
            }
            column += guishell::display_width(text);
        }
    }
}

void TreeKeyboardSelector::print_styled(int row, int column, const std::string& line, int width) {
    move(row, column);
    for (auto& part : guishell::parse_styled(line)) {
        std::string text = guishell::printable(part.text);
        text.resize(guishell::nr_bytes_that_fit(text.data(), text.size(), std::max(0, width)));
        attron(COLOR_PAIR(guishell::color_pair(part.style)));
        if (part.is_bold) {
            attron(A_BOLD);
        }
        addstr(text.c_str());
        attroff(A_BOLD);
        attroff(COLOR_PAIR(guishell::color_pair(part.style)));
        width -= guishell::display_width(text);
    }
}

void TreeKeyboardSelector::print_preview(const Layout& layout) {
    const auto& lines = selected_preview();
    // After the line and a space, up to the screen's last column, which stays empty
    const int column = layout.preview_column + 2;
    const int width = std::max(0, getmaxx(stdscr) - 1 - column);
    for (int i = 0; i < layout.tree_height; ++i) {
        const int row = layout.tree_row + i;
        mvaddstr(row, layout.preview_column, row == layout.column_line_row ? "├" : "│");
        if (row == layout.column_line_row) {
            for (int column_left = 0; column_left <= width; ++column_left) {
                addstr("─");
            }
        }
        // Below the side tree, if there's one
        const int line_index = row - layout.column_preview_row;
        if (line_index >= 0 and line_index < layout.nr_column_preview_rows and
                line_index < static_cast<int>(lines.size())) {
            const Label line = parse_preview_line(lines[line_index]);
            std::string text = guishell::printable(line.text);
            text.resize(guishell::nr_bytes_that_fit(text.data(), text.size(), width));
            const int pair = line.color.empty() ? -1 : guishell::label_color_pair(line.color);
            if (pair < 0 and line.text.find(guishell::STYLE_MARK) != std::string::npos) {
                print_styled(row, column, line.text, width);
                continue;
            }
            if (pair < 0) {
                mvaddstr(row, column, text.c_str());
                continue;
            }
            // Across the column, so it's seen at once
            text += std::string(std::max(0, width - guishell::display_width(text)), ' ');
            attron(COLOR_PAIR(pair));
            attron(A_BOLD);
            mvaddstr(row, column, text.c_str());
            attroff(A_BOLD);
            attroff(COLOR_PAIR(pair));
        }
    }
    // The image below the lines, after a blank row
    const int nr_lines = std::min(layout.nr_column_preview_rows, static_cast<int>(lines.size()));
    const int image_row = layout.column_preview_row + nr_lines + (nr_lines > 0 ? 1 : 0);
    print_image(image_row, layout.column_preview_row + layout.nr_column_preview_rows - image_row,
                column, width);
}

void TreeKeyboardSelector::print_image(int row, int nr_rows, int column, int width) {
    // Fewer, and it's too small to make out
    static constexpr int MIN_ROWS = 3;
    static constexpr int MIN_WIDTH = 6;
    if (nr_rows < MIN_ROWS or width < MIN_WIDTH or not guishell::can_show_images()) {
        return;
    }
    const auto found = _images.find(_tree_selector.get_selection()->identifier);
    if (found == _images.end()) {
        return;
    }
    // Two pixels a character, one above the other
    const Image image = fit_image(found->second, width, nr_rows * 2);
    if (guishell::nr_image_color_pairs_left() < image.width * (image.height + 1) / 2) {
        guishell::reset_image_color_pairs();
    }
    for (int y = 0; y < image.height; y += 2) {
        move(row + y / 2, column);
        for (int x = 0; x < image.width; ++x) {
            const short upper = nearest_256_color(image.at(x, y));
            // The last row of an odd height: the background's color below
            const short lower = y + 1 < image.height ? nearest_256_color(image.at(x, y + 1)) : -1;
            attr_set(A_NORMAL, guishell::image_color_pair(upper, lower), nullptr);
            addstr("\u2580");
        }
    }
    attr_set(A_NORMAL, 0, nullptr);
}

bool TreeKeyboardSelector::is_search_line_shown(void) {
    return not _applied_keyword.empty() or _mode == mode::MODE_EDIT_SEARCH;
}

TreeKeyboardSelector::Layout TreeKeyboardSelector::get_layout(void) {
    Layout layout;
    int nr_rows_left = getmaxy(stdscr);

    // The bottom row: the keys, or the search line while there's a search
    layout.search_row = -1;
    if (nr_rows_left > 0) {
        layout.search_row = --nr_rows_left;
    }

    // The stats line above it, unless the tree would then be too short to show the selection
    layout.stats_row = -1;
    if (nr_rows_left - 1 >= MIN_TREE_HEIGHT) {
        layout.stats_row = --nr_rows_left;
    }

    // The preview below the tree (with --preview-below), as many rows as its longest, if the tree
    // keeps enough room
    layout.preview_row = -1;
    layout.nr_preview_rows = 0;
    if (_options.preview_fd >= 0 and _options.is_preview_below) {
        std::size_t longest = _default_preview.size();
        for (auto& preview : _previews) {
            longest = std::max(longest, preview.second.size());
        }
        layout.nr_preview_rows = std::min({static_cast<int>(longest), MAX_PREVIEW_BELOW_ROWS,
                                           std::max(0, nr_rows_left - MIN_TREE_HEIGHT)});
        nr_rows_left -= layout.nr_preview_rows;
        layout.preview_row = nr_rows_left;
    }

    // The header on top, as much of it as leaves the tree enough room, and a blank row after it
    layout.nr_header_rows = std::min(static_cast<int>(_header_lines.size()),
                                     std::max(0, nr_rows_left - MIN_TREE_HEIGHT));
    nr_rows_left -= layout.nr_header_rows;
    layout.tree_row = layout.nr_header_rows;
    if (layout.nr_header_rows > 0 and nr_rows_left - 1 >= MIN_TREE_HEIGHT) {
        ++layout.tree_row;
        --nr_rows_left;
    }

    layout.tree_height = nr_rows_left;

    // The preview column on the right, if there are previews, and room for it
    const int window_width = getmaxx(stdscr);
    layout.preview_column = -1;
    const bool has_column_preview = _options.preview_fd >= 0 and not _options.is_preview_below;
    if ((has_column_preview or _side_selector != nullptr) and _is_preview_shown and
            window_width >= MIN_WIDTH_WITH_PREVIEW) {
        layout.preview_column = window_width - window_width * PREVIEW_FIFTHS / 5;
    }
    layout.nr_column_preview_rows = has_column_preview ? layout.tree_height : 0;
    layout.column_preview_row = layout.tree_row;
    if (layout.preview_column >= 0 and _side_selector != nullptr) {
        // The side tree on top, level with the tree, after its title; and below it, after a line,
        // the preview
        const int nr_preview_rows = has_column_preview ? layout.tree_height * SIDE_PREVIEW_FIFTHS / 5 : 0;
        const int nr_line_rows = nr_preview_rows > 0 ? 1 : 0;
        const int side_height = layout.tree_height - nr_preview_rows - nr_line_rows - 1;
        if (side_height >= MIN_SIDE_HEIGHT) {
            layout.nr_column_preview_rows = nr_preview_rows;
            layout.side_title_row = layout.tree_row;
            layout.side_row = layout.side_title_row + 1;
            layout.side_height = side_height;
            layout.column_line_row = nr_line_rows > 0 ? layout.side_row + side_height : -1;
            layout.column_preview_row = layout.side_row + side_height + nr_line_rows;
        }
    }
    return layout;
}

void TreeKeyboardSelector::layout_tree(void) {
    load_around_selection();
    const int tree_max_height = get_layout().tree_height;
    if (tree_max_height > 0) {
        _tree_printer.layout(not _applied_keyword.empty(), _tree_selector.get_selection(),
                             tree_max_height);
    }
}

void TreeKeyboardSelector::load_around_selection(void) {
    auto selection = _tree_selector.get_selection();
    if (selection == nullptr) {
        return;
    }
    bool is_any_loaded = _tree.load_children(selection);

    // The printer shows a subtree, down as many levels as fit: from the root it showed last, or
    // from the selection's parent (see choose_printed_subtree_root()). So from each, the nodes of
    // the levels under it, until there are more than fit on the screen (which a whole level then
    // doesn't).
    const int nr_rows = getmaxy(stdscr);
    auto parent = selection == _tree.get_root() ? selection : _tree.get_node(selection->parent);
    for (auto& top : {_tree_printer.get_printed_subtree_root(), parent}) {
        std::vector<std::shared_ptr<treelib::Node>> level = {top};
        int nr_nodes = 1;
        while (not level.empty() and nr_nodes <= nr_rows) {
            std::vector<std::shared_ptr<treelib::Node>> next_level;
            for (auto& node : level) {
                is_any_loaded = _tree.load_children(node) or is_any_loaded;
                for (auto& child : node->children) {
                    if (nr_nodes > nr_rows) {
                        break;
                    }
                    next_level.push_back(child);
                    ++nr_nodes;
                }
            }
            level = std::move(next_level);
        }
    }

    // New nodes are shown, whatever a search would say of them
    if (is_any_loaded and not _applied_keyword.empty()) {
        _search.search(_applied_keyword, _is_filtering);
        _tree_selector.select_visible_ancestor();
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

void TreeKeyboardSelector::apply_updates(const std::vector<Update>& updates) {
    if (updates.empty()) {
        return;
    }
    for (const auto& update : updates) {
        if (update.id.empty()) {
            _status = update.tag;
            continue;
        }
        auto node = _tree.get_node(update.id);
        if (node != nullptr) {
            // Searches match the text, not its styles' marks
            const bool is_styled = update.tag.find(guishell::STYLE_MARK) != std::string::npos;
            node->tag = is_styled ? guishell::unstyled(update.tag) : update.tag;
            node->styled_tag = is_styled ? update.tag : "";
            node->is_disabled = update.is_disabled;
            node->is_skipped = update.is_skipped;
            node->color = update.color;
        }
    }
    // Searches match tags
    if (not _applied_keyword.empty()) {
        _search.search(_applied_keyword, _is_filtering);
        _tree_selector.select_visible_ancestor();
    }
    print_tree();
}

void TreeKeyboardSelector::apply_previews(const std::vector<Preview>& previews) {
    if (previews.empty()) {
        return;
    }
    for (const auto& preview : previews) {
        if (preview.is_image) {
            const std::string id = preview.path == "." and _tree.get_root() != nullptr ?
                _tree.get_root()->identifier : preview.path;
            _image_ids.erase(std::remove(_image_ids.begin(), _image_ids.end(), id), _image_ids.end());
            if (preview.image.empty()) {
                _images.erase(id);
                continue;
            }
            _images[id] = preview.image;
            _image_ids.push_back(id);
            if (_image_ids.size() > MAX_IMAGES) {
                _images.erase(_image_ids.front());
                _image_ids.pop_front();
            }
        } else if (preview.path.empty()) {
            _default_preview = preview.lines;
        } else if (preview.path == "." and _tree.get_root() != nullptr) {
            // As printed: the root is "."
            _previews[_tree.get_root()->identifier] = preview.lines;
        } else {
            _previews[preview.path] = preview.lines;
        }
    }
    // Only the column changes, but drawing it alone isn't worth its own code
    if (get_layout().preview_column >= 0 and _mode != mode::MODE_HELP) {
        print_tree();
    }
}

} // namespace picker
