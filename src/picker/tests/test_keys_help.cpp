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

TEST(key_hints, treepickers_then_the_callers_then_search_help_and_quit) {
    picker::PickerOptions options;
    options.is_multi = true;
    options.key_hints = {{"Enter", "choose what to do"}, {"#", "hashing"}, {"q", "back"}};
    ASSERT_EQ(picker::key_hints(options), std::vector<picker::KeyHint>({
        {"Space", "pick"}, {"Enter", "choose what to do"}, {"#", "hashing"}, {"/", "search"},
        {"?", "all keys"}, {"q", "back"}}));
}

TEST(key_hints, says_otherwise_while_nothings_picked) {
    picker::PickerOptions options;
    options.is_multi = true;
    options.key_hints = {{"Enter", "choose what to do"}};
    options.none_picked_key_hints = {{"Enter", "Pick & choose what to do"}};
    ASSERT_EQ(picker::key_hints(options)[1], picker::KeyHint({"Enter", "choose what to do"}));
    ASSERT_EQ(picker::key_hints(options, true)[1], picker::KeyHint({"Enter", "Pick & choose what to do"}));
}

TEST(key_hints, in_the_side_tree_its_own_the_way_back_help_and_quit) {
    picker::PickerOptions options;
    options.is_multi = true;
    options.key_hints = {{"Enter", "choose what to do"}, {"q", "back"}};
    options.side_key_hints = {{"d", "remove"}};
    ASSERT_EQ(picker::key_hints(options, true, true), std::vector<picker::KeyHint>({
        {"d", "remove"}, {"Ctrl-W h", "back"}, {"?", "all keys"}, {"q", "back"}}));
}

TEST(key_hints, a_list_chooses_and_a_tree_picks) {
    picker::PickerOptions options;
    options.is_root_pickable = false;
    ASSERT_EQ(picker::key_hints(options)[0], picker::KeyHint({"Enter", "choose"}));
    ASSERT_EQ(picker::key_hints(picker::PickerOptions())[0], picker::KeyHint({"Enter", "pick"}));
}
