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
