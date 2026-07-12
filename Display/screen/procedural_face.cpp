#include "procedural_face.hpp"
#include "color.hpp"

namespace ui {

void ProceduralFace::update(std::uint32_t dt_ms)
{
    phase_ms_ += dt_ms;
    while (phase_ms_ >= kCycleMs) {
        phase_ms_ -= kCycleMs;
    }
}

void ProceduralFace::render(gfx::Canvas& canvas)
{
    const std::int16_t w = canvas.width();
    const std::int16_t h = canvas.height();

    canvas.clear(gfx::color::yellow);

    // Eyes: two squares in the upper third, positioned relative to canvas size.
    const std::int16_t eye_w = static_cast<std::int16_t>(w / 6);
    const std::int16_t eye_h = static_cast<std::int16_t>(h / 6);
    const std::int16_t eye_y = static_cast<std::int16_t>(h / 3);
    const std::int16_t left_x = static_cast<std::int16_t>(w / 4 - eye_w / 2);
    const std::int16_t right_x = static_cast<std::int16_t>(3 * w / 4 - eye_w / 2);

    if (blinking()) {
        // Closed: a thin line where the eye's middle would be.
        const std::int16_t line_y = static_cast<std::int16_t>(eye_y + eye_h / 2);
        canvas.draw_hline(left_x, line_y, eye_w, gfx::color::black);
        canvas.draw_hline(right_x, line_y, eye_w, gfx::color::black);
    } else {
        // Open: filled squares.
        canvas.fill_rect(left_x, eye_y, eye_w, eye_h, gfx::color::black);
        canvas.fill_rect(right_x, eye_y, eye_w, eye_h, gfx::color::black);
    }

    // Mouth: a bar in the lower third.
    const std::int16_t mouth_w = static_cast<std::int16_t>(w / 2);
    const std::int16_t mouth_h = static_cast<std::int16_t>(h / 12);
    const std::int16_t mouth_x = static_cast<std::int16_t>(w / 2 - mouth_w / 2);
    const std::int16_t mouth_y = static_cast<std::int16_t>(2 * h / 3);
    canvas.fill_rect(mouth_x, mouth_y, mouth_w, mouth_h, gfx::color::black);
}

}  // namespace ui
