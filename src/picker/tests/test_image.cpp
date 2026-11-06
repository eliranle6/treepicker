#include <cstdint>
#include <string>
#include "gtest/gtest.h"

#include "picker/image.h"

using Pixels = std::vector<std::uint32_t>;

TEST(image, parsed_from_its_size_and_hex_pixels) {
    const picker::Image image = picker::parse_image("2 2 000000FFFFFF102030a0b0c0");
    ASSERT_EQ(image.width, 2);
    ASSERT_EQ(image.height, 2);
    ASSERT_EQ(image.pixels, Pixels({0x000000, 0xffffff, 0x102030, 0xa0b0c0}));
    ASSERT_EQ(image.at(1, 1), 0xa0b0c0u);
}

TEST(image, what_isnt_one_is_none) {
    ASSERT_TRUE(picker::parse_image("").empty());
    ASSERT_TRUE(picker::parse_image("2 2 000000").empty());
    ASSERT_TRUE(picker::parse_image("1 1 00000g").empty());
    ASSERT_TRUE(picker::parse_image("0 1 ").empty());
    ASSERT_TRUE(picker::parse_image("x 1 000000").empty());
    ASSERT_TRUE(picker::parse_image("1 1 0000000").empty());
    // Too big to be sent whole
    ASSERT_TRUE(picker::parse_image("100000 100000 000000").empty());
}

TEST(image, one_that_fits_is_kept) {
    const picker::Image image = picker::parse_image("2 1 ff000000ff00");
    ASSERT_EQ(picker::fit_image(image, 2, 1), image);
    ASSERT_EQ(picker::fit_image(image, 10, 10), image);
    ASSERT_TRUE(picker::fit_image(image, 0, 10).empty());
}

TEST(image, a_smaller_one_averages_the_pixels) {
    // 4x2: a black and white half, each 2x2
    const picker::Image image = picker::parse_image(
        "4 2 000000000000ffffffffffff000000000000ffffffffffff");
    const picker::Image fitted = picker::fit_image(image, 2, 10);
    ASSERT_EQ(fitted.width, 2);
    ASSERT_EQ(fitted.height, 1);
    ASSERT_EQ(fitted.pixels, Pixels({0x000000, 0xffffff}));
    const picker::Image one = picker::fit_image(image, 1, 1);
    ASSERT_EQ(one.width, 1);
    ASSERT_EQ(one.height, 1);
    ASSERT_EQ(one.pixels, Pixels({0x808080}));
}

TEST(image, a_smaller_one_keeps_its_proportions) {
    picker::Image image;
    image.width = 100;
    image.height = 50;
    image.pixels.assign(100 * 50, 0x123456);
    for (int max_width = 1; max_width <= 120; ++max_width) {
        for (int max_height = 1; max_height <= 60; max_height += 7) {
            const picker::Image fitted = picker::fit_image(image, max_width, max_height);
            ASSERT_LE(fitted.width, max_width);
            ASSERT_LE(fitted.height, max_height);
            ASSERT_EQ(fitted.pixels.size(), static_cast<std::size_t>(fitted.width) * fitted.height);
            ASSERT_TRUE(fitted.width == std::min(100, max_width) or
                        fitted.height == std::min(50, max_height));
            ASSERT_EQ(fitted.pixels[0], 0x123456u);
        }
    }
}

TEST(image, colors_go_to_the_closest_of_256) {
    ASSERT_EQ(picker::nearest_256_color(0x000000), 16);
    ASSERT_EQ(picker::nearest_256_color(0xffffff), 231);
    ASSERT_EQ(picker::nearest_256_color(0xff0000), 196);
    ASSERT_EQ(picker::nearest_256_color(0x00ff00), 46);
    ASSERT_EQ(picker::nearest_256_color(0x0000ff), 21);
    // A grey between the cube's levels: one of the greys
    ASSERT_EQ(picker::nearest_256_color(0x808080), 244);
    ASSERT_EQ(picker::nearest_256_color(0x5f87af), 16 + 36 * 1 + 6 * 2 + 3);
}
