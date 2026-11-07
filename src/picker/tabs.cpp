#include <algorithm>

#include "picker/tabs.h"

namespace picker {

// Whether the tab shows the group's nodes
static bool does_show(const TabOption& tab, int group) {
    return tab.groups.empty() or std::find(tab.groups.begin(), tab.groups.end(), group) != tab.groups.end();
}

Tabs::Tabs(treelib::Tree& tree, const PickerOptions& options):
    _tabs(options.tabs),
    _counts(options.tabs.size(), 0)
{
    if (_tabs.empty() or tree.get_root() == nullptr) {
        return;
    }
    std::vector<treelib::Node*> dfs_stack = {tree.get_root().get()};
    while (not dfs_stack.empty()) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        if (node->children.empty() and node != tree.get_root().get()) {
            for (std::size_t i = 0; i < _tabs.size(); ++i) {
                _counts[i] += does_show(_tabs[i], node->group);
            }
        }
        for (auto& child : node->children) {
            dfs_stack.push_back(child.get());
        }
    }
}

void Tabs::move(int step) {
    if (not _tabs.empty()) {
        const int size = static_cast<int>(_tabs.size());
        _current = static_cast<std::size_t>(((static_cast<int>(_current) + step) % size + size) % size);
    }
}

bool Tabs::is_filtering(void) const {
    return not _tabs.empty() and not _tabs[_current].groups.empty();
}

std::function<bool(const treelib::Node&)> Tabs::get_filter(void) const {
    if (not is_filtering()) {
        return nullptr;
    }
    const TabOption tab = _tabs[_current];
    return [tab](const treelib::Node& node) { return does_show(tab, node.group); };
}

std::vector<std::string> Tabs::get_titles(void) const {
    std::vector<std::string> titles;
    for (std::size_t i = 0; i < _tabs.size(); ++i) {
        titles.push_back(_tabs[i].title + " " + std::to_string(_counts[i]));
    }
    return titles;
}

} // namespace picker
