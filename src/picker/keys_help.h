#ifndef PICKER_KEYS_HELP_H__
#define PICKER_KEYS_HELP_H__

#include <string>

namespace picker {

// The picker's keys, a line each, made from TreeKeyboardSelector::get_key_commands(): shown by
// --help, and by the picker itself (the ? key)
std::string keys_help(void);

} // namespace picker

#endif
