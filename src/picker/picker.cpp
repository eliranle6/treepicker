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
    _notify_fd(options.notify_fd),
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
                       }},
                       {options.side_tree != nullptr ? options.side_select_fd : -1, [&](const std::string& data) {
                          // The last whole path read: where the side tree's selection goes
                          _side_select_pending += data;
                          std::string path;
                          bool is_any = false;
                          for (std::size_t end; (end = _side_select_pending.find(_delimiter)) != std::string::npos;) {
                              path = _side_select_pending.substr(0, end);
                              _side_select_pending.erase(0, end + 1);
                              is_any = true;
                          }
                          if (is_any) {
                              _tree_keyboard_selector.apply_side_selection(path);
                          }
                       }}})
{
    if (_notify_fd >= 0) {
        _tree_keyboard_selector.set_notify([&](int key) { notify(key); });
    }
    if (_selection_fd >= 0 or _notify_fd >= 0) {
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
    write_all(_selection_fd, path_of(selection) + _delimiter);
}

void Picker::notify(int key) {
    std::string data = std::string(1, static_cast<char>(key)) + _delimiter;
    auto selection = _tree_keyboard_selector.get_selection();
    if (selection != nullptr) {
        data += path_of(selection) + _delimiter;
    }
    for (auto& node : _tree_keyboard_selector.get_picked_so_far()) {
        data += path_of(node) + _delimiter;
    }
    write_all(_notify_fd, data + _delimiter);
}

std::string Picker::path_of(const std::shared_ptr<treelib::Node>& node) {
    // As printed: the root is "."
    return node == _tree.get_root() ? std::string(".") : node->identifier;
}

bool Picker::write_all(int& fd, const std::string& data) {
    for (std::size_t written = 0; fd >= 0 and written < data.size();) {
        const ssize_t size = write(fd, data.data() + written, data.size() - written);
        if (size < 0 and errno == EINTR) {
            continue;
        }
        if (size <= 0) {
            fd = -1;
            return false;
        }
        written += size;
    }
    return fd >= 0;
}

}
