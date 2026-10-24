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
    ASSERT_EQ(nr_items_removed_at_the_end, 6);
}

TEST_F(TreePaginatorTest, paginate_selection_at_the_end) {
    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(10, 5, 9);

    ASSERT_EQ(nr_items_removed_at_the_end, 0);
    // 5 rows: '... 6 more' and the last 4 items
    ASSERT_EQ(nr_items_removed_at_the_beginning, 6);
}

TEST_F(TreePaginatorTest, paginate_selection_in_the_middle) {
    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(20, 6, 10);

    // The selection is the last item shown
    ASSERT_EQ(nr_items_removed_at_the_end, 9);
    ASSERT_GT(nr_items_removed_at_the_beginning, 0);
    ASSERT_LE(nr_items_removed_at_the_beginning, 10);
}

TEST_F(TreePaginatorTest, invariants_for_all_small_sizes) {
    // For every number of items, page size and selection:
    // - the selected item is shown
    // - items and '... N more' rows together fit in the page
    // - nothing is hidden when everything fits
    // - a '... N more' row never hides a single item (the row could show the item instead)
    for (int page_size = 3; page_size <= 15; ++page_size) {
        for (int nr_items = 1; nr_items <= 40; ++nr_items) {
            for (int selected = 0; selected < nr_items; ++selected) {
                SCOPED_TRACE("page_size=" + std::to_string(page_size) + " nr_items=" +
                             std::to_string(nr_items) + " selected=" + std::to_string(selected));
                int removed_at_beginning, removed_at_end;
                std::tie(removed_at_beginning, removed_at_end) =
                    paginate(nr_items, page_size, selected);

                ASSERT_GE(removed_at_beginning, 0);
                ASSERT_GE(removed_at_end, 0);
                ASSERT_GE(selected, removed_at_beginning);
                ASSERT_LT(selected, nr_items - removed_at_end);
                const int nr_rows = nr_items - removed_at_beginning - removed_at_end +
                    (removed_at_beginning > 0) + (removed_at_end > 0);
                ASSERT_LE(nr_rows, page_size);
                if (nr_items <= page_size) {
                    ASSERT_EQ(removed_at_beginning + removed_at_end, 0);
                }
                ASSERT_NE(removed_at_beginning, 1);
                ASSERT_NE(removed_at_end, 1);
            }
        }
    }
}
