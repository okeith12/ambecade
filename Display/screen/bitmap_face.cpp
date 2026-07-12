#include "bitmap_face.hpp"

namespace ui {

void BitmapFace::update(std::uint32_t /*dt_ms*/)
{
    // Static image; nothing to advance.
}

void BitmapFace::render(gfx::Canvas& canvas)
{
    // Own the whole screen: clear first so the bitmap is not drawn over whatever
    // the previous screen left behind.
    canvas.clear(background_);
    if (bitmap_.pixels == nullptr || bitmap_.width <= 0 || bitmap_.height <= 0) {
        return;
    }

    // Largest integer scale that fits the canvas on both axes (at least 1).
    const std::int16_t sx = static_cast<std::int16_t>(canvas.width() / bitmap_.width);
    const std::int16_t sy = static_cast<std::int16_t>(canvas.height() / bitmap_.height);
    std::int16_t scale = (sx < sy) ? sx : sy;
    if (scale < 1) {
        scale = 1;
    }

    // Center the scaled image on the canvas.
    const std::int16_t draw_w = static_cast<std::int16_t>(bitmap_.width * scale);
    const std::int16_t draw_h = static_cast<std::int16_t>(bitmap_.height * scale);
    const std::int16_t ox = static_cast<std::int16_t>((canvas.width() - draw_w) / 2);
    const std::int16_t oy = static_cast<std::int16_t>((canvas.height() - draw_h) / 2);

    // Draw each source pixel as a scale-by-scale block.
    for (std::int16_t y = 0; y < bitmap_.height; ++y) {
        for (std::int16_t x = 0; x < bitmap_.width; ++x) {
            const gfx::color_t c =
                bitmap_.pixels[static_cast<std::size_t>(y) * bitmap_.width + x];
            canvas.fill_rect(static_cast<std::int16_t>(ox + x * scale),
                             static_cast<std::int16_t>(oy + y * scale),
                             scale, scale, c);
        }
    }
}

}  // namespace ui
