#include "firefly_catch.hpp"
#include "buttons.hpp"

namespace game {

bool FireflyCatch::overlaps(std::int16_t ax, std::int16_t ay, std::int16_t aw, std::int16_t ah,
                            std::int16_t bx, std::int16_t by, std::int16_t bw, std::int16_t bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

std::uint32_t FireflyCatch::next_rand()
{
    seed_ = seed_ * 1664525u + 1013904223u;   // numerical-recipes LCG
    return seed_;
}

void FireflyCatch::spawn(bool at_center)
{
    for (Firefly& f : flies_) {
        if (f.active) {
            continue;
        }
        const std::int16_t span = static_cast<std::int16_t>(w_ - kFlyW);
        f.x = at_center
            ? static_cast<std::int16_t>((w_ - kFlyW) / 2)
            : static_cast<std::int16_t>(next_rand() % static_cast<std::uint32_t>(span > 0 ? span : 1));
        f.y = static_cast<std::int16_t>(-kFlyH);
        f.active = true;
        return;
    }
}

void FireflyCatch::reset()
{
    player_x_ = static_cast<std::int16_t>((w_ - kJarW) / 2);
    player_y_ = static_cast<std::int16_t>(h_ - kJarH - 4);
    score_ = 0;
    lives_ = kStartLives;
    state_ = CatchState::playing;
    acc_ms_ = 0u;
    spawn_counter_ = 0;
    for (Firefly& f : flies_) {
        f.active = false;
    }
    spawn(true);   // first firefly is centered: an easy opener
}

void FireflyCatch::step(std::uint32_t dt_ms, std::uint32_t input_mask)
{
    if (state_ != CatchState::playing) {
        return;
    }
    acc_ms_ += dt_ms;
    while (acc_ms_ >= kStepMs) {
        acc_ms_ -= kStepMs;
        tick(input_mask);
        if (state_ != CatchState::playing) {
            break;
        }
    }
}

void FireflyCatch::tick(std::uint32_t input_mask)
{
    // Move the jar, clamped to the field.
    if (input_mask & ui::BUTTON_LEFT) {
        player_x_ = static_cast<std::int16_t>(player_x_ - kPlayerPx);
    }
    if (input_mask & ui::BUTTON_RIGHT) {
        player_x_ = static_cast<std::int16_t>(player_x_ + kPlayerPx);
    }
    if (player_x_ < 0) {
        player_x_ = 0;
    }
    if (player_x_ > w_ - kJarW) {
        player_x_ = static_cast<std::int16_t>(w_ - kJarW);
    }

    // Fireflies fall; catch on jar overlap, miss when they pass the bottom.
    for (Firefly& f : flies_) {
        if (!f.active) {
            continue;
        }
        f.y = static_cast<std::int16_t>(f.y + kFallPx);
        if (overlaps(f.x, f.y, kFlyW, kFlyH, player_x_, player_y_, kJarW, kJarH)) {
            f.active = false;
            score_ += 10;
        } else if (f.y > h_) {
            f.active = false;
            --lives_;
            if (lives_ <= 0) {
                state_ = CatchState::over;
                return;
            }
        }
    }

    // Periodically release a new firefly from a random spot.
    if (++spawn_counter_ >= kSpawnTicks) {
        spawn_counter_ = 0;
        spawn(false);
    }
}

}  // namespace game
