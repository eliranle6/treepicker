#include <gtest/gtest.h>
#include "../paginator.h"

class TreePaginatorTest: public testing::Test
{
protected:
    int nr_items_removed_at_the_beginning;
    int nr_items_removed_at_the_end;
};

TEST_F(TreePaginatorTest, no_pagination_when_selecting_first_line) {
    int window_height = 10;
    int nr_lines = 5;
    int selection_idx = 0;

    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_lines, window_height, selection_idx);

    ASSERT_EQ(nr_items_removed_at_the_beginning, 0);
    ASSERT_EQ(nr_items_removed_at_the_end, 0);
}

TEST_F(TreePaginatorTest, no_pagination_when_selecting_last_line) {
    int window_height = 10;
    int nr_lines = 5;
    int selection_idx = 4;

    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_lines, window_height, selection_idx);

    ASSERT_EQ(nr_items_removed_at_the_beginning, 0);
    ASSERT_EQ(nr_items_removed_at_the_end, 0);
}

TEST_F(TreePaginatorTest, no_pagination_when_selecting_some_line_in_the_middle) {
    int window_height = 10;
    int nr_lines = 5;
    int selection_idx = 2;

    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_lines, window_height, selection_idx);

    ASSERT_EQ(nr_items_removed_at_the_beginning, 0);
    ASSERT_EQ(nr_items_removed_at_the_end, 0);
}

TEST_F(TreePaginatorTest, paginate) {
    int window_height = 5;
    int nr_lines = 10;
    int selection_idx = 2;

    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_lines, window_height, selection_idx);

    ASSERT_EQ(nr_items_removed_at_the_beginning, 0);
    ASSERT_EQ(nr_items_removed_at_the_end, 5);
}
