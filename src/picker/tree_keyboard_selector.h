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

    // A line of the keys' help, and the keys it's about, each with what it does when moving
    // around (not while editing the search keyword): the keys' only list, so the help can't
    // miss one
    struct KeyCommand {
        // Returns whether picking is finished
        using Action = bool (TreeKeyboardSelector::*)(void);
        const char* keys;
        // Lines after the first are shown indented under it
        const char* description;
        std::vector<std::pair<int, Action>> bindings;
    };
    static const std::vector<KeyCommand>& get_key_commands(void);

    // What was picked, once finished: empty if the user quit
    const std::vector<std::shared_ptr<treelib::Node>>& get_picked(void) const { return _picked; }

    void print_tree(void);

    void initialize(void);

    // Show the nodes with these ids by these tags (id, tag), from now on. Ids that aren't in the
    // tree are skipped.
    void set_tags(const std::vector<std::pair<std::string, std::string>>& tags);

private:
    // Number of nodes that page up/down move
    static constexpr int PAGE_SIZE = 10;

    // What the keys do (see get_key_commands())
    bool move_to_prev(void);
    bool move_to_next(void);
    bool page_down(void);
    bool page_up(void);
    bool move_to_first_sibling(void);
    bool move_to_last_sibling(void);
    bool explore_children(void);
    bool move_one_up(void);
    bool pick(void);
    bool toggle_pick(void);
    bool quit(void);
    bool drop_search(void);
    bool show_help(void);
    bool toggle_filter(void);
    bool start_search(void);
    bool start_search_on_enter(void);

    // Search for the keyword being edited, filtering or highlighting
    void search(void);

    // After the keyword being edited changed: search for it, unless searching waits for Enter
    void keyword_changed(void);

    // Leave search mode, back to navigation
    void stop_editing_keyword(void);

    // Lines the tree needs to show the selection: the root, the selection, and a '... N more'
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
