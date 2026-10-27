#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "picker/updates.h"

using Updates = std::vector<picker::Update>;

TEST(updates, each_update_sets_an_ids_tag) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tA (3 files)\nb\tB (loading...)\n"),
              Updates({{"a", "A (3 files)", false}, {"b", "B (loading...)", false}}));
}

TEST(updates, an_update_split_between_reads_comes_when_it_ends) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tA (3"), Updates());
    ASSERT_EQ(reader.feed(" files)\nb\t"), Updates({{"a", "A (3 files)", false}}));
    ASSERT_EQ(reader.feed("B\n"), Updates({{"b", "B", false}}));
}

TEST(updates, null_delimited) {
    picker::UpdatesReader reader('\0');
    ASSERT_EQ(reader.feed(std::string("a\tA\nmore\0b\tB\0", 14)),
              Updates({{"a", "A\nmore", false}, {"b", "B", false}}));
}

TEST(updates, the_tag_may_have_tabs_and_be_empty) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tA\tB\nb\t\n"), Updates({{"a", "A\tB", false}, {"b", "", false}}));
}

TEST(updates, ones_without_a_tab_are_skipped) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("nonsense\n\na\tA\n"), Updates({{"a", "A", false}}));
}

TEST(updates, disabled) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tA (nothing to do)\tdisabled\nb\tB\tdisabled more\n"),
              Updates({{"a", "A (nothing to do)", true}, {"b", "B\tdisabled more", false}}));
}
