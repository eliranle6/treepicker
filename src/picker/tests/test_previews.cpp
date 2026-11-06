#include <string>
#include "gtest/gtest.h"

#include "picker/previews.h"

using Previews = std::vector<picker::Preview>;

TEST(previews, each_is_a_paths_lines) {
    picker::PreviewsReader reader('\n');
    ASSERT_EQ(reader.feed("a/x\tIn b:\t  y/x\nc\tNone\n"),
              Previews({{"a/x", {"In b:", "  y/x"}}, {"c", {"None"}}}));
}

TEST(previews, one_split_between_reads_comes_when_it_ends) {
    picker::PreviewsReader reader('\n');
    ASSERT_EQ(reader.feed("a\tone\tt"), Previews());
    ASSERT_EQ(reader.feed("wo\nb"), Previews({{"a", {"one", "two"}}}));
    ASSERT_EQ(reader.feed("\tB\n"), Previews({{"b", {"B"}}}));
}

TEST(previews, null_delimited) {
    picker::PreviewsReader reader('\0');
    ASSERT_EQ(reader.feed(std::string("a\nb\tone\ttwo\0c\t\0", 16)),
              Previews({{"a\nb", {"one", "two"}}, {"c", {""}}}));
}

TEST(previews, a_line_may_be_highlighted) {
    ASSERT_EQ(picker::parse_preview_line("\x01green\x01-> moved").text, "-> moved");
    ASSERT_EQ(picker::parse_preview_line("\x01green\x01-> moved").color, "green");
    ASSERT_EQ(picker::parse_preview_line("plain").color, "");
    ASSERT_EQ(picker::parse_preview_line("").color, "");
    // Without the second mark, it's text
    ASSERT_EQ(picker::parse_preview_line("\x01green").text, "green");
    // Its parts' marks aren't shown
    ASSERT_EQ(picker::parse_preview_line("\x01" "green\x01" "a\x01" "red\x01" "b").text, "ab");
    ASSERT_EQ(picker::parse_preview_line("\x01" "green\x01" "a\x01" "red\x01" "b").color, "green");
}

TEST(previews, ones_without_a_tab_are_skipped_and_the_path_may_be_empty) {
    picker::PreviewsReader reader('\n');
    ASSERT_EQ(reader.feed("no tab\n\tWorking...\n"), Previews({{"", {"Working..."}}}));
}

TEST(previews, an_image_replaces_the_paths_image) {
    picker::PreviewsReader reader('\n');
    const Previews previews = reader.feed("\x03" "a/x.jpg\t2 1 ff000000ff00\n\x03" "b\t\n");
    ASSERT_EQ(previews.size(), 2u);
    ASSERT_EQ(previews[0].path, "a/x.jpg");
    ASSERT_TRUE(previews[0].is_image);
    ASSERT_TRUE(previews[0].lines.empty());
    ASSERT_EQ(previews[0].image.width, 2);
    ASSERT_EQ(previews[0].image.pixels, std::vector<std::uint32_t>({0xff0000, 0x00ff00}));
    // An empty one: no image
    ASSERT_EQ(previews[1].path, "b");
    ASSERT_TRUE(previews[1].is_image);
    ASSERT_TRUE(previews[1].image.empty());
}

using Segments = std::vector<picker::Label>;

static bool same(const Segments& a, const Segments& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].text != b[i].text or a[i].color != b[i].color) {
            return false;
        }
    }
    return true;
}

TEST(previews, a_line_below_the_tree_in_colored_parts) {
    // "\x01" apart from what follows it, which may be a hex digit
    ASSERT_TRUE(same(picker::parse_segments("/root/\x01" "magenta\x01" "docs/\x01\x01" "x.pdf"),
                     Segments({{"/root/", ""}, {"docs/", "magenta"}, {"x.pdf", ""}})));
    ASSERT_TRUE(same(picker::parse_segments("plain"), Segments({{"plain", ""}})));
    ASSERT_TRUE(same(picker::parse_segments(""), Segments()));
    ASSERT_TRUE(same(picker::parse_segments("a\x01" "green"), Segments({{"a", ""}, {"green", ""}})));
}
