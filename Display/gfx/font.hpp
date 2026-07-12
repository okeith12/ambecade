#ifndef GFX_FONT_HPP
#define GFX_FONT_HPP

#include <cstdint>
#include "canvas.hpp"
#include "color.hpp"

namespace gfx {

// Glyph cell size for the built-in 5x7 font.
constexpr std::int16_t kGlyphW = 5;
constexpr std::int16_t kGlyphH = 7;

// One 5x7 glyph as seven rows of five characters ('#' = lit, anything else off).
struct Glyph {
    const char* rows[kGlyphH];
};

// Returns the glyph for a character (digits, A-Z, and a few symbols), or nullptr.
const Glyph* glyph_for(char c);

// Draws one glyph at (x, y), each font pixel rendered as a scale-by-scale block.
void draw_char(Canvas& canvas, char c, std::int16_t x, std::int16_t y,
               std::int16_t scale, color_t color);

// Draws a string left to right and returns the x just past the last glyph.
std::int16_t draw_text(Canvas& canvas, const char* text, std::int16_t x,
                       std::int16_t y, std::int16_t scale, color_t color);

// Horizontal advance per character at the given scale (glyph width plus a gap).
constexpr std::int16_t text_advance(std::int16_t scale)
{
    return static_cast<std::int16_t>((kGlyphW + 1) * scale);
}

}  // namespace gfx

#endif  // GFX_FONT_HPP
