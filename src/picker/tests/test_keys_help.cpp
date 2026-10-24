#include <set>
#include <sstream>
#include <string>
#include "gtest/gtest.h"

#include "picker/cli.h"
#include "picker/keys_help.h"
#include "picker/tree_keyboard_selector.h"

TEST(keys_help, fits_an_80_column_terminal) {
    // The picker shows it as is (the ? key), and the screen's last column stays empty
    std::istringstream keys(picker::keys_help());
    for (std::string line; std::getline(keys, line);) {
        ASSERT_LE(line.size(), 79) << line;
    }
}

TEST(keys_help, lists_the_help_key) {
    ASSERT_NE(picker::keys_help().find("  ?  "), std::string::npos);
}

TEST(keys_help, is_part_of_the_usage) {
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find(picker::keys_help()), std::string::npos);
}

TEST(keys_help, keys_do_one_thing) {
    // A key bound twice would do only the first command's action
    std::set<int> keys;
    for (auto& command : picker::TreeKeyboardSelector::get_key_commands()) {
        for (auto& binding : command.bindings) {
            ASSERT_TRUE(keys.insert(binding.first).second) << command.keys;
        }
    }
}
