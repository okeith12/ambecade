#ifndef GFX_SEAHORSE_HPP
#define GFX_SEAHORSE_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"
#include "bitmap.hpp"

namespace gfx {
namespace sea {

// Palette for the seahorse art below; each named color maps to one art char.
// kBg doubles as the transparent key so the sprite floats on its background.
constexpr color_t kBg      = rgb565(26, 14, 26);    // background / transparent
constexpr color_t kOutline = rgb565(20, 18, 24);    // black outline + eye
constexpr color_t kBody    = rgb565(246, 214, 120); // yellow body
constexpr color_t kBelly   = rgb565(240, 165, 92);  // orange belly / snout
constexpr color_t kFin     = rgb565(176, 222, 236); // light blue fins

// Translates one art character into its palette color.
constexpr color_t palette(char c)
{
    switch (c) {
        case 'K': return kOutline;
        case 'Y': return kBody;
        case 'O': return kBelly;
        case 'B': return kFin;
        default:  return kBg;
    }
}

constexpr std::int16_t kWidth  = 18;
constexpr std::int16_t kHeight = 28;

// A cute seahorse facing left: yellow body, orange belly, blue fins, black
// outline, curled tail. Quantized from reference pixel art. '.' is transparent.
constexpr const char* kArt[kHeight] = {
    ".........OB.......",
    "........OKKBB.....",
    "......BKKKKBBB....",
    ".....KKKKKKBBK....",
    "....KOYYYYOKKKB...",
    "...KOYYYYYYOKKK...",
    "...KOYYYYYYOKKK...",
    "KKKOYYYYYYYYOKB...",
    "KKOYYYYYYYYYOKB...",
    "KKYKOYYYYYYYOKK...",
    "OKKOKKKYYYYYOKKKK.",
    ".OO.BOKKYYYOKKKKKB",
    ".....BOOYYOOKOBBKB",
    "....BOOYYYOKBBBBB.",
    "...BKOYYYYOKBBBBB.",
    "...KOYYYYYOKBBBBB.",
    "...KOYYYYYYOKBBKKB",
    "...KOYYYYYYOKKKKKB",
    "....KOOYYYYOK.....",
    "....BKOOYYYOK.....",
    "...KKOOOKYYOK.....",
    "..KOKKOOKYYOK.....",
    ".KOYKKYOKYYOK.....",
    ".KOOKKOOOYOKB.....",
    ".OOOOOOOOOKB......",
    "..OOOOOOOOO.......",
    "...OOOOOOO........",
    "....OKKKO.........",
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

}  // namespace sea

// The seahorse: a self-describing, named asset carrying its own pixels and size.
constexpr Bitmap seahorse{ sea::kPixels.data, sea::kWidth, sea::kHeight };

// The color the seahorse treats as transparent when drawn over a background.
constexpr color_t seahorse_key = sea::kBg;

}  // namespace gfx

#endif  // GFX_SEAHORSE_HPP
