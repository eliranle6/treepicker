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
    KEYCODE_MOVE_FROM_SEARCH_TO_NAV_MODE = KEYCODE_ENTER,
    KEYCODE_CANCEL_SEARCH = KEYCODE_ESCAPE
};

enum mode {
    MODE_NAVIGATION,
    MODE_EDIT_SEARCH
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

    // Filter the tree by the current search keyword
    void search(void);

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

    // Lay the tree out without drawing it, so the selection can move relative to what would
    // be printed
    void layout_tree(void);

    treelib::Tree& _tree;
    guishell::GuiShell& _out;
    std::string _search_keyword;
    treelib::TreePrinter _tree_printer;
    TreeSelector _tree_selector;
    mode _mode;
    ::treelib::Search _search;
    PickerOptions _options;
    // The header's lines
    std::vector<std::string> _header_lines;
    std::vector<std::shared_ptr<treelib::Node>> _picked;
};

}

#endif
