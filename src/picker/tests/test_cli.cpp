#include <sstream>
#include <vector>
#include "gtest/gtest.h"

#include "picker/cli.h"

static picker::CliOptions parse(std::vector<const char*> args) {
    args.insert(args.begin(), "program");
    return picker::parse_cli(args.size(), const_cast<char**>(args.data()));
}

TEST(cli, defaults) {
    auto options = parse({});
    ASSERT_FALSE(options.is_null_separated);
    ASSERT_FALSE(options.is_help_requested);
    ASSERT_TRUE(options.error.empty());
    ASSERT_TRUE(options.args.empty());
}

TEST(cli, options_and_args) {
    auto options = parse({"-0", "some/dir"});
    ASSERT_TRUE(options.is_null_separated);
    ASSERT_EQ(options.args, std::vector<std::string>({"some/dir"}));
}

TEST(cli, dash_is_an_arg) {
    // "-" means stdin, so it's an argument, not an option
    auto options = parse({"-"});
    ASSERT_TRUE(options.error.empty());
    ASSERT_EQ(options.args, std::vector<std::string>({"-"}));
}

TEST(cli, help) {
    ASSERT_TRUE(parse({"-h"}).is_help_requested);
    ASSERT_TRUE(parse({"--help"}).is_help_requested);
}

TEST(cli, unknown_option) {
    auto options = parse({"--no-such-option"});
    ASSERT_NE(options.error.find("--no-such-option"), std::string::npos);
}

TEST(cli, multi) {
    ASSERT_FALSE(parse({}).is_multi);
    ASSERT_TRUE(parse({"-m"}).is_multi);
    ASSERT_TRUE(parse({"--multi"}).is_multi);
}

TEST(cli, header) {
    auto options = parse({"--header", "line 1\nline 2", "-"});
    ASSERT_EQ(options.header, "line 1\nline 2");
    ASSERT_EQ(options.args, std::vector<std::string>({"-"}));
}

TEST(cli, header_without_value) {
    ASSERT_FALSE(parse({"--header"}).error.empty());
}

TEST(cli, root_name) {
    ASSERT_EQ(parse({"--root", "/some/dir"}).root_name, "/some/dir");
    ASSERT_FALSE(parse({"--root"}).error.empty());
}

TEST(cli, list) {
    ASSERT_FALSE(parse({}).is_list);
    ASSERT_TRUE(parse({"--list"}).is_list);
}

TEST(cli, add_list_from_stream) {
    // Labels may have '/' in them: they're not paths
    treelib::Tree tree;
    std::istringstream in("Copy from /a to /b\n\nQuit\nQuit\n");
    picker::add_list_from_stream(tree, in, '\n');

    auto& options = tree.get_root()->children;
    ASSERT_EQ(options.size(), 2);
    ASSERT_EQ(options[0]->tag, "Copy from /a to /b");
    ASSERT_EQ(options[0]->identifier, "Copy from /a to /b");
    ASSERT_EQ(options[1]->tag, "Quit");
}

TEST(cli, max_depth) {
    ASSERT_EQ(parse({}).max_depth, -1);
    ASSERT_EQ(parse({"--max-depth", "3", "dir"}).max_depth, 3);
    ASSERT_EQ(parse({"--max-depth", "3", "dir"}).args, std::vector<std::string>({"dir"}));
    ASSERT_FALSE(parse({"--max-depth"}).error.empty());
    ASSERT_FALSE(parse({"--max-depth", "x"}).error.empty());
    ASSERT_FALSE(parse({"--max-depth", "-1"}).error.empty());
    ASSERT_FALSE(parse({"--max-depth", ""}).error.empty());
}

TEST(cli, updates) {
    ASSERT_EQ(parse({}).updates_fd, -1);
    ASSERT_EQ(parse({"--updates", "5", "--list"}).updates_fd, 5);
    ASSERT_TRUE(parse({"--updates", "5", "--list"}).is_list);
    ASSERT_FALSE(parse({"--updates"}).error.empty());
    ASSERT_FALSE(parse({"--updates", "x"}).error.empty());
    ASSERT_NE(parse({"--updates", "-1"}).error.find("--updates"), std::string::npos);
}

TEST(cli, scan_errors_warning) {
    ASSERT_EQ(picker::scan_errors_warning("/d", {}), "");
    ASSERT_EQ(picker::scan_errors_warning("/d", {"find: '/d/a': Permission denied"}),
              "Warning: find couldn't read everything in /d (1 errors), so the tree may lack files:\n"
              "  find: '/d/a': Permission denied");
}

TEST(cli, scan_errors_warning_lists_the_first_errors) {
    auto warning = picker::scan_errors_warning("/d", {"e1", "e2", "e3", "e4", "e5"});
    ASSERT_EQ(warning.substr(warning.find('\n')), "\n  e1\n  e2\n  e3\n  (and 2 more)");
}

TEST(cli, picked_file) {
    ASSERT_EQ(parse({"--picked", "/tmp/picked"}).picked_file, "/tmp/picked");
    ASSERT_FALSE(parse({"--picked"}).error.empty());
}

TEST(cli, label) {
    auto options = parse({"--label", "DRY RUN", "--label-color", "green"});
    ASSERT_EQ(options.labels.size(), 1u);
    ASSERT_EQ(options.labels[0].text, "DRY RUN");
    ASSERT_EQ(options.labels[0].color, "green");
    ASSERT_TRUE(options.error.empty());
    ASSERT_TRUE(parse({}).labels.empty());
}

