#ifndef __PICKER_H
#define __PICKER_H

#include <memory>
#include <vector>
#include "tree/tree.h"
#include "utils/guishell.h"
#include "picker/keyboardreactor.h"
#include "picker/picker_options.h"
#include "picker/tree_keyboard_selector.h"
#include "picker/updates.h"
#include "picker/previews.h"

namespace picker {

class Picker {
public:
    Picker(treelib::Tree& tree, const PickerOptions& options = PickerOptions());

    // Let the user pick, and return what they picked: empty if they quit
    std::vector<std::shared_ptr<treelib::Node>> run(void);

    // Once run: the expect key that finished it, or 0 (see PickerOptions::expect_keys)
    int get_expected_key(void) const { return _tree_keyboard_selector.get_expected_key(); }

    // The selected node, e.g. when an expect key finished it
    std::shared_ptr<treelib::Node> get_selection(void) { return _tree_keyboard_selector.get_selection(); }

    // The side tree's selected node (see PickerOptions::side_tree), e.g. when Enter in it finished it
    std::shared_ptr<treelib::Node> get_side_selection(void) { return _tree_keyboard_selector.get_side_selection(); }

private:
    // Write the selection's path to options.selection_fd, if it moved since it was last written
    void report_selection(void);

    // Write to options.notify_fd that key was pressed: the key, the selected node's path, the
    // picked ones' paths, each ending with the delimiter, and an empty one to end it (as printed:
    // the root is ".", so no path is empty)
    void notify(int key);

    // Write data to fd, all of it; false if it can't (and fd is then not written to again)
    static bool write_all(int& fd, const std::string& data);

    // A node's path, as printed
    std::string path_of(const std::shared_ptr<treelib::Node>& node);

    TreeKeyboardSelector _tree_keyboard_selector;
    treelib::Tree& _tree;
    UpdatesReader _updates_reader;
    PreviewsReader _previews_reader;
    int _selection_fd;
    int _notify_fd;
    char _delimiter;
    // The node last written to _selection_fd
    std::shared_ptr<treelib::Node> _reported_selection;
    // The start of a path read from options.side_select_fd whose end hasn't come yet
    std::string _side_select_pending;
    KeyboardReactor _keyboard_reactor;
};

} // namespace picker

#endif
