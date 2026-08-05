#ifndef GFX_HEART_HPP
#define GFX_HEART_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"
#include "bitmap.hpp"

namespace gfx {
namespace hrt {

// Palette for the heart; kBg is the transparent key.
constexpr color_t kBg   = rgb565(26, 14, 26);     // transparent
constexpr color_t kPink = rgb565(240, 110, 165);  // heart fill

// Translates one art character into its palette color.
constexpr color_t palette(char c)
{
    return (c == 'P') ? kPink : kBg;
}

constexpr std::int16_t kWidth  = 7;
constexpr std::int16_t kHeight = 7;

// A small pink heart. '.' is transparent.
constexpr const char* kArt[kHeight] = {
    ".PP.PP.",
    "PPPPPPP",
    "PPPPPPP",
    "PPPPPPP",
    ".PPPPP.",
    "..PPP..",
    "...P...",
};

// Length of a NUL-terminated art row (constexpr so make_pixels stays compile time).
constexpr std::size_t row_len(const char* s)
{
    std::size_t n = 0u;
    while (s[n] != '\0') {
        ++n;
    }
    return n;
}

// Holds the expanded pixels so a constexpr builder can fill a plain array.
struct Pixels {
    color_t data[static_cast<std::size_t>(kWidth) * kHeight];
};

// Expands the art into an RGB565 pixel buffer at compile time (no heap, no I/O).
constexpr Pixels make_pixels()
{
    Pixels px{};
    for (std::int16_t y = 0; y < kHeight; ++y) {
        const char* row = kArt[y];
        const std::size_t len = row_len(row);
        for (std::int16_t x = 0; x < kWidth; ++x) {
            const char c = (static_cast<std::size_t>(x) < len) ? row[x] : '.';
            px.data[static_cast<std::size_t>(y) * kWidth + x] = palette(c);
        }
    }
    return px;
}

constexpr Pixels kPixels = make_pixels();

}  // namespace hrt

// The heart: a self-describing, named asset carrying its own pixels and size.
constexpr Bitmap heart{ hrt::kPixels.data, hrt::kWidth, hrt::kHeight };

// The color the heart treats as transparent when drawn over a background.
constexpr color_t heart_key = hrt::kBg;

}  // namespace gfx

#endif  // GFX_HEART_HPP
