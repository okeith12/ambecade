#ifndef GFX_COLOR_HPP
#define GFX_COLOR_HPP

#include <cstdint>

namespace gfx {

// One pixel in RGB565 (5 red, 6 green, 5 blue), matching the panel's COLMOD.
using color_t = std::uint16_t;

// Packs 8-bit r/g/b into RGB565 at compile time (top bits kept, low bits dropped).
constexpr color_t rgb565(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    return static_cast<color_t>(((static_cast<std::uint16_t>(r) & 0xF8u) << 8) |
                                ((static_cast<std::uint16_t>(g) & 0xFCu) << 3) |
                                (static_cast<std::uint16_t>(b) >> 3));
}

// Common named colors, resolved at compile time via rgb565().
namespace color {
    constexpr color_t black   = 0x0000u;
    constexpr color_t white   = 0xFFFFu;
    constexpr color_t red     = rgb565(255u, 0u, 0u);
    constexpr color_t green   = rgb565(0u, 255u, 0u);
    constexpr color_t blue    = rgb565(0u, 0u, 255u);
    constexpr color_t yellow  = rgb565(255u, 255u, 0u);
    constexpr color_t cyan    = rgb565(0u, 255u, 255u);
    constexpr color_t magenta = rgb565(255u, 0u, 255u);
}

}  // namespace gfx

#endif  // GFX_COLOR_HPP
