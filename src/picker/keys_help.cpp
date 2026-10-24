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

} // namespace picker
