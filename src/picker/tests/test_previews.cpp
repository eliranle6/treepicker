#include <sstream>
#include <string>
#include "gtest/gtest.h"

#include "picker/previews.h"

using Lines = std::vector<std::string>;

TEST(previews, each_line_is_a_paths_preview) {
    std::istringstream in("a/x\tIn b:\t  y/x\nc\tNone\n");
    const auto previews = picker::read_previews(in, '\n');
    ASSERT_EQ(previews.size(), 2u);
    ASSERT_EQ(previews.at("a/x"), Lines({"In b:", "  y/x"}));
    ASSERT_EQ(previews.at("c"), Lines({"None"}));
}

TEST(previews, null_delimited) {
    std::istringstream in(std::string("a\nb\tone\ttwo\0c\t\0", 16));
    const auto previews = picker::read_previews(in, '\0');
    ASSERT_EQ(previews.at("a\nb"), Lines({"one", "two"}));
    ASSERT_EQ(previews.at("c"), Lines({""}));
}

TEST(previews, ones_without_a_tab_are_skipped_and_the_last_one_counts) {
    std::istringstream in("no tab\na\tfirst\na\tlast\n");
    const auto previews = picker::read_previews(in, '\n');
    ASSERT_EQ(previews.size(), 1u);
    ASSERT_EQ(previews.at("a"), Lines({"last"}));
}
