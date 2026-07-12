#include "galaga.hpp"
#include "buttons.hpp"

namespace game {

bool Galaga::overlaps(std::int16_t ax, std::int16_t ay, std::int16_t aw, std::int16_t ah,
                      std::int16_t bx, std::int16_t by, std::int16_t bw, std::int16_t bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void Galaga::spawn_wave()
{
    const std::int16_t margin = 16;
    const std::int16_t usable = static_cast<std::int16_t>(w_ - 2 * margin - kEnemyW);
    const std::int16_t step_x = static_cast<std::int16_t>(kCols > 1 ? usable / (kCols - 1) : 0);
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            Enemy& e = enemies_[row * kCols + col];
            e.x = static_cast<std::int16_t>(margin + col * step_x);
            e.y = static_cast<std::int16_t>(20 + row * (kEnemyH + 8));
            e.alive = true;
        }
    }
}

void Galaga::reset()
{
    player_x_ = static_cast<std::int16_t>((w_ - kPlayerW) / 2);
    player_y_ = static_cast<std::int16_t>(h_ - kPlayerH - 4);
    enemy_dir_ = 1;
    score_ = 0;
    state_ = GalagaState::playing;
    acc_ms_ = 0u;
    for (Bullet& b : bullets_) {
        b.active = false;
    }
    spawn_wave();
}

int Galaga::alive_count() const
{
    int n = 0;
    for (const Enemy& e : enemies_) {
        if (e.alive) {
            ++n;
        }
    }
    return n;
}

void Galaga::fire()
{
    for (Bullet& b : bullets_) {
        if (!b.active) {
            b.x = static_cast<std::int16_t>(player_x_ + kPlayerW / 2 - kBulletW / 2);
            b.y = static_cast<std::int16_t>(player_y_ - kBulletH);
            b.active = true;
            return;   // one bullet per press
        }
    }
}

void Galaga::step(std::uint32_t dt_ms, std::uint32_t input_mask, bool fire_edge)
{
    if (state_ != GalagaState::playing) {
        return;
    }
    if (fire_edge) {
        fire();
    }
    acc_ms_ += dt_ms;
    while (acc_ms_ >= kStepMs) {
        acc_ms_ -= kStepMs;
        tick(input_mask);
        if (state_ != GalagaState::playing) {
            break;
        }
    }
}

void Galaga::tick(std::uint32_t input_mask)
{
    // Player movement, clamped to the field.
    if (input_mask & ui::BUTTON_LEFT) {
        player_x_ = static_cast<std::int16_t>(player_x_ - kPlayerPx);
    }
    if (input_mask & ui::BUTTON_RIGHT) {
        player_x_ = static_cast<std::int16_t>(player_x_ + kPlayerPx);
    }
    if (player_x_ < 0) {
        player_x_ = 0;
    }
    if (player_x_ > w_ - kPlayerW) {
        player_x_ = static_cast<std::int16_t>(w_ - kPlayerW);
    }

    // Bullets rise and expire off the top.
    for (Bullet& b : bullets_) {
        if (!b.active) {
            continue;
        }
        b.y = static_cast<std::int16_t>(b.y - kBulletPx);
        if (b.y + kBulletH < 0) {
            b.active = false;
        }
    }

    // Formation marches; reverse and drop at either edge.
    std::int16_t min_x = w_;
    std::int16_t max_x = 0;
    for (const Enemy& e : enemies_) {
        if (!e.alive) {
            continue;
        }
        if (e.x < min_x) { min_x = e.x; }
        if (e.x > max_x) { max_x = e.x; }
    }
    const bool hit_edge =
        (enemy_dir_ > 0 && max_x + kEnemyW + kEnemyPx > w_) ||
        (enemy_dir_ < 0 && min_x - kEnemyPx < 0);
    for (Enemy& e : enemies_) {
        if (!e.alive) {
            continue;
        }
        if (hit_edge) {
            e.y = static_cast<std::int16_t>(e.y + kEnemyDrop);
        } else {
            e.x = static_cast<std::int16_t>(e.x + enemy_dir_ * kEnemyPx);
        }
    }
    if (hit_edge) {
        enemy_dir_ = static_cast<std::int16_t>(-enemy_dir_);
    }

    // Bullet/enemy collisions.
    for (Bullet& b : bullets_) {
        if (!b.active) {
            continue;
        }
        for (Enemy& e : enemies_) {
            if (e.alive && overlaps(b.x, b.y, kBulletW, kBulletH, e.x, e.y, kEnemyW, kEnemyH)) {
                e.alive = false;
                b.active = false;
                score_ += 10;
                break;
            }
        }
    }

    // Win when the formation is cleared; lose when it reaches the player.
    if (alive_count() == 0) {
        state_ = GalagaState::won;
        return;
    }
    for (const Enemy& e : enemies_) {
        if (e.alive && e.y + kEnemyH >= player_y_) {
            state_ = GalagaState::lost;
            return;
        }
    }
}

}  // namespace game
