#ifndef GAME_GALAGA_HPP
#define GAME_GALAGA_HPP

#include <cstdint>

namespace game {

// One player shot; inactive slots are reused.
struct Bullet {
    std::int16_t x;
    std::int16_t y;
    bool active;
};

// One enemy in the formation.
struct Enemy {
    std::int16_t x;
    std::int16_t y;
    bool alive;
};

enum class GalagaState : std::uint8_t { playing, won, lost };

/* A small fixed-array, heap-free Galaga: the player slides along the bottom and
   fires up at a formation that marches side to side and drops toward the player.
   Pure logic advanced in fixed ticks, so it plays identically on hardware and in
   host tests. Input is a ui::Button bitmask plus a one-shot fire edge. */
class Galaga {
public:
    static constexpr int kCols = 6;
    static constexpr int kRows = 2;
    static constexpr int kMaxEnemies = kCols * kRows;
    static constexpr int kMaxBullets = 4;

    static constexpr std::int16_t kPlayerW = 24;
    static constexpr std::int16_t kPlayerH = 12;
    static constexpr std::int16_t kEnemyW = 18;
    static constexpr std::int16_t kEnemyH = 14;
    static constexpr std::int16_t kBulletW = 3;
    static constexpr std::int16_t kBulletH = 8;

    Galaga(std::int16_t field_w, std::int16_t field_h) : w_(field_w), h_(field_h) { reset(); }

    // Restarts a fresh wave.
    void reset();

    // Advances the game by dt_ms; input_mask holds direction bits, fire_edge fires once.
    void step(std::uint32_t dt_ms, std::uint32_t input_mask, bool fire_edge);

    std::int16_t player_x() const { return player_x_; }
    std::int16_t player_y() const { return player_y_; }
    const Bullet* bullets() const { return bullets_; }
    const Enemy* enemies() const { return enemies_; }
    int alive_count() const;
    int score() const { return score_; }
    GalagaState state() const { return state_; }
    std::int16_t field_w() const { return w_; }
    std::int16_t field_h() const { return h_; }

    // Axis-aligned overlap test used for bullet/enemy collisions.
    static bool overlaps(std::int16_t ax, std::int16_t ay, std::int16_t aw, std::int16_t ah,
                         std::int16_t bx, std::int16_t by, std::int16_t bw, std::int16_t bh);

private:
    static constexpr std::uint32_t kStepMs = 16u;
    static constexpr std::int16_t kPlayerPx = 4;
    static constexpr std::int16_t kBulletPx = 6;
    static constexpr std::int16_t kEnemyPx = 2;
    static constexpr std::int16_t kEnemyDrop = 8;

    void spawn_wave();
    void fire();
    void tick(std::uint32_t input_mask);

    std::int16_t w_;
    std::int16_t h_;
    std::int16_t player_x_ = 0;
    std::int16_t player_y_ = 0;
    std::int16_t enemy_dir_ = 1;
    Bullet bullets_[kMaxBullets] = {};
    Enemy enemies_[kMaxEnemies] = {};
    int score_ = 0;
    GalagaState state_ = GalagaState::playing;
    std::uint32_t acc_ms_ = 0u;
};

}  // namespace game

#endif  // GAME_GALAGA_HPP
