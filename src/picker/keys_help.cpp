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

std::vector<KeyHint> key_hints(const PickerOptions& options) {
    std::vector<KeyHint> hints;
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
    return hints;
}

} // namespace picker
