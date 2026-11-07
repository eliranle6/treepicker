#include "picker/states.h"
#include "utils/guishell.h"

namespace picker {

States::States(treelib::Tree& tree, TreeSelector& selector, const PickerOptions& options):
    _tree(tree),
    _selector(selector),
    _states(options.states)
{
    if (_states.empty() or tree.get_root() == nullptr) {
        return;
    }
    const std::size_t nr_groups = options.groups.size();
    for (auto& state : _states) {
        std::vector<treelib::StateMark> marks(nr_groups + 1);
        marks[0] = {state.mark, guishell::text_style(state.color)};
        for (auto& group_mark : state.group_marks) {
            if (group_mark.group >= 0 and static_cast<std::size_t>(group_mark.group) < nr_groups) {
                marks[group_mark.group + 1] = {group_mark.mark, guishell::text_style(group_mark.color)};
            }
        }
        _marks.push_back(marks);
    }
    _counts.assign(_states.size(), std::vector<std::size_t>(nr_groups + 1, 0));

    // Each node's allowed states and group, then all in the first state, counted
    for (auto& node_states : options.node_states) {
        auto node = tree.get_node(node_states.id);
        if (node != nullptr) {
            node->allowed_states = node_states.allowed;
            node->group = node_states.group >= 0 and static_cast<std::size_t>(node_states.group) < nr_groups ?
                node_states.group : -1;
        }
    }
    std::vector<treelib::Node*> dfs_stack = {tree.get_root().get()};
    while (not dfs_stack.empty()) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        node->state = 0;
        if (node->children.empty()) {
            ++_counts[0][node->group + 1];
        }
        for (auto& child : node->children) {
            dfs_stack.push_back(child.get());
        }
    }
    for (auto& node_states : options.node_states) {
        auto node = tree.get_node(node_states.id);
        if (node != nullptr and node->children.empty()) {
            set_one(node.get(), node_states.initial);
        }
    }
    update_around(tree.get_root());
}

void States::set_one(treelib::Node* node, int state) {
    if (state < 0 or static_cast<std::size_t>(state) >= _states.size() or node->state == state or
            not (node->allowed_states & (1u << state))) {
        return;
    }
    if (node->state >= 0) {
        --_counts[node->state][node->group + 1];
    }
    ++_counts[state][node->group + 1];
    node->state = state;
    _selector.set_picked_flag(node, state != 0);
}

void States::update_dir(treelib::Node* node) {
    if (node->children.empty()) {
        return;
    }
    // Its nodes' state, and their group (so it's shown by its mark for them), if they're the same
    int state = node->children.front()->state;
    int group = node->children.front()->group;
    for (auto& child : node->children) {
        if (child->state != state) {
            state = treelib::Node::MIXED_STATE;
        }
        if (child->group != group) {
            group = -1;
        }
    }
    node->state = state;
    node->group = group;
    _selector.update_from_children(node);
}

void States::update_around(const NodePtr& node) {
    // Children before their parents: a DFS's preorder, reversed
    std::vector<treelib::Node*> preorder;
    std::vector<treelib::Node*> dfs_stack = {node.get()};
    while (not dfs_stack.empty()) {
        auto current = dfs_stack.back();
        dfs_stack.pop_back();
        preorder.push_back(current);
        for (auto& child : current->children) {
            dfs_stack.push_back(child.get());
        }
    }
    for (auto current = preorder.rbegin(); current != preorder.rend(); ++current) {
        update_dir(*current);
    }
    for (auto current = node; current != _tree.get_root();) {
        current = _tree.get_node(current->parent);
        update_dir(current.get());
    }
}

std::vector<treelib::Node*> States::get_files_under(const NodePtr& node, bool only_shown) {
    std::vector<treelib::Node*> files;
    std::vector<treelib::Node*> dfs_stack = {node.get()};
    while (not dfs_stack.empty()) {
        auto current = dfs_stack.back();
        dfs_stack.pop_back();
        if (current->children.empty()) {
            files.push_back(current);
        }
        for (auto& child : current->children) {
            if (not only_shown or child->is_search_visible) {
                dfs_stack.push_back(child.get());
            }
        }
    }
    return files;
}

void States::set(const NodePtr& node, int state, bool only_shown) {
    if (not is_on() or node == nullptr) {
        return;
    }
    for (auto file : get_files_under(node, only_shown)) {
        set_one(file, state);
    }
    update_around(node);
}

void States::cycle(const NodePtr& node, bool only_shown) {
    if (not is_on() or node == nullptr) {
        return;
    }
    unsigned allowed = ~0u;
    for (auto file : get_files_under(node, only_shown)) {
        allowed &= file->allowed_states;
    }
    const int nr_states = static_cast<int>(_states.size());
    const int current = node->state >= 0 ? node->state : 0;
    for (int i = 1; i < nr_states; ++i) {
        const int state = (current + i) % nr_states;
        if (allowed & (1u << state)) {
            set(node, state, only_shown);
            return;
        }
    }
}

void States::reset(void) {
    if (is_on()) {
        set(_tree.get_root(), 0, false);
    }
}

int States::get_state_of_key(int key) const {
    for (std::size_t i = 0; i < _states.size(); ++i) {
        if (_states[i].key != 0 and _states[i].key == key) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const treelib::StateMark& States::get_mark(int state, int group) const {
    const auto& marks = _marks[state];
    const std::size_t index = static_cast<std::size_t>(group + 1);
    return index > 0 and index < marks.size() and not marks[index].mark.empty() ? marks[index] : marks[0];
}

std::string States::get_counts_text(void) const {
    // By mark, in the states' order, then the groups'
    std::vector<std::pair<std::string, std::size_t>> counts;
    for (std::size_t state = 1; state < _counts.size(); ++state) {
        for (std::size_t index = 0; index < _counts[state].size(); ++index) {
            if (_counts[state][index] == 0) {
                continue;
            }
            const std::string& mark = get_mark(state, static_cast<int>(index) - 1).mark;
            auto count = counts.begin();
            while (count != counts.end() and count->first != mark) {
                ++count;
            }
            if (count == counts.end()) {
                counts.push_back({mark, 0});
                count = counts.end() - 1;
            }
            count->second += _counts[state][index];
        }
    }
    std::string text;
    for (auto& count : counts) {
        text += (text.empty() ? "" : "  ") + count.first + " " + std::to_string(count.second);
    }
    return text;
}

std::vector<std::pair<std::string, States::NodePtr>> States::get_set_nodes(void) {
    std::vector<std::pair<std::string, NodePtr>> nodes;
    if (not is_on()) {
        return nodes;
    }
    std::vector<const NodePtr*> dfs_stack = {&_tree.get_root()};
    while (not dfs_stack.empty()) {
        const NodePtr& node = *dfs_stack.back();
        dfs_stack.pop_back();
        if (node->children.empty() and node->state > 0) {
            nodes.push_back({_states[node->state].name, node});
        }
        // Reversed, so the first child is popped first
        for (auto child = node->children.rbegin(); child != node->children.rend(); ++child) {
            dfs_stack.push_back(&*child);
        }
    }
    return nodes;
}

} // namespace picker
