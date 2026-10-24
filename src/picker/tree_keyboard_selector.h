//TODO generalize this module to a 'keyboard selector'
#ifndef TREE_KEYBOARD_SELECTOR_H__
#define TREE_KEYBOARD_SELECTOR_H__

#include <memory>
#include <string>
#include <vector>
#include "tree/tree.h"
#include "treeprinter/treeprinter.h"
#include "utils/guishell.h"
#include "tree_selector.h"
#include "tree/search.h"
#include "picker/picker_options.h"

namespace picker {

enum {
    KEYCODE_CTRL_C = 3,
    KEYCODE_CTRL_D = 4,
    KEYCODE_CTRL_H = 8,
    KEYCODE_CTRL_J = 10,
    KEYCODE_CTRL_K = 11,
    KEYCODE_CTRL_L = 12,
    KEYCODE_CTRL_U = 21,
    KEYCODE_CTRL_W = 23,
    KEYCODE_ESCAPE = 27,  // Also Ctrl-[
    KEYCODE_BACKSPACE = 127,
    KEYCODE_SLASH = 47,
    KEYCODE_SPACE = 32,
    KEYCODE_ENTER = 10,
    KEYCODE_H = 104,
    KEYCODE_J = 106,
    KEYCODE_K = 107,
    KEYCODE_L = 108,
    KEYCODE_LOWER_G = 103,
    KEYCODE_F = 102,
    KEYCODE_QUESTION_MARK = 63,
    KEYCODE_CTRL_SLASH = 31,  // What terminals send for Ctrl-/ (and Ctrl-_)
    KEYCODE_UPPER_G = 71,
    KEYCODE_Q = 113
};

enum {
    KEYCODE_UP = KEYCODE_K,
    KEYCODE_DOWN = KEYCODE_J,
    KEYCODE_PAGE_UP = KEYCODE_CTRL_U,
    KEYCODE_PAGE_DOWN = KEYCODE_CTRL_D,
    KEYCODE_RIGHT = KEYCODE_L,
    KEYCODE_LEFT = KEYCODE_H,
    KEYCODE_BACK = KEYCODE_CTRL_C,
    KEYCODE_PICK = KEYCODE_ENTER,
    KEYCODE_TOGGLE_PICK = KEYCODE_SPACE,
    KEYCODE_QUIT = KEYCODE_Q,
    KEYCODE_FIRST_SIBLING = KEYCODE_LOWER_G,
    KEYCODE_LAST_SIBLING = KEYCODE_UPPER_G,
    KEYCODE_START_SEARCH = KEYCODE_SLASH,
    // Search, but only once Enter is pressed rather than as the keyword is typed
    KEYCODE_START_SEARCH_ON_ENTER = KEYCODE_CTRL_SLASH,
    // Switch between showing only the matches (and their ancestors), and the whole tree with
    // the matches highlighted
    KEYCODE_TOGGLE_FILTER = KEYCODE_F,
    KEYCODE_HELP = KEYCODE_QUESTION_MARK,
    KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE = KEYCODE_ENTER,
    KEYCODE_CANCEL_SEARCH = KEYCODE_ESCAPE
};

enum mode {
    MODE_NAVIGATION,
    MODE_EDIT_SEARCH,
    // Showing the keys, until any key is pressed
    MODE_HELP
};

class TreeKeyboardSelector
{
public:
    TreeKeyboardSelector(treelib::Tree& tree, const PickerOptions& options = PickerOptions());

    // Handle a key; returns whether picking is finished (a pick, or quitting)
    bool char_pressed(int c);

    // What was picked, once finished: empty if the user quit
    const std::vector<std::shared_ptr<treelib::Node>>& get_picked(void) const { return _picked; }

    void print_tree(void);

    void initialize(void);

private:
    // Number of nodes that page up/down move
    static constexpr int PAGE_SIZE = 10;

    // Search for the keyword being edited, filtering or highlighting
    void search(void);

    // After the keyword being edited changed: search for it, unless searching waits for Enter
    void keyword_changed(void);

    // Leave search mode, back to navigation
    void stop_editing_keyword(void);

    // Lines the tree needs to show the selection: the root, the selection, and a '... (N more)'
    // line on each side of it
    static constexpr int MIN_TREE_HEIGHT = 4;

    // Where things go on the screen, from the top: the header, the tree, the stats line, the
    // search line.
    // A row of -1 means it isn't shown.
    struct Layout {
        int nr_header_rows;
        int tree_height;
        int stats_row;
        int search_row;
    };
    Layout get_layout(void);

    bool is_search_line_shown(void);

    // Draw the keys' help instead of the tree
    void print_help(void);

    // Lay the tree out without drawing it, so the selection can move relative to what would
    // be printed
    void layout_tree(void);

    treelib::Tree& _tree;
    guishell::GuiShell& _out;
    // The keyword shown in the search line, maybe being edited
    std::string _search_keyword;
    // The keyword last searched for, so the one the tree is filtered by (or highlighted by)
    std::string _applied_keyword;
    // Whether the keyword being edited is searched for only on Enter
    bool _is_search_on_enter;
    // Whether a search hides the nodes that don't match, or only highlights the ones that do
    bool _is_filtering;
    treelib::TreePrinter _tree_printer;
    TreeSelector _tree_selector;
    mode _mode;
    ::treelib::Search _search;
    PickerOptions _options;
    // Whether the tree says which nodes are files (Node::kind), to count them
    bool _are_files_known;
    // The header's lines
    std::vector<std::string> _header_lines;
    std::vector<std::shared_ptr<treelib::Node>> _picked;
};

}

#endif
