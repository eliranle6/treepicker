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
    ASSERT_EQ(picker::parse_preview_line("\x01green\x01-> moved").highlight, "green");
    ASSERT_EQ(picker::parse_preview_line("plain").highlight, "");
    // Without the second mark, it's text (shown with '?' for the mark)
    ASSERT_EQ(picker::parse_preview_line("\x01green").text, "\x01green");
}

TEST(previews, ones_without_a_tab_are_skipped_and_the_path_may_be_empty) {
    picker::PreviewsReader reader('\n');
    ASSERT_EQ(reader.feed("no tab\n\tWorking...\n"), Previews({{"", {"Working..."}}}));
}

using Segments = std::vector<picker::PreviewLine>;

static bool same(const Segments& a, const Segments& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].text != b[i].text or a[i].highlight != b[i].highlight) {
            return false;
        }
    }
    return true;
}

TEST(previews, a_line_below_the_tree_in_colored_parts) {
    // "\x01" apart from what follows it, which may be a hex digit
    ASSERT_TRUE(same(picker::parse_preview_segments("/root/\x01" "magenta\x01" "docs/\x01\x01" "x.pdf"),
                     Segments({{"/root/", ""}, {"docs/", "magenta"}, {"x.pdf", ""}})));
    ASSERT_TRUE(same(picker::parse_preview_segments("plain"), Segments({{"plain", ""}})));
    ASSERT_TRUE(same(picker::parse_preview_segments(""), Segments()));
    ASSERT_TRUE(same(picker::parse_preview_segments("a\x01" "green"), Segments({{"a", ""}, {"green", ""}})));
}
