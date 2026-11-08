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

TEST(updates, an_empty_id_is_the_status) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("\tClaude: 3 files\n"), Updates({{"", "Claude: 3 files", false}}));
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

TEST(updates, a_color_before_disabled) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tMove\tcolor=blue\nb\tRemove\tcolor=red\tdisabled\nc\tA\tB\n"),
              Updates({{"a", "Move", false, "blue"}, {"b", "Remove", true, "red"}, {"c", "A\tB", false, ""}}));
}

using Labels = std::vector<picker::Label>;

namespace picker {
// For comparing vectors of them (found by argument-dependent lookup)
bool operator==(const Label& a, const Label& b) {
    return a.text == b.text and a.color == b.color;
}
}

TEST(status, plain_text_is_one_yellow_label) {
    ASSERT_EQ(picker::parse_status("Claude: 3 files"), Labels({{"Claude: 3 files", "yellow"}}));
}

TEST(status, plain_text_before_a_mark_is_yellow_too) {
    ASSERT_EQ(picker::parse_status("a\x01" "green\x01" "b"), Labels({{"a", "yellow"}, {"b", "green"}}));
}

TEST(status, empty_is_none) {
    ASSERT_EQ(picker::parse_status(""), Labels());
}

// "\x01" apart from what follows it, which may be a hex digit (\x01a is one character)
TEST(status, marked_parts_are_labels_of_their_colors) {
    ASSERT_EQ(picker::parse_status("\x01" "green\x01" "a hashed\x01" "orange\x01" "b 40% hashed"),
              Labels({{"a hashed", "green"}, {"b 40% hashed", "orange"}}));
}

TEST(status, an_unknown_color_is_yellow_and_empty_parts_are_dropped) {
    ASSERT_EQ(picker::parse_status("\x01" "pink\x01" "a\x01" "red\x01\x01" "green\x01"),
              Labels({{"a", "yellow"}}));
}

TEST(status, a_mark_without_its_closing_mark_is_followed_by_text) {
    ASSERT_EQ(picker::parse_status("\x01" "green\x01" "a\x01" "red"), Labels({{"a", "green"}, {"red", "green"}}));
}

TEST(updates, a_skipped_node_is_disabled_too) {
    picker::UpdatesReader reader('\n');
    picker::Update skipped{"a", "-- A --", true, "", true};
    ASSERT_EQ(reader.feed("a\t-- A --\tskipped\n"), Updates({skipped}));
}

TEST(updates, a_state_to_put_the_node_in) {
    picker::UpdatesReader reader('\n');
    ASSERT_EQ(reader.feed("a\tA  [ai: copy]\tcolor=magenta\tstate=to-b\nb\tB\tstate=skip\n"),
              Updates({{"a", "A  [ai: copy]", false, "magenta", false, "to-b"}, {"b", "B", false, "", false, "skip"}}));
}
