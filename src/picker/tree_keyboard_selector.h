//TODO generalize this module to a 'keyboard selector'
#ifndef TREE_KEYBOARD_SELECTOR_H__
#define TREE_KEYBOARD_SELECTOR_H__

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "tree/tree.h"
#include "treeprinter/treeprinter.h"
#include "utils/guishell.h"
#include "tree_selector.h"
#include "tree/search.h"
#include "picker/picker_options.h"
#include "picker/updates.h"
#include "picker/previews.h"

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
    // Enter sends a carriage return; Ctrl-J, a newline (told apart since curses doesn't turn
    // carriage returns into newlines: see guishell's nonl())
    KEYCODE_ENTER = 13,
    KEYCODE_CTRL_J = 10,
    KEYCODE_CTRL_K = 11,
    KEYCODE_CTRL_H = 8,
    KEYCODE_CTRL_L = 12,
    KEYCODE_H = 104,
    KEYCODE_J = 106,
    KEYCODE_K = 107,
    KEYCODE_L = 108,
    KEYCODE_LOWER_G = 103,
    KEYCODE_P = 112,
    KEYCODE_F = 102,
    KEYCODE_QUESTION_MARK = 63,
    KEYCODE_CTRL_SLASH = 31,  // What terminals send for Ctrl-/ (and Ctrl-_)
    KEYCODE_UPPER_G = 71,
    KEYCODE_UPPER_V = 86,
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

    // Once finished: the expect key (see PickerOptions::expect_keys) that finished it, or 0 if
    // none did. Then get_picked() is what's picked so far (with is_multi; else nothing).
    int get_expected_key(void) const { return _expected_key; }

    std::shared_ptr<treelib::Node> get_selection(void) { return _tree_selector.get_selection(); }

    void print_tree(void);

    void initialize(void);

    // Show the nodes with these ids by these tags, enabled or disabled, from now on. Ids that
    // aren't in the tree are skipped.
    void apply_updates(const std::vector<Update>& updates);

    // Show these previews beside the tree from now on (each replacing the node's earlier one). Ones
    // for paths that aren't in the tree are kept anyway: they cost little.
    void apply_previews(const std::vector<Preview>& previews);

    // Select the side tree's node at the path (or its closest ancestor there), unless the user is
    // moving in the side tree (see PickerOptions::side_tree)
    void apply_side_selection(const std::string& path);

    // The side tree's selected node (nullptr without one)
    std::shared_ptr<treelib::Node> get_side_selection(void) {
        return _side_selector ? _side_selector->get_selection() : nullptr;
    }

    // Call notify(key) when a PickerOptions::notify_keys key is pressed (moving around)
    void set_notify(std::function<void(int)> notify) { _notify = notify; }

    // The picked nodes so far (with is_multi)
    std::vector<std::shared_ptr<treelib::Node>> get_picked_so_far(void) { return _tree_selector.get_picked_nodes(); }

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
    bool move_to_next_sibling(void);
    // V: start picking a range, or end it, keeping its picks
    bool toggle_range(void);
    // Pick the range: every node shown from _range_start to the selection, in the tree's order,
    // each with what's under it (as Space picks a dir)
    void pick_range(void);
    // Stop picking a range; without keep, unpick what it picked
    void end_range(bool keep);
    // Move with first, then again with then while the selection is a skipped node (see
    // Node::is_skipped); back where it was if that gets to no other node
    void move_skipping(void (TreeSelector::*first)(void), void (TreeSelector::*then)(void));
    bool move_to_prev_sibling(void);
    bool move_to_first_shown(void);
    bool move_to_last_shown(void);
    bool explore_children(void);
    bool move_one_up(void);
    bool pick(void);
    bool toggle_pick(void);
    bool quit(void);
    bool drop_search(void);
    bool show_help(void);
    bool toggle_filter(void);
    bool toggle_preview(void);
    // Ctrl-L: move in the side tree (until Ctrl-H)
    bool focus_side(void);
    // A key pressed while moving in the side tree; returns whether picking is finished
    bool side_char_pressed(int c);

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

    // The most rows the preview below the tree takes (--preview-below)
    static constexpr int MAX_PREVIEW_BELOW_ROWS = 4;
    // The narrowest screen that the preview column is shown on, and the part of the screen's
    // width it takes, in fifths
    static constexpr int MIN_WIDTH_WITH_PREVIEW = 60;
    static constexpr int PREVIEW_FIFTHS = 2;
    static constexpr int SIDE_PREVIEW_FIFTHS = 2;
    // The fewest rows the side tree is shown in
    static constexpr int MIN_SIDE_HEIGHT = 3;

    // Where things go on the screen, from the top: the header, the tree (with the preview column
    // beside it), the stats line, the search line.
    // A row (or column) of -1 means it isn't shown.
    struct Layout {
        int nr_header_rows;
        int tree_height;
        int stats_row;
        int search_row;
        // The column of the line between the tree and the preview column
        int preview_column;
        // The row the tree starts at (below the header, and a blank row after it), and the
        // preview's rows below the tree (--preview-below): the first, and how many (0: none)
        int tree_row;
        int preview_row;
        int nr_preview_rows;
        // The right column's rows of the preview (the first, and how many), the line above it
        // when the side tree is too (-1: none), and the side tree's: its title's row, its first
        // one and how many (0: none)
        int nr_column_preview_rows = 0;
        int column_preview_row = -1;
        int column_line_row = -1;
        int side_title_row = -1;
        int side_row = -1;
        int side_height = 0;
    };
    Layout get_layout(void);

    bool is_search_line_shown(void);

    // Draw the keys' help instead of the tree
    void print_help(void);

    // Draw the keys row (see key_hints()) on the row
    void print_key_hints(int row, int window_width);

    // Draw the selected node's preview in its column, right of the tree's rows
    // A line in styled parts (see guishell::parse_styled()) at row and column, cut at width
    void print_styled(int row, int column, const std::string& line, int width);
    void print_preview(const Layout& layout);
    // Draw the side tree, above the preview, in the right column
    void print_side(const Layout& layout);

    // Draw it below the tree instead (--preview-below), in its colored parts
    void print_preview_below(const Layout& layout, int window_width);

    // The selected node's preview's lines
    const std::vector<std::string>& selected_preview(void);

    // Lay the tree out without drawing it, so the selection can move relative to what would
    // be printed
    void layout_tree(void);

    // For a tree that loads nodes lazily (Tree::load_children()): load what may be shown
    // around the selection, and what the keys may move to (its children)
    void load_around_selection(void);

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
    // Whether the preview column is shown (when there are previews, and it fits)
    bool _is_preview_shown;
    // The previews' lines, by node id, and the lines of the nodes without one
    std::unordered_map<std::string, std::vector<std::string>> _previews;
    std::vector<std::string> _default_preview;
    std::function<void(int)> _notify;
    // What the caller says it's doing (an update with no id), shown as a label; "": nothing
    std::string _status;
    treelib::TreePrinter _tree_printer;
    TreeSelector _tree_selector;
    // The side tree's printer and selection, and whether the user is moving in it
    std::unique_ptr<treelib::TreePrinter> _side_printer;
    std::unique_ptr<TreeSelector> _side_selector;
    bool _is_side_focused = false;
    mode _mode;
    ::treelib::Search _search;
    PickerOptions _options;
    // Whether the tree says which nodes are files (Node::kind), to count them
    bool _are_files_known;
    // The header's lines
    std::vector<std::string> _header_lines;
    std::vector<std::shared_ptr<treelib::Node>> _picked;
    int _expected_key;
    // While picking a range (V, with -m, like vim's visual line mode): the node V was pressed on,
    // and the nodes the range picked (the ones picked before stay), to unpick when it changes or
    // is dropped; nullptr: not picking a range
    std::shared_ptr<treelib::Node> _range_start;
    // An --expect-twice key pressed once, waiting for the second (0: none)
    int _pending_twice_key = 0;
    std::vector<std::shared_ptr<treelib::Node>> _range_picked;
};

}

#endif
