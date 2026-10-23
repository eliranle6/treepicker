#include <clocale>
#include <cstring>
#include <gtest/gtest.h>

#include "utils/guishell.h"

class TextWidthTest: public testing::Test
{
protected:
    void SetUp(void) override {
        // Display widths of non-ASCII characters depend on a UTF-8 locale
        ASSERT_NE(setlocale(LC_ALL, "C.UTF-8"), nullptr);
    }

    static std::size_t fit(const char* text, int nr_columns) {
        return guishell::nr_bytes_that_fit(text, strlen(text), nr_columns);
    }
};

TEST_F(TextWidthTest, ascii_that_fits) {
    ASSERT_EQ(fit("hello", 10), 5);
    ASSERT_EQ(fit("hello", 5), 5);
}

TEST_F(TextWidthTest, ascii_that_is_cut) {
    ASSERT_EQ(fit("hello", 3), 3);
    ASSERT_EQ(fit("hello", 0), 0);
}

TEST_F(TextWidthTest, hebrew_takes_one_column_per_letter) {
    // Each letter is 2 bytes
    ASSERT_EQ(fit("\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d", 10), 8);
    ASSERT_EQ(fit("\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d", 2), 4);
}

TEST_F(TextWidthTest, wide_characters_take_two_columns_and_arent_split) {
    // Two CJK characters, 3 bytes and 2 columns each
    const char* text = "\xe4\xb8\xad\xe6\x96\x87";
    ASSERT_EQ(fit(text, 4), 6);
    ASSERT_EQ(fit(text, 3), 3);
    ASSERT_EQ(fit(text, 1), 0);
}

TEST_F(TextWidthTest, box_drawing_characters_take_one_column) {
    // "├── x": the tree's own connector characters
    ASSERT_EQ(fit("\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 x", 5), 11);
    ASSERT_EQ(fit("\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 x", 4), 10);
}

TEST_F(TextWidthTest, invalid_utf8_counts_as_one_column_per_byte) {
    ASSERT_EQ(fit("a\xff\xfe" "b", 10), 4);
    ASSERT_EQ(fit("a\xff\xfe" "b", 2), 2);
}

TEST_F(TextWidthTest, character_cut_at_the_end_of_the_buffer) {
    // A buffer can end in the middle of a character (the stream flushes in chunks)
    ASSERT_EQ(guishell::nr_bytes_that_fit("a\xd7\xa9", 2, 10), 2);
}

TEST_F(TextWidthTest, display_width) {
    ASSERT_EQ(guishell::display_width(""), 0);
    ASSERT_EQ(guishell::display_width("hello"), 5);
    ASSERT_EQ(guishell::display_width("\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d"), 4);
    ASSERT_EQ(guishell::display_width("\xe4\xb8\xad\xe6\x96\x87"), 4);
}
