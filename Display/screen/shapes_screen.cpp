#include "shapes_screen.hpp"
#include "color.hpp"

namespace ui {

// Background palette the screen steps through over time.
static constexpr gfx::color_t kPalette[] = {
    gfx::color::red, gfx::color::yellow, gfx::color::green,
    gfx::color::cyan, gfx::color::blue, gfx::color::magenta
};
static constexpr std::size_t kPaletteCount = sizeof(kPalette) / sizeof(kPalette[0]);

void ShapesScreen::update(std::uint32_t dt_ms)
{
    elapsed_ms_ += dt_ms;
}

void ShapesScreen::render(gfx::Canvas& canvas)
{
    const std::int16_t w = canvas.width();
    const std::int16_t h = canvas.height();

    // Background steps through the palette.
    const std::size_t idx = (elapsed_ms_ / kColorStepMs) % kPaletteCount;
    canvas.clear(kPalette[idx]);

    // A white square sliding left to right across the middle.
    const std::int16_t side = static_cast<std::int16_t>(w / 4);
    const std::int16_t span = static_cast<std::int16_t>(w - side);
    const std::int16_t x = static_cast<std::int16_t>((elapsed_ms_ / 16u) % static_cast<std::uint32_t>(span));
    const std::int16_t y = static_cast<std::int16_t>(h / 2 - side / 2);
    canvas.fill_rect(x, y, side, side, gfx::color::white);

    // A fixed frame around the edge.
    canvas.draw_rect(static_cast<std::int16_t>(w / 8), static_cast<std::int16_t>(h / 8),
                     static_cast<std::int16_t>(w * 3 / 4), static_cast<std::int16_t>(h * 3 / 4),
                     gfx::color::black);
}

}  // namespace ui
