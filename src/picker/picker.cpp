#include <cerrno>
#include <csignal>
#include <unistd.h>

#include "picker.h"
#include "tree/tree.h"

namespace picker {

Picker::Picker(treelib::Tree& tree, const PickerOptions& options) :
    _tree_keyboard_selector(tree, options),
    _tree(tree),
    _updates_reader(options.updates_delimiter),
    _previews_reader(options.updates_delimiter),
    _selection_fd(options.selection_fd),
    _delimiter(options.updates_delimiter),
    _keyboard_reactor([&](void) {
                          _tree_keyboard_selector.initialize();
                          report_selection();
                      },
                      [&](int c) {
                          const bool is_finished = _tree_keyboard_selector.char_pressed(c);
                          report_selection();
                          return is_finished;
                      },
                      {{options.updates_fd, [&](const std::string& data) {
                          _tree_keyboard_selector.apply_updates(_updates_reader.feed(data));
                          // A search may hide the selection, moving it
                          report_selection();
                       }},
                       {options.preview_fd, [&](const std::string& data) {
                          _tree_keyboard_selector.apply_previews(_previews_reader.feed(data));
                       }}})
{
    if (_selection_fd >= 0) {
        // A caller that stopped reading makes a write fail (then it's no longer written to),
        // rather than end treepicker
        signal(SIGPIPE, SIG_IGN);
    }
}

std::vector<std::shared_ptr<treelib::Node>> Picker::run(void) {
    _keyboard_reactor.run();
    return _tree_keyboard_selector.get_picked();
}

void Picker::report_selection(void) {
    auto selection = _tree_keyboard_selector.get_selection();
    if (_selection_fd < 0 or selection == nullptr or selection == _reported_selection) {
        return;
    }
    _reported_selection = selection;
    // As printed: the root is "."
    const std::string data = (selection == _tree.get_root() ? std::string(".") : selection->identifier) +
        _delimiter;
    for (std::size_t written = 0; written < data.size();) {
        const ssize_t size = write(_selection_fd, data.data() + written, data.size() - written);
        if (size < 0 and errno == EINTR) {
            continue;
        }
        if (size <= 0) {
            _selection_fd = -1;
            return;
        }
        written += size;
    }
}

}
