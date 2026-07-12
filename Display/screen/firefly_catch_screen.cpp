#include "firefly_catch_screen.hpp"

#include <cstdio>
#include "color.hpp"
#include "font.hpp"

namespace ui {

void FireflyCatchScreen::update(std::uint32_t dt_ms)
{
    if (game_.state() == game::CatchState::playing) {
        game_.step(dt_ms, buttons_.held());
    } else if (buttons_.was_pressed(BUTTON_FIRE)) {
        game_.reset();
    }
}

// Centers a string of the given length and scale on the canvas width.
static std::int16_t centered_x(const gfx::Canvas& canvas, int length, std::int16_t scale)
{
    const std::int16_t width = static_cast<std::int16_t>(length * gfx::text_advance(scale));
    return static_cast<std::int16_t>((canvas.width() - width) / 2);
}

void FireflyCatchScreen::render(gfx::Canvas& canvas)
{
    canvas.clear(gfx::rgb565(8, 10, 28));   // night sky

    // Fireflies glow greenish-yellow.
    const game::Firefly* flies = game_.flies();
    for (int i = 0; i < game::FireflyCatch::kMaxFlies; ++i) {
        if (!flies[i].active) {
            continue;
        }
        canvas.fill_rect(flies[i].x, flies[i].y,
                         game::FireflyCatch::kFlyW, game::FireflyCatch::kFlyH,
                         gfx::rgb565(200, 255, 90));
    }

    // The jar: a glassy body with a lighter rim.
    canvas.fill_rect(game_.player_x(), game_.player_y(),
                     game::FireflyCatch::kJarW, game::FireflyCatch::kJarH,
                     gfx::rgb565(150, 190, 235));
    canvas.fill_rect(game_.player_x(),
                     static_cast<std::int16_t>(game_.player_y() - 3),
                     game::FireflyCatch::kJarW, 3, gfx::color::white);

    // Score and lives.
    char buf[16];
    std::snprintf(buf, sizeof(buf), "SCORE %d", game_.score());
    gfx::draw_text(canvas, buf, 4, 4, 2, gfx::color::white);
    std::snprintf(buf, sizeof(buf), "LIVES %d", game_.lives());
    gfx::draw_text(canvas, buf, 4, 22, 1, gfx::color::yellow);

    if (game_.state() == game::CatchState::over) {
        gfx::draw_text(canvas, "GAME OVER", centered_x(canvas, 9, 3),
                       static_cast<std::int16_t>(canvas.height() / 2 - 20), 3, gfx::color::yellow);
        gfx::draw_text(canvas, "FIRE TO RETRY", centered_x(canvas, 13, 1),
                       static_cast<std::int16_t>(canvas.height() / 2 + 10), 1, gfx::color::white);
    }
}

}  // namespace ui
