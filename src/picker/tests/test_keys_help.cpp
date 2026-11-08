#include <set>
#include <sstream>
#include <string>
#include "gtest/gtest.h"

#include "picker/cli.h"
#include "picker/keys_help.h"
#include "picker/tree_keyboard_selector.h"
#include "utils/guishell.h"

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

static std::vector<std::string> help_tags(const picker::PickerOptions& options) {
    auto tree = picker::keys_help_tree(options);
    std::vector<std::string> tags;
    for (auto& node : tree->get_root()->children) {
        tags.push_back(node->tag);
    }
    return tags;
}

static bool has_line(const std::vector<std::string>& tags, const std::string& text) {
    for (auto& tag : tags) {
        if (tag.find(text) != std::string::npos) {
            return true;
        }
    }
    return false;
}

TEST(keys_help_tree, lists_every_key_and_every_line_of_its_description) {
    const auto tags = help_tags(picker::PickerOptions());
    for (auto& line : picker::key_lines()) {
        // Each word, as a long line is wrapped
        std::istringstream words(line.description);
        for (std::string word; words >> word;) {
            ASSERT_TRUE(has_line(tags, word)) << word;
        }
        ASSERT_TRUE(has_line(tags, "  " + line.keys + " ")) << line.keys;
    }
}

TEST(keys_help_tree, lists_the_keys_handled_outside_the_table) {
    const auto help = picker::keys_help();
    for (auto keys : {"Backspace", "Ctrl-U, Ctrl-W", "a state's key"}) {
        ASSERT_NE(help.find(keys), std::string::npos) << keys;
    }
}

TEST(keys_help_tree, starts_with_the_callers_keys_and_lines_then_its_side_trees) {
    picker::PickerOptions options;
    options.key_hints = {{"#", "hashing"}};
    options.help_lines = {"Tags:", "  [x]  what x is"};
    options.side_key_hints = {{"d", "remove"}};
    const auto tags = help_tags(options);
    ASSERT_EQ(std::vector<std::string>(tags.begin(), tags.begin() + 8), std::vector<std::string>({
        "This screen's:", "  #                hashing", "", "Tags:", "  [x]  what x is", "",
        "This screen's side tree:", "  d                remove"}));
}

TEST(keys_help_tree, lists_the_states_keys) {
    picker::PickerOptions options;
    options.states = {{"skip", " ", "", 's'}, {"copy", ">", "green", '>'}, {"untouched", "-", "", 0}};
    const auto tags = help_tags(options);
    // A line with the key, then what its state is
    auto has_key = [&](const std::string& key, const std::string& name) {
        for (auto& tag : tags) {
            std::istringstream words(tag);
            std::string first, second;
            if (words >> first >> second and first == key and second == name) {
                return true;
            }
        }
        return false;
    };
    ASSERT_TRUE(has_key("s", "skip"));
    ASSERT_TRUE(has_key(">", "copy"));
    ASSERT_FALSE(has_line(tags, "untouched"));
}

TEST(keys_help_tree, moving_skips_only_the_headings) {
    auto tree = picker::keys_help_tree(picker::PickerOptions());
    for (auto& node : tree->get_root()->children) {
        // The keys' lines are indented, the headings aren't
        const bool is_heading = node->tag.empty() or node->tag[0] != ' ';
        ASSERT_EQ(node->is_skipped, is_heading) << node->tag;
    }
}

TEST(keys_help_tree, wraps_long_lines_to_fit_an_80_column_screen) {
    picker::PickerOptions options;
    options.key_hints = {{"#", std::string(30, 'x') + " " + std::string(40, 'y') + " the end"}};
    options.help_lines = {"  " + std::string(50, 'a') + " " + std::string(30, 'b'),
                          "  k                " + std::string(40, 'c') + " " + std::string(30, 'd')};
    const auto tags = help_tags(options);
    for (auto& tag : tags) {
        ASSERT_LE(guishell::display_width(tag), 73) << tag;
    }
    // The rest under the description, or the line's text
    ASSERT_TRUE(has_line(tags, std::string(19, ' ') + std::string(40, 'y')));
    ASSERT_TRUE(has_line(tags, "    " + std::string(30, 'b')));
    // A line like a key's: under its description
    ASSERT_TRUE(has_line(tags, std::string(19, ' ') + std::string(30, 'd')));
}
