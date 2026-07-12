#include "bitmap_face.hpp"

namespace ui {

void BitmapFace::update(std::uint32_t /*dt_ms*/)
{
    // Static image; nothing to advance.
}

void BitmapFace::render(gfx::Canvas& canvas)
{
    if (bitmap_.pixels == nullptr) {
        return;
    }
    // Center on the canvas; blit clips if the bitmap is larger than the surface.
    const std::int16_t x =
        static_cast<std::int16_t>((canvas.width() - bitmap_.width) / 2);
    const std::int16_t y =
        static_cast<std::int16_t>((canvas.height() - bitmap_.height) / 2);
    canvas.blit(bitmap_.pixels, x, y, bitmap_.width, bitmap_.height);
}

}  // namespace ui
