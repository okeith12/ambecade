#include "anniversary_screen.hpp"

#include <cstring>
#include <ctime>
#include "font.hpp"
#include "heart.hpp"
#include "anniversary.hpp"

namespace ui {

void AnniversaryScreen::update(std::uint32_t dt_ms)
{
    phase_ms_ = (phase_ms_ + dt_ms) % kBopMs;
}

std::int16_t AnniversaryScreen::bob_offset() const
{
    const std::int32_t cyc = static_cast<std::int32_t>(kBopMs);
    const std::int32_t p = static_cast<std::int32_t>(phase_ms_ % kBopMs);
    const std::int32_t amp = kBopAmp;
    const std::int32_t q = cyc / 4;   // quarter period
    std::int32_t v;
    if (p < q) {
        v = -(amp * p) / q;                    // 0 -> -amp (rising up)
    } else if (p < 2 * q) {
        v = -amp + (amp * (p - q)) / q;         // -amp -> 0
    } else if (p < 3 * q) {
        v = (amp * (p - 2 * q)) / q;            // 0 -> +amp (dipping down)
    } else {
        v = amp - (amp * (p - 3 * q)) / q;      // +amp -> 0
    }
    return static_cast<std::int16_t>(v);
}

// Centers a string of `length` chars at `scale` on the canvas width.
static std::int16_t centered_x(const gfx::Canvas& canvas, int length, std::int16_t scale)
{
    const std::int16_t w = static_cast<std::int16_t>(length * gfx::text_advance(scale));
    return static_cast<std::int16_t>((canvas.width() - w) / 2);
}

// Draws a bitmap at (ox, oy) scaled by an integer factor, skipping its transparent key.
static void blit_keyed(gfx::Canvas& canvas, const gfx::Bitmap& bmp, std::int16_t ox,
                       std::int16_t oy, std::int16_t scale, gfx::color_t key)
{
    if (bmp.pixels == nullptr) {
        return;
    }
    for (std::int16_t y = 0; y < bmp.height; ++y) {
        for (std::int16_t x = 0; x < bmp.width; ++x) {
            const gfx::color_t c = bmp.pixels[static_cast<std::size_t>(y) * bmp.width + x];
            if (c == key) {
                continue;   // transparent
            }
            canvas.fill_rect(static_cast<std::int16_t>(ox + x * scale),
                             static_cast<std::int16_t>(oy + y * scale), scale, scale, c);
        }
    }
}

void AnniversaryScreen::render(gfx::Canvas& canvas)
{
    canvas.clear(background_);

    const std::int16_t cw = canvas.width();
    const std::int16_t ch = canvas.height();

    // The seahorse: kept small in the upper ~38% of the screen, bobbing.
    if (sprite_.pixels != nullptr && sprite_.width > 0 && sprite_.height > 0) {
        const std::int16_t budget_h = static_cast<std::int16_t>(ch * 38 / 100);
        const std::int16_t sx = static_cast<std::int16_t>(cw / 2 / sprite_.width);
        const std::int16_t sy = static_cast<std::int16_t>(budget_h / sprite_.height);
        std::int16_t scale = (sx < sy) ? sx : sy;
        if (scale < 1) {
            scale = 1;
        }
        const std::int16_t ox = static_cast<std::int16_t>((cw - sprite_.width * scale) / 2);
        const std::int16_t oy = static_cast<std::int16_t>(ch * 3 / 100 + bob_offset());
        blit_keyed(canvas, sprite_, ox, oy, scale, sprite_key_);
    }

    // A pink heart floating just above the date.
    const std::int16_t heart_scale = (cw >= 180) ? 3 : 2;
    const std::int16_t hx = static_cast<std::int16_t>((cw - gfx::heart.width * heart_scale) / 2);
    const std::int16_t hy = static_cast<std::int16_t>(ch * 46 / 100);
    blit_keyed(canvas, gfx::heart, hx, hy, heart_scale, gfx::heart_key);

    // The anniversary date, e.g. "AUG 16".
    const std::int16_t date_scale = 3;
    const std::int16_t date_y = static_cast<std::int16_t>(ch * 56 / 100);
    char date[16];
    anniversary::date_label(ann_month_, ann_day_, date, sizeof(date));
    const int date_len = static_cast<int>(std::strlen(date));
    gfx::draw_text(canvas, date, centered_x(canvas, date_len, date_scale), date_y,
                   date_scale, gfx::rgb565(250, 240, 245));

    // Days until the next anniversary, read from the system clock.
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    const int days = anniversary::days_until(local.tm_year + 1900, local.tm_mon + 1,
                                             local.tm_mday, ann_month_, ann_day_);
    const std::int16_t count_scale = 3;
    const std::int16_t count_y = static_cast<std::int16_t>(ch * 71 / 100);
    char count[16];
    anniversary::countdown_label(days, count, sizeof(count));
    const int count_len = static_cast<int>(std::strlen(count));
    gfx::draw_text(canvas, count, centered_x(canvas, count_len, count_scale), count_y,
                   count_scale, gfx::rgb565(245, 150, 60));

    // The little tagline at the bottom.
    if (tagline_ != nullptr && tagline_[0] != '\0') {
        const std::int16_t tag_scale = 1;
        const std::int16_t tag_y = static_cast<std::int16_t>(ch * 88 / 100);
        const int tag_len = static_cast<int>(std::strlen(tagline_));
        gfx::draw_text(canvas, tagline_, centered_x(canvas, tag_len, tag_scale), tag_y,
                       tag_scale, gfx::rgb565(240, 120, 175));
    }
}

}  // namespace ui
