#ifndef PICKER_IMAGE_H__
#define PICKER_IMAGE_H__

#include <cstdint>
#include <string>
#include <vector>

namespace picker {

// A picture to show in the preview column (e.g. the selected file's, if it's a photo): its pixels,
// row by row, each 0xRRGGBB. Drawn with half blocks ("▀"), two pixels a character: the upper one
// its foreground color, the lower its background.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;

    bool empty(void) const { return pixels.empty(); }

    std::uint32_t at(int x, int y) const { return pixels[static_cast<std::size_t>(y) * width + x]; }

    bool operator==(const Image& other) const {
        return width == other.width and height == other.height and pixels == other.pixels;
    }
};

// The image in s: its width, a space, its height, a space, and its pixels in hex, 6 digits each
// (RRGGBB), row by row. Returns an empty image for "" (no image) or one that isn't that.
Image parse_image(const std::string& s);

// image made smaller to fit in max_width x max_height pixels, if it doesn't, keeping its
// proportions: each pixel the average of the ones it stands for. Never made bigger.
Image fit_image(const Image& image, int max_width, int max_height);

// The color of the 256-color palette (its 6x6x6 cube or its greys: 16-255, not the first 16, which
// terminals change) closest to rgb (0xRRGGBB)
short nearest_256_color(std::uint32_t rgb);

} // namespace picker

#endif
