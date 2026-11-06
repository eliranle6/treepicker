#include "picker/image.h"

#include <algorithm>
#include <cmath>

namespace picker {

namespace {

int hex_digit(char c) {
    if (c >= '0' and c <= '9') {
        return c - '0';
    }
    if (c >= 'a' and c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' and c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

// The number at s's start, and where it ends (after a space), or -1 if there's none
int parse_dimension(const std::string& s, std::size_t& start) {
    const std::size_t space = s.find(' ', start);
    if (space == std::string::npos or space == start or space - start > 5) {
        return -1;
    }
    int value = 0;
    for (std::size_t i = start; i < space; ++i) {
        if (s[i] < '0' or s[i] > '9') {
            return -1;
        }
        value = value * 10 + (s[i] - '0');
    }
    start = space + 1;
    return value;
}

int red(std::uint32_t rgb) { return (rgb >> 16) & 0xff; }
int green(std::uint32_t rgb) { return (rgb >> 8) & 0xff; }
int blue(std::uint32_t rgb) { return rgb & 0xff; }

int squared_distance(int r1, int g1, int b1, int r2, int g2, int b2) {
    return (r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2);
}

// The levels of each channel in the palette's cube (xterm's)
const int CUBE_LEVELS[] = {0, 95, 135, 175, 215, 255};

int nearest_cube_index(int value) {
    int nearest = 0;
    for (int i = 1; i < 6; ++i) {
        if (std::abs(CUBE_LEVELS[i] - value) < std::abs(CUBE_LEVELS[nearest] - value)) {
            nearest = i;
        }
    }
    return nearest;
}

} // namespace

Image parse_image(const std::string& s) {
    std::size_t start = 0;
    const int width = parse_dimension(s, start);
    const int height = width > 0 ? parse_dimension(s, start) : -1;
    if (width <= 0 or height <= 0 or
            s.size() - start != static_cast<std::size_t>(width) * height * 6) {
        return Image();
    }
    Image image;
    image.width = width;
    image.height = height;
    image.pixels.reserve(static_cast<std::size_t>(width) * height);
    for (std::size_t i = start; i < s.size(); i += 6) {
        std::uint32_t rgb = 0;
        for (std::size_t j = i; j < i + 6; ++j) {
            const int digit = hex_digit(s[j]);
            if (digit < 0) {
                return Image();
            }
            rgb = rgb << 4 | digit;
        }
        image.pixels.push_back(rgb);
    }
    return image;
}

Image fit_image(const Image& image, int max_width, int max_height) {
    if (image.empty() or max_width <= 0 or max_height <= 0) {
        return Image();
    }
    if (image.width <= max_width and image.height <= max_height) {
        return image;
    }
    const double scale = std::min(static_cast<double>(max_width) / image.width,
                                  static_cast<double>(max_height) / image.height);
    Image fitted;
    fitted.width = std::clamp(static_cast<int>(std::lround(image.width * scale)), 1, max_width);
    fitted.height = std::clamp(static_cast<int>(std::lround(image.height * scale)), 1, max_height);
    fitted.pixels.reserve(static_cast<std::size_t>(fitted.width) * fitted.height);
    for (int y = 0; y < fitted.height; ++y) {
        // The rows (and below, columns) of image that this pixel stands for
        const int y0 = y * image.height / fitted.height;
        const int y1 = std::max(y0 + 1, (y + 1) * image.height / fitted.height);
        for (int x = 0; x < fitted.width; ++x) {
            const int x0 = x * image.width / fitted.width;
            const int x1 = std::max(x0 + 1, (x + 1) * image.width / fitted.width);
            long r = 0, g = 0, b = 0;
            for (int source_y = y0; source_y < y1; ++source_y) {
                for (int source_x = x0; source_x < x1; ++source_x) {
                    const std::uint32_t rgb = image.at(source_x, source_y);
                    r += red(rgb);
                    g += green(rgb);
                    b += blue(rgb);
                }
            }
            const long n = static_cast<long>(y1 - y0) * (x1 - x0);
            fitted.pixels.push_back(static_cast<std::uint32_t>(
                (r + n / 2) / n << 16 | (g + n / 2) / n << 8 | (b + n / 2) / n));
        }
    }
    return fitted;
}

short nearest_256_color(std::uint32_t rgb) {
    const int r = red(rgb), g = green(rgb), b = blue(rgb);
    const int ri = nearest_cube_index(r), gi = nearest_cube_index(g), bi = nearest_cube_index(b);
    const int cube_distance = squared_distance(r, g, b, CUBE_LEVELS[ri], CUBE_LEVELS[gi],
                                               CUBE_LEVELS[bi]);
    // The greys: 232-255, 8 to 238 by 10
    const int average = (r + g + b) / 3;
    const int grey_index = std::clamp((average - 8 + 5) / 10, 0, 23);
    const int grey = 8 + grey_index * 10;
    const int grey_distance = squared_distance(r, g, b, grey, grey, grey);
    if (grey_distance < cube_distance) {
        return static_cast<short>(232 + grey_index);
    }
    return static_cast<short>(16 + 36 * ri + 6 * gi + bi);
}

} // namespace picker
