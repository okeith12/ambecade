#ifndef GFX_TIARA_HPP
#define GFX_TIARA_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"
#include "bitmap.hpp"

namespace gfx {
namespace crown {

// Palette for the tiara art below; each named color maps to one art char.
constexpr color_t kBg      = rgb565(20, 14, 28);    // deep background
constexpr color_t kOutline = rgb565(22, 20, 28);    // black outline
constexpr color_t kGold    = rgb565(246, 206, 58);  // gold body
constexpr color_t kGem     = rgb565(236, 66, 144);  // pink gems

// Translates one art character into its palette color.
constexpr color_t palette(char c)
{
    switch (c) {
        case 'K': return kOutline;
        case 'G': return kGold;
        case 'P': return kGem;
        default:  return kBg;
    }
}

constexpr std::int16_t kWidth  = 22;
constexpr std::int16_t kHeight = 15;

// A five-point gold crown with a black outline, pink gems on each point, and a
// pink diamond in the center of the band. Quantized from reference pixel art.
constexpr const char* kArt[kHeight] = {
    ".....PP........PP.....",
    ".PP.KKKK..PP..KKKK.PP.",
    "KKKKGKKK.KKKK.KKKGKKKK",
    ".KKGKGKK.KKKK.KKGKGKK.",
    ".KKKKGGK.KGKK.KGGKKKK.",
    ".KKGGGGGKKGGKKGGGGGGK.",
    ".KGGGGGGGGGGGGGGGGGGK.",
    ".KGGGGGGGGGGGGGGGGGGK.",
    ".KGGGGGGGGGGGGGGGGGGK.",
    ".KGGGGGGGGPKGGGGGGGGK.",
    ".KGGGGGGGPPPKGGGGGGGK.",
    ".KGGGGGGGPPPPGGGGGGGK.",
    ".KGGGGGGGGPPGGGGGGGGK.",
    "..KGGGGGGGGGGGGGGGGK..",
    "...KKKKKKKKKKKKKKKK...",
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
// A short art row is padded with background rather than read out of bounds.
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

}  // namespace crown

// The tiara: a self-describing, named asset carrying its own pixels and size.
constexpr Bitmap tiara{ crown::kPixels.data, crown::kWidth, crown::kHeight };

}  // namespace gfx

#endif  // GFX_TIARA_HPP