TEST(cli, labels_each_with_its_color) {
    auto options = parse({"--label", "LIVE", "--label-color", "red", "--label", "COPY"});
    ASSERT_EQ(options.labels.size(), 2u);
    ASSERT_EQ(options.labels[0].color, "red");
    ASSERT_EQ(options.labels[1].text, "COPY");
    ASSERT_EQ(options.labels[1].color, "blue");
}

TEST(cli, unknown_label_color) {
    ASSERT_NE(parse({"--label", "A", "--label-color", "plaid"}).error.find("plaid"),
              std::string::npos);
}

TEST(cli, label_color_without_a_label) {
    ASSERT_FALSE(parse({"--label-color", "red"}).error.empty());
}

TEST(cli, dirs_only_and_lazy) {
    ASSERT_FALSE(parse({}).is_dirs_only);
    ASSERT_FALSE(parse({}).is_lazy);
    auto options = parse({"--dirs-only", "--lazy", "/"});
    ASSERT_TRUE(options.is_dirs_only);
    ASSERT_TRUE(options.is_lazy);
    ASSERT_EQ(options.args, std::vector<std::string>({"/"}));
}

TEST(cli, typed) {
    ASSERT_FALSE(parse({}).is_typed);
    ASSERT_TRUE(parse({"--typed", "-"}).is_typed);
}

TEST(cli, expect) {
    ASSERT_EQ(parse({}).expect_keys, "");
    ASSERT_EQ(parse({"--expect", "r", "-"}).expect_keys, "r");
    ASSERT_EQ(parse({"--expect", "r", "--expect", "x1"}).expect_keys, "rx1");
    ASSERT_TRUE(parse({"--expect", "rx"}).error.empty());
    ASSERT_FALSE(parse({"--expect"}).error.empty());
}

TEST(cli, expect_keys_cant_be_treepickers_or_unprintable) {
    ASSERT_NE(parse({"--expect", "rj"}).error.find("'j'"), std::string::npos);
    ASSERT_FALSE(parse({"--expect", "q"}).error.empty());
    ASSERT_FALSE(parse({"--expect", "?"}).error.empty());
    ASSERT_FALSE(parse({"--expect", " "}).error.empty());
    ASSERT_FALSE(parse({"--expect", "\x7f"}).error.empty());
}

TEST(cli, select) {
    ASSERT_EQ(parse({}).selected_path, "");
    ASSERT_EQ(parse({"--select", "a/b", "-"}).selected_path, "a/b");
    ASSERT_EQ(parse({"--select", "a/b", "-"}).args, std::vector<std::string>({"-"}));
    ASSERT_FALSE(parse({"--select"}).error.empty());
}

TEST(cli, expect_and_select_are_in_the_usage) {
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--expect KEYS"), std::string::npos);
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--select PATH"), std::string::npos);
}

TEST(cli, print_selection) {
    ASSERT_FALSE(parse({}).is_selection_printed);
    ASSERT_TRUE(parse({"--print-selection", "-"}).is_selection_printed);
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--print-selection"), std::string::npos);
}

TEST(cli, enter_picks_selection) {
    ASSERT_FALSE(parse({}).is_selection_picked_if_none);
    ASSERT_TRUE(parse({"-m", "--enter-picks-selection", "-"}).is_selection_picked_if_none);
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--enter-picks-selection"), std::string::npos);
}

TEST(cli, selection) {
    ASSERT_EQ(parse({}).selection_fd, -1);
    ASSERT_EQ(parse({"--selection", "7", "-"}).selection_fd, 7);
    ASSERT_FALSE(parse({"--selection"}).error.empty());
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--selection FD"), std::string::npos);
}

TEST(cli, notify) {
    auto options = parse({"--notify", "\t", "--notify-fd", "8", "-"});
    ASSERT_EQ(options.notify_keys, "\t");
    ASSERT_EQ(options.notify_fd, 8);
    ASSERT_TRUE(options.error.empty());
    // Not one of treepicker's keys
    ASSERT_FALSE(parse({"--notify", "j"}).error.empty());
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--notify KEYS"), std::string::npos);
}

TEST(cli, preview) {
    ASSERT_EQ(parse({}).preview_fd, -1);
    ASSERT_EQ(parse({"--preview", "5", "-"}).preview_fd, 5);
    ASSERT_EQ(parse({"--preview", "5", "-"}).args, std::vector<std::string>({"-"}));
    ASSERT_FALSE(parse({"--preview"}).error.empty());
    ASSERT_FALSE(parse({"--preview", "file.txt"}).error.empty());
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find("--preview FD"), std::string::npos);
}

TEST(cli, keys_and_help_lines) {
    auto options = parse({"--key", "#", "hashing", "--key", "Enter", "act", "--help-line", "[x]: y"});
    ASSERT_TRUE(options.error.empty());
    ASSERT_EQ(options.key_hints, std::vector<picker::KeyHint>({{"#", "hashing"}, {"Enter", "act"}}));
    ASSERT_EQ(options.help_lines, std::vector<std::string>({"[x]: y"}));
    ASSERT_FALSE(parse({"--key", "#"}).error.empty());
}

TEST(cli, expect_twice) {
    ASSERT_EQ(parse({"--expect-twice", "c", "-"}).expect_twice_keys, "c");
    ASSERT_TRUE(parse({"--expect-twice", "c"}).error.empty());
    ASSERT_FALSE(parse({"--expect-twice", "j"}).error.empty());
}

TEST(cli, expect_a_control_key) {
    // Ctrl-R
    ASSERT_TRUE(parse({"--expect", "\x12"}).error.empty());
    // Ctrl-C and Ctrl-D are treepicker's
    ASSERT_FALSE(parse({"--expect", "\x03"}).error.empty());
    ASSERT_FALSE(parse({"--expect", "\x04"}).error.empty());
}
