#ifndef GAME_FIREFLY_CATCH_HPP
#define GAME_FIREFLY_CATCH_HPP

#include <cstdint>

namespace game {

// One falling firefly; inactive slots are reused.
struct Firefly {
    std::int16_t x;
    std::int16_t y;
    bool active;
};

enum class CatchState : std::uint8_t { playing, over };

/* Firefly Catch: slide the jar along the bottom to catch fireflies that fall
   from random spots up top; miss one and you lose a life. Heap-free, advanced in
   fixed ticks, with a seeded RNG so it plays identically on hardware and in
   host tests. */
class FireflyCatch {
public:
    static constexpr int kMaxFlies = 6;
    static constexpr int kStartLives = 3;
    static constexpr std::int16_t kJarW = 36;
    static constexpr std::int16_t kJarH = 18;
    static constexpr std::int16_t kFlyW = 8;
    static constexpr std::int16_t kFlyH = 8;

    FireflyCatch(std::int16_t field_w, std::int16_t field_h, std::uint32_t seed)
        : w_(field_w), h_(field_h), seed_(seed) { reset(); }

    // Restarts a fresh round (keeps the constructed seed's sequence going).
    void reset();

    // Advances play by dt_ms; input_mask holds the left/right direction bits.
    void step(std::uint32_t dt_ms, std::uint32_t input_mask);

    std::int16_t player_x() const { return player_x_; }
    std::int16_t player_y() const { return player_y_; }
    const Firefly* flies() const { return flies_; }
    int score() const { return score_; }
    int lives() const { return lives_; }
    CatchState state() const { return state_; }
    std::int16_t field_w() const { return w_; }
    std::int16_t field_h() const { return h_; }

    // Axis-aligned overlap test used for jar/firefly catches.
    static bool overlaps(std::int16_t ax, std::int16_t ay, std::int16_t aw, std::int16_t ah,
                         std::int16_t bx, std::int16_t by, std::int16_t bw, std::int16_t bh);

private:
    static constexpr std::uint32_t kStepMs = 16u;
    static constexpr std::int16_t kPlayerPx = 5;
    static constexpr std::int16_t kFallPx = 3;
    static constexpr int kSpawnTicks = 45;   // roughly 0.7s between fireflies

    std::uint32_t next_rand();
    void spawn(bool at_center);
    void tick(std::uint32_t input_mask);

    std::int16_t w_;
    std::int16_t h_;
    std::int16_t player_x_ = 0;
    std::int16_t player_y_ = 0;
    std::uint32_t seed_;
    Firefly flies_[kMaxFlies] = {};
    int score_ = 0;
    int lives_ = kStartLives;
    CatchState state_ = CatchState::playing;
    std::uint32_t acc_ms_ = 0u;
    int spawn_counter_ = 0;
};

}  // namespace game

#endif  // GAME_FIREFLY_CATCH_HPP
