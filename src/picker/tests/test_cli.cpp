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
