#include "clock_screen.hpp"

#include <cstring>
#include <ctime>
#include "color.hpp"
#include "font.hpp"
#include "clock_format.hpp"

namespace ui {

void ClockScreen::update(std::uint32_t /*dt_ms*/)
{
    // The time is read straight from the system clock in render(); nothing to advance.
}

// Centers a string of the given length and scale on the canvas width.
static std::int16_t centered_x(const gfx::Canvas& canvas, int length, std::int16_t scale)
{
    const std::int16_t width = static_cast<std::int16_t>(length * gfx::text_advance(scale));
    return static_cast<std::int16_t>((canvas.width() - width) / 2);
}

void ClockScreen::render(gfx::Canvas& canvas)
{
    canvas.clear(gfx::rgb565(6, 8, 20));

    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);

    const std::int16_t time_scale = 6;
    const std::int16_t y_time = static_cast<std::int16_t>(canvas.height() / 2 - gfx::kGlyphH * time_scale / 2);

    if (local.tm_year + 1900 >= 2024) {
        char hm[8];
        clock_format::hhmm(local.tm_hour, local.tm_min, hm, sizeof(hm));
        gfx::draw_text(canvas, hm, centered_x(canvas, 5, time_scale), y_time, time_scale, gfx::color::white);

        char date[24];
        clock_format::date_str(local.tm_wday, local.tm_mon, local.tm_mday, date, sizeof(date));
        const int date_len = static_cast<int>(std::strlen(date));
        gfx::draw_text(canvas, date, centered_x(canvas, date_len, 2),
                       static_cast<std::int16_t>(y_time + gfx::kGlyphH * time_scale + 12), 2, gfx::color::cyan);
    } else {
        gfx::draw_text(canvas, "--:--", centered_x(canvas, 5, time_scale), y_time, time_scale,
                       gfx::rgb565(120, 120, 140));
        gfx::draw_text(canvas, "SYNCING", centered_x(canvas, 7, 2),
                       static_cast<std::int16_t>(y_time + gfx::kGlyphH * time_scale + 12), 2, gfx::color::cyan);
    }
}

}  // namespace ui
