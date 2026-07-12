#include "galaga_screen.hpp"

#include <cstdio>
#include "color.hpp"
#include "font.hpp"

namespace ui {

void GalagaScreen::update(std::uint32_t dt_ms)
{
    if (game_.state() == game::GalagaState::playing) {
        game_.step(dt_ms, buttons_.held(), buttons_.was_pressed(BUTTON_FIRE));
    } else if (buttons_.was_pressed(BUTTON_FIRE)) {
        game_.reset();   // fire restarts after a win or loss
    }
}

// Centers a string of the given length and scale on the canvas width.
static std::int16_t centered_x(const gfx::Canvas& canvas, int length, std::int16_t scale)
{
    const std::int16_t width = static_cast<std::int16_t>(length * gfx::text_advance(scale));
    return static_cast<std::int16_t>((canvas.width() - width) / 2);
}

void GalagaScreen::render(gfx::Canvas& canvas)
{
    canvas.clear(gfx::color::black);

    // Enemies: top row one color, bottom row another.
    const game::Enemy* enemies = game_.enemies();
    for (int i = 0; i < game::Galaga::kMaxEnemies; ++i) {
        if (!enemies[i].alive) {
            continue;
        }
        const gfx::color_t c = (i < game::Galaga::kCols) ? gfx::color::magenta : gfx::color::cyan;
        canvas.fill_rect(enemies[i].x, enemies[i].y,
                         game::Galaga::kEnemyW, game::Galaga::kEnemyH, c);
    }

    // Bullets.
    const game::Bullet* bullets = game_.bullets();
    for (int i = 0; i < game::Galaga::kMaxBullets; ++i) {
        if (!bullets[i].active) {
            continue;
        }
        canvas.fill_rect(bullets[i].x, bullets[i].y,
                         game::Galaga::kBulletW, game::Galaga::kBulletH, gfx::color::yellow);
    }

    // Player ship: body plus a small cannon nub.
    canvas.fill_rect(game_.player_x(), game_.player_y(),
                     game::Galaga::kPlayerW, game::Galaga::kPlayerH, gfx::color::green);
    canvas.fill_rect(static_cast<std::int16_t>(game_.player_x() + game::Galaga::kPlayerW / 2 - 2),
                     static_cast<std::int16_t>(game_.player_y() - 6),
                     4, 6, gfx::color::green);

    // Score.
    char buf[16];
    std::snprintf(buf, sizeof(buf), "SCORE %d", game_.score());
    gfx::draw_text(canvas, buf, 4, 4, 2, gfx::color::white);

    // End-of-game banners.
    if (game_.state() == game::GalagaState::lost) {
        gfx::draw_text(canvas, "GAME OVER", centered_x(canvas, 9, 3),
                       static_cast<std::int16_t>(canvas.height() / 2 - 20), 3, gfx::color::red);
        gfx::draw_text(canvas, "FIRE TO RETRY", centered_x(canvas, 13, 1),
                       static_cast<std::int16_t>(canvas.height() / 2 + 10), 1, gfx::color::white);
    } else if (game_.state() == game::GalagaState::won) {
        gfx::draw_text(canvas, "YOU WIN", centered_x(canvas, 7, 3),
                       static_cast<std::int16_t>(canvas.height() / 2 - 20), 3, gfx::color::green);
        gfx::draw_text(canvas, "FIRE TO RETRY", centered_x(canvas, 13, 1),
                       static_cast<std::int16_t>(canvas.height() / 2 + 10), 1, gfx::color::white);
    }
}

}  // namespace ui
