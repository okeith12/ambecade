#ifndef GFX_FROG_FACE_HPP
#define GFX_FROG_FACE_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"
#include "bitmap.hpp"

namespace gfx {
namespace frog {

// Palette for the frog face art below; each named color maps to one art char.
constexpr color_t kBackground = rgb565(24, 36, 48);
constexpr color_t kGreen      = rgb565(80, 200, 90);
constexpr color_t kDark       = rgb565(40, 120, 55);
constexpr color_t kWhite      = rgb565(245, 245, 245);
constexpr color_t kPupil      = rgb565(10, 12, 14);

// Translates one art character into its palette color.
constexpr color_t palette(char c)
{
    switch (c) {
        case 'G': return kGreen;
        case 'D': return kDark;
        case 'W': return kWhite;
        case 'K': return kPupil;
        default:  return kBackground;
    }
}

constexpr std::int16_t kWidth  = 12;
constexpr std::int16_t kHeight = 12;

// The frog face as readable pixel art: one string per row, one char per pixel.
constexpr const char* kArt[kHeight] = {
    "...GGGGGG...",
    "..GGGGGGGG..",
    ".GWWGGGGWWG.",
    ".GWKGGGGWKG.",
    "GGGGGGGGGGGG",
    "GGGGGGGGGGGG",
    "GGGGGGGGGGGG",
    "GGDDDDDDDDGG",
    "GGGGGGGGGGGG",
    ".GGGGGGGGGG.",
    "..GGGGGGGG..",
    "...GGGGGG...",
};

// Holds the expanded pixels so a constexpr builder can fill a plain array.
struct Pixels {
    color_t data[static_cast<std::size_t>(kWidth) * kHeight];
};

// Expands the art into an RGB565 pixel buffer at compile time (no heap, no I/O).
constexpr Pixels make_pixels()
{
    Pixels px{};
    for (std::int16_t y = 0; y < kHeight; ++y) {
        for (std::int16_t x = 0; x < kWidth; ++x) {
            px.data[static_cast<std::size_t>(y) * kWidth + x] = palette(kArt[y][x]);
        }
    }
    return px;
}

constexpr Pixels kPixels = make_pixels();

}  // namespace frog

// The frog face: a self-describing, named asset carrying its own pixels and size.
constexpr Bitmap frog_face{ frog::kPixels.data, frog::kWidth, frog::kHeight };

}  // namespace gfx

#endif  // GFX_FROG_FACE_HPP
