#include "picker/keys_help.h"
#include "picker/tree_keyboard_selector.h"

namespace picker {

std::string keys_help(void) {
    // "  keys            description", the descriptions in a column
    static const std::size_t KEYS_WIDTH = 17;
    const std::string indent = "  ";
    std::string help;
    for (auto& command : TreeKeyboardSelector::get_key_commands()) {
        std::string keys = command.keys;
        help += indent + keys + std::string(KEYS_WIDTH - keys.size(), ' ');
        for (const char* c = command.description; *c != '\0'; ++c) {
            help += *c;
            if (*c == '\n') {
                help += indent + std::string(KEYS_WIDTH, ' ');
            }
        }
        help += '\n';
    }
    return help;
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
        hints.push_back({"Space", "pick"});
        hints.push_back({"Enter", "done"});
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
