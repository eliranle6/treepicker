#include <algorithm>
#include <sstream>

#include "picker/keys_help.h"
#include "picker/tree_keyboard_selector.h"
#include "utils/guishell.h"

namespace picker {

// The keys' column's width, the descriptions in a column after it
static constexpr std::size_t KEYS_WIDTH = 17;

// The groups, in the help's order, and the keys handled outside get_key_commands()'s table (a
// function's statics, so they're there for cli's usage text, made while statics are initialized)
static const std::vector<std::string>& groups(void) {
    static const std::vector<std::string> groups = {
        "Moving", "Searching", "Picking", "Showing", "The side tree", "Help and quitting"};
    return groups;
}

static const std::vector<KeyLine>& other_key_lines(void) {
    static const std::vector<KeyLine> lines = {
        {"Searching", "Enter", "while typing: done (keeps the filter)"},
        {"Searching", "Esc", "while typing: done, dropping the filter"},
        {"Searching", "Backspace", "while typing: erase a character (none left: done)"},
        {"Searching", "Ctrl-U, Ctrl-W", "while typing: erase the keyword"},
        {"Picking", "a state's key", "put the selection in that state (with --state KEY; a dir: all under it)"},
        {"The side tree", "in it:", "h/j/k/l, J/K, Ctrl-J/Ctrl-K, Home/End move, Enter picks (with --side-key), Ctrl-H, Esc or Backspace: back"},
    };
    return lines;
}

std::vector<KeyLine> key_lines(void) {
    std::vector<KeyLine> lines;
    for (auto& group : groups()) {
        for (auto& command : TreeKeyboardSelector::get_key_commands()) {
            if (command.group == group) {
                lines.push_back({command.group, command.keys, command.description});
            }
        }
        for (auto& line : other_key_lines()) {
            if (line.group == group) {
                lines.push_back(line);
            }
        }
    }
    return lines;
}

// "keys            description", the keys padded to KEYS_WIDTH
static std::string key_line(const std::string& keys, const std::string& description) {
    return keys + std::string(keys.size() < KEYS_WIDTH ? KEYS_WIDTH - keys.size() : 1, ' ') + description;
}

// A line cut into lines of at most width display columns, at spaces where it can be, the ones after
// the first indented as it is and by indent more
static std::vector<std::string> wrapped(const std::string& line, std::size_t width, std::size_t indent) {
    const std::size_t own_indent = std::min(line.find_first_not_of(' '), line.size());
    std::vector<std::string> lines;
    std::string rest = line;
    while (static_cast<std::size_t>(guishell::display_width(rest)) > width) {
        std::size_t cut = guishell::nr_bytes_that_fit(rest.data(), rest.size(), width + 1);
        // A space just after what fits ends the line there too
        const std::size_t space = rest.rfind(' ', cut - 1);
        const std::size_t start = lines.empty() ? own_indent : own_indent + indent;
        if (space != std::string::npos and space > start) {
            cut = space;
        } else {
            cut = guishell::nr_bytes_that_fit(rest.data(), rest.size(), width);
        }
        lines.push_back(rest.substr(0, cut));
        const std::size_t next = rest.find_first_not_of(' ', cut);
        if (next == std::string::npos) {
            return lines;
        }
        rest = std::string(own_indent + indent, ' ') + rest.substr(next);
    }
    lines.push_back(rest);
    return lines;
}

std::string keys_help(void) {
    std::string help;
    std::string group;
    for (auto& line : key_lines()) {
        if (line.group != group) {
            group = line.group;
            help += "  " + group + ":\n";
        }
        // Fitting 80 columns, the last one left empty
        for (auto& part : wrapped("    " + key_line(line.keys, line.description), 79, KEYS_WIDTH)) {
            help += part + '\n';
        }
    }
    return help;
}

std::unique_ptr<treelib::Tree> keys_help_tree(const PickerOptions& options, std::size_t width) {
    // A list rather than a tree of groups, so all of it shows (paged as the screen allows), each
    // group after a heading that moving skips, as a menu's
    std::unique_ptr<treelib::Tree> tree(new treelib::Tree());
    const std::string root = tree->create_node("Keys", "0", "")->identifier;
    int nr_nodes = 1;
    auto add = [&](const std::string& tag, bool is_skipped = false, const std::string& color = "") {
        auto node = tree->create_node(tag, std::to_string(nr_nodes++), root);
        node->is_skipped = is_skipped;
        node->color = color;
    };
    auto add_heading = [&](const std::string& heading) {
        if (nr_nodes > 1) {
            add("", true);
        }
        add(heading + ":", true, "cyan");
    };
    // A key's line, wrapped under its description
    auto add_key = [&](const std::string& keys, const std::string& description) {
        for (auto& part : wrapped("  " + key_line(keys, description), width, KEYS_WIDTH)) {
            add(part);
        }
    };
    if (not options.key_hints.empty() or not options.help_lines.empty()) {
        add_heading("This screen's");
        for (auto& hint : options.key_hints) {
            add_key(hint.key, hint.text);
        }
        // A line that isn't indented and ends with ':' is a heading, as treepicker's groups'
        for (auto& line : options.help_lines) {
            if (not line.empty() and line[0] != ' ' and line.back() == ':') {
                add_heading(line.substr(0, line.size() - 1));
            } else {
                // Under the text after its first gap of two spaces (e.g. a key's description)
                const std::size_t start = line.find_first_not_of(' ');
                const std::size_t gap = line.find("  ", start);
                const std::size_t text = gap == std::string::npos ? std::string::npos : line.find_first_not_of(' ', gap);
                const std::size_t hang = text == std::string::npos or text - start > width / 2 ? 2 : text - start;
                for (auto& part : wrapped(line, width, hang)) {
                    add(part);
                }
            }
        }
    }
    if (not options.side_key_hints.empty()) {
        add_heading("This screen's side tree");
        for (auto& hint : options.side_key_hints) {
            add_key(hint.key, hint.text);
        }
    }
    std::string group;
    for (auto& line : key_lines()) {
        if (line.group != group) {
            group = line.group;
            add_heading(group);
        }
        add_key(line.keys, line.description);
        // The states' own keys, after the line that says what they are
        if (line.keys == "a state's key") {
            for (auto& state : options.states) {
                if (state.key != 0) {
                    add_key("  " + std::string(1, static_cast<char>(state.key)), state.name);
                }
            }
        }
    }
    return tree;
}

std::vector<KeyHint> key_hints(const PickerOptions& options, bool is_none_picked, bool is_side_focused) {
    std::vector<KeyHint> hints;
    if (is_side_focused) {
        hints = options.side_key_hints;
        hints.push_back({"Ctrl-W h", "back"});
        hints.push_back({"?", "all keys"});
        std::string quit = "quit";
        for (auto& hint : options.key_hints) {
            if (hint.key == "q") {
                quit = hint.text;
            }
        }
        hints.push_back({"q", quit});
        return hints;
    }
    if (options.is_multi) {
        hints.push_back({"Space", options.states.empty() ? "pick" : "next state"});
        hints.push_back({"Enter", "done"});
        // The keys that set a state
        for (auto& state : options.states) {
            if (state.key != 0) {
                hints.push_back({std::string(1, static_cast<char>(state.key)), state.name});
            }
        }
        if (not options.tabs.empty()) {
            hints.push_back({"Ctrl-L/H", "tabs"});
        }
    } else {
        hints.push_back({"Enter", options.is_root_pickable ? "pick" : "choose"});
    }
    std::vector<KeyHint> tail = {{"/", "search"}, {"?", "all keys"}, {"q", "quit"}};
    for (auto& hint : options.key_hints) {
        bool is_own = false;
        for (auto* own : {&hints, &tail}) {
            for (auto& existing : *own) {
                if (existing.key == hint.key) {
                    existing.text = hint.text;
                    is_own = true;
                }
            }
        }
        if (not is_own) {
            hints.push_back(hint);
        }
    }
    hints.insert(hints.end(), tail.begin(), tail.end());
    if (is_none_picked) {
        for (auto& override_hint : options.none_picked_key_hints) {
            for (auto& hint : hints) {
                if (hint.key == override_hint.key) {
                    hint.text = override_hint.text;
                }
            }
        }
    }
    return hints;
}

} // namespace picker
