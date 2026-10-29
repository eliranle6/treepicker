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

private:
    // Write the selection's path to options.selection_fd, if it moved since it was last written
    void report_selection(void);

    TreeKeyboardSelector _tree_keyboard_selector;
    treelib::Tree& _tree;
    UpdatesReader _updates_reader;
    PreviewsReader _previews_reader;
    int _selection_fd;
    char _delimiter;
    // The node last written to _selection_fd
    std::shared_ptr<treelib::Node> _reported_selection;
    KeyboardReactor _keyboard_reactor;
};

} // namespace picker

#endif
