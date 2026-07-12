#include "font.hpp"

namespace gfx {

// Digits 0-9 as readable 5x7 art.
static const Glyph kDigits[10] = {
    {{ " ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### " }},  // 0
    {{ "  #  ", " ##  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### " }},  // 1
    {{ " ### ", "#   #", "    #", "   # ", "  #  ", " #   ", "#####" }},  // 2
    {{ "#####", "    #", "   # ", "  ## ", "    #", "#   #", " ### " }},  // 3
    {{ "   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # " }},  // 4
    {{ "#####", "#    ", "#### ", "    #", "    #", "#   #", " ### " }},  // 5
    {{ " ### ", "#    ", "#    ", "#### ", "#   #", "#   #", " ### " }},  // 6
    {{ "#####", "    #", "   # ", "  #  ", " #   ", " #   ", " #   " }},  // 7
    {{ " ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### " }},  // 8
    {{ " ### ", "#   #", "#   #", " ####", "    #", "    #", " ### " }},  // 9
};

// Uppercase A-Z as readable 5x7 art.
static const Glyph kUpper[26] = {
    {{ " ### ", "#   #", "#   #", "#####", "#   #", "#   #", "#   #" }},  // A
    {{ "#### ", "#   #", "#   #", "#### ", "#   #", "#   #", "#### " }},  // B
    {{ " ### ", "#   #", "#    ", "#    ", "#    ", "#   #", " ### " }},  // C
    {{ "#### ", "#   #", "#   #", "#   #", "#   #", "#   #", "#### " }},  // D
    {{ "#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#####" }},  // E
    {{ "#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#    " }},  // F
    {{ " ### ", "#   #", "#    ", "# ###", "#   #", "#   #", " ### " }},  // G
    {{ "#   #", "#   #", "#   #", "#####", "#   #", "#   #", "#   #" }},  // H
    {{ " ### ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### " }},  // I
    {{ "  ###", "   # ", "   # ", "   # ", "   # ", "#  # ", " ##  " }},  // J
    {{ "#   #", "#  # ", "# #  ", "##   ", "# #  ", "#  # ", "#   #" }},  // K
    {{ "#    ", "#    ", "#    ", "#    ", "#    ", "#    ", "#####" }},  // L
    {{ "#   #", "## ##", "# # #", "# # #", "#   #", "#   #", "#   #" }},  // M
    {{ "#   #", "##  #", "# # #", "# # #", "#  ##", "#   #", "#   #" }},  // N
    {{ " ### ", "#   #", "#   #", "#   #", "#   #", "#   #", " ### " }},  // O
    {{ "#### ", "#   #", "#   #", "#### ", "#    ", "#    ", "#    " }},  // P
    {{ " ### ", "#   #", "#   #", "#   #", "# # #", "#  # ", " ## #" }},  // Q
    {{ "#### ", "#   #", "#   #", "#### ", "# #  ", "#  # ", "#   #" }},  // R
    {{ " ####", "#    ", "#    ", " ### ", "    #", "    #", "#### " }},  // S
    {{ "#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  " }},  // T
    {{ "#   #", "#   #", "#   #", "#   #", "#   #", "#   #", " ### " }},  // U
    {{ "#   #", "#   #", "#   #", "#   #", "#   #", " # # ", "  #  " }},  // V
    {{ "#   #", "#   #", "#   #", "# # #", "# # #", "## ##", "#   #" }},  // W
    {{ "#   #", "#   #", " # # ", "  #  ", " # # ", "#   #", "#   #" }},  // X
    {{ "#   #", "#   #", " # # ", "  #  ", "  #  ", "  #  ", "  #  " }},  // Y
    {{ "#####", "    #", "   # ", "  #  ", " #   ", "#    ", "#####" }},  // Z
};

static const Glyph kSpace  = {{ "     ", "     ", "     ", "     ", "     ", "     ", "     " }};
static const Glyph kColon  = {{ "     ", "  #  ", "  #  ", "     ", "  #  ", "  #  ", "     " }};
static const Glyph kSlash  = {{ "    #", "    #", "   # ", "  #  ", " #   ", "#    ", "#    " }};
static const Glyph kDash   = {{ "     ", "     ", "     ", "#####", "     ", "     ", "     " }};
static const Glyph kPeriod = {{ "     ", "     ", "     ", "     ", "     ", "  ## ", "  ## " }};

const Glyph* glyph_for(char c)
{
    if (c >= '0' && c <= '9') {
        return &kDigits[c - '0'];
    }
    if (c >= 'A' && c <= 'Z') {
        return &kUpper[c - 'A'];
    }
    if (c >= 'a' && c <= 'z') {
        return &kUpper[c - 'a'];   // fold lowercase onto uppercase
    }
    switch (c) {
        case ' ': return &kSpace;
        case ':': return &kColon;
        case '/': return &kSlash;
        case '-': return &kDash;
        case '.': return &kPeriod;
        default:  return nullptr;
    }
}

void draw_char(Canvas& canvas, char c, std::int16_t x, std::int16_t y,
               std::int16_t scale, color_t color)
{
    const Glyph* g = glyph_for(c);
    if (g == nullptr || scale < 1) {
        return;
    }
    for (std::int16_t row = 0; row < kGlyphH; ++row) {
        const char* line = g->rows[row];
        for (std::int16_t col = 0; col < kGlyphW; ++col) {
            if (line[col] == '#') {
                canvas.fill_rect(static_cast<std::int16_t>(x + col * scale),
                                 static_cast<std::int16_t>(y + row * scale),
                                 scale, scale, color);
            }
        }
    }
}

std::int16_t draw_text(Canvas& canvas, const char* text, std::int16_t x,
                       std::int16_t y, std::int16_t scale, color_t color)
{
    if (text == nullptr) {
        return x;
    }
    std::int16_t cursor = x;
    for (const char* p = text; *p != '\0'; ++p) {
        draw_char(canvas, *p, cursor, y, scale, color);
        cursor = static_cast<std::int16_t>(cursor + text_advance(scale));
    }
    return cursor;
}

}  // namespace gfx
