#include <unity.h>
#include "canvas.hpp"
#include "color.hpp"
#include "framebuffer_canvas.hpp"
#include "screen.hpp"
#include "screen_manager.hpp"
#include "procedural_face.hpp"
#include "shapes_screen.hpp"
#include "buttons.hpp"
#include "galaga.hpp"
#include "firefly_catch.hpp"
#include "clock_format.hpp"

using namespace ui;

void setUp(void) {}
void tearDown(void) {}

namespace {

// Test-only id set (the manager is generic over any contiguous scoped enum).
enum class test_id : std::size_t { a = 0, b = 1, count = 2 };

// Records how often it is driven, so tests can prove who the manager delegates to.
class StubScreen : public Screen {
public:
    void update(std::uint32_t dt_ms) override { ++updates; last_dt = dt_ms; }
    void render(gfx::Canvas&) override { ++renders; }
    int updates = 0;
    int renders = 0;
    std::uint32_t last_dt = 0u;
};

}  // namespace

static void test_manager_no_active_is_error(void)
{
    ScreenManager<test_id> mgr;
    gfx::FramebufferCanvas<8, 8> fb;
    TEST_ASSERT_EQUAL_INT((int)screen_status::err_no_active, (int)mgr.update(16u));
    TEST_ASSERT_EQUAL_INT((int)screen_status::err_no_active, (int)mgr.render(fb));
    TEST_ASSERT_FALSE(mgr.has_active());
}

static void test_manager_rejects_out_of_range(void)
{
    ScreenManager<test_id> mgr;
    StubScreen a;
    mgr.set_screen(test_id::a, &a);
    TEST_ASSERT_EQUAL_INT((int)screen_status::err_range,
        (int)mgr.set_active(static_cast<test_id>(5)));
    TEST_ASSERT_FALSE(mgr.has_active());
}

static void test_manager_rejects_empty_slot(void)
{
    ScreenManager<test_id> mgr;
    TEST_ASSERT_EQUAL_INT((int)screen_status::err_empty,
        (int)mgr.set_active(test_id::a));
}

static void test_manager_rejects_null_screen(void)
{
    ScreenManager<test_id> mgr;
    TEST_ASSERT_EQUAL_INT((int)screen_status::err_null,
        (int)mgr.set_screen(test_id::a, nullptr));
}

static void test_manager_delegates_to_active_only(void)
{
    ScreenManager<test_id> mgr;
    StubScreen a, b;
    mgr.set_screen(test_id::a, &a);
    mgr.set_screen(test_id::b, &b);
    TEST_ASSERT_EQUAL_INT((int)screen_status::ok, (int)mgr.set_active(test_id::a));

    gfx::FramebufferCanvas<8, 8> fb;
    mgr.update(10u);
    mgr.render(fb);

    TEST_ASSERT_EQUAL_INT(1, a.updates);
    TEST_ASSERT_EQUAL_INT(1, a.renders);
    TEST_ASSERT_EQUAL_UINT32(10u, a.last_dt);
    TEST_ASSERT_EQUAL_INT(0, b.updates);
    TEST_ASSERT_EQUAL_INT(0, b.renders);
}

static void test_manager_switches_active(void)
{
    ScreenManager<test_id> mgr;
    StubScreen a, b;
    mgr.set_screen(test_id::a, &a);
    mgr.set_screen(test_id::b, &b);

    mgr.set_active(test_id::a);
    mgr.update(5u);
    mgr.set_active(test_id::b);
    mgr.update(7u);

    TEST_ASSERT_EQUAL_INT(1, a.updates);   // unchanged after switch
    TEST_ASSERT_EQUAL_INT(1, b.updates);
    TEST_ASSERT_EQUAL_UINT32(7u, b.last_dt);
}

static void test_face_renders_eyes_open(void)
{
    ProceduralFace face;
    gfx::FramebufferCanvas<48, 48> fb;
    face.render(fb);

    TEST_ASSERT_FALSE(face.blinking());
    TEST_ASSERT_EQUAL_HEX16(gfx::color::yellow, fb.pixel_at(0, 0));   // background
    TEST_ASSERT_EQUAL_HEX16(gfx::color::black, fb.pixel_at(12, 18));  // left eye fill
    TEST_ASSERT_EQUAL_HEX16(gfx::color::black, fb.pixel_at(36, 18));  // right eye fill
}

static void test_face_blinks_briefly_then_reopens(void)
{
    ProceduralFace face;
    gfx::FramebufferCanvas<48, 48> fb;

    // Eyes stay open through almost the whole cycle.
    TEST_ASSERT_FALSE(face.blinking());
    face.update(2999u);
    TEST_ASSERT_FALSE(face.blinking());

    // They shut only for the brief closure at the end of the cycle.
    face.update(1u);
    TEST_ASSERT_TRUE(face.blinking());
    face.render(fb);
    TEST_ASSERT_EQUAL_HEX16(gfx::color::yellow, fb.pixel_at(12, 18));  // closed: background

    // After the short closure the eyes open again (not a 50/50 toggle).
    face.update(150u);
    TEST_ASSERT_FALSE(face.blinking());
    face.render(fb);
    TEST_ASSERT_EQUAL_HEX16(gfx::color::black, fb.pixel_at(12, 18));   // open: filled
}

static void test_shapes_screen_cycles_background(void)
{
    ShapesScreen s;
    gfx::FramebufferCanvas<16, 16> fb;

    s.render(fb);
    const gfx::color_t first = fb.pixel_at(0, 0);   // corner is background

    s.update(600u);                                 // advance past one color step
    s.render(fb);
    const gfx::color_t second = fb.pixel_at(0, 0);

    TEST_ASSERT_TRUE(first != second);
}

static void test_buttons_debounce_press_release(void)
{
    Buttons b;

    // A single glitchy sample never settles.
    b.poll(5u, BUTTON_FIRE);
    b.poll(5u, BUTTON_NONE);
    TEST_ASSERT_EQUAL_UINT32(0u, b.held());

    // Held steadily past the debounce window: settles and fires one edge.
    b.poll(10u, BUTTON_LEFT);   // new level seen, settle timer starts
    b.poll(10u, BUTTON_LEFT);   // +10ms
    b.poll(10u, BUTTON_LEFT);   // +10ms = 20ms window reached
    TEST_ASSERT_TRUE(b.is_held(BUTTON_LEFT));
    TEST_ASSERT_TRUE(b.was_pressed(BUTTON_LEFT));

    // No repeated edge while it stays held.
    b.poll(50u, BUTTON_LEFT);
    TEST_ASSERT_TRUE(b.is_held(BUTTON_LEFT));
    TEST_ASSERT_FALSE(b.was_pressed(BUTTON_LEFT));

    // Released steadily: clears after the window.
    b.poll(10u, BUTTON_NONE);
    b.poll(10u, BUTTON_NONE);
    b.poll(10u, BUTTON_NONE);
    TEST_ASSERT_FALSE(b.is_held(BUTTON_LEFT));
}

static void test_galaga_reset_starts_full_wave(void)
{
    game::Galaga g(240, 240);
    TEST_ASSERT_EQUAL_INT(game::Galaga::kMaxEnemies, g.alive_count());
    TEST_ASSERT_EQUAL_INT(0, g.score());
    TEST_ASSERT_EQUAL_INT((int)game::GalagaState::playing, (int)g.state());
}

static void test_galaga_overlaps_math(void)
{
    TEST_ASSERT_TRUE(game::Galaga::overlaps(0, 0, 4, 4, 2, 2, 4, 4));
    TEST_ASSERT_FALSE(game::Galaga::overlaps(0, 0, 4, 4, 10, 10, 4, 4));
    TEST_ASSERT_FALSE(game::Galaga::overlaps(0, 0, 4, 4, 4, 0, 4, 4));  // touching edge, no overlap
}

static void test_galaga_fire_spawns_one_bullet(void)
{
    game::Galaga g(240, 240);
    g.step(1u, 0u, true);   // fire edge

    int active = 0;
    const game::Bullet* b = g.bullets();
    for (int i = 0; i < game::Galaga::kMaxBullets; ++i) {
        if (b[i].active) { ++active; }
    }
    TEST_ASSERT_EQUAL_INT(1, active);
}

static void test_galaga_shots_destroy_enemies_and_score(void)
{
    game::Galaga g(240, 240);
    const int before = g.alive_count();
    for (int i = 0; i < 200 && g.state() == game::GalagaState::playing; ++i) {
        g.step(16u, 0u, (i % 8 == 0));   // fire every 8 ticks into the formation
    }
    TEST_ASSERT_TRUE(g.alive_count() < before);
    TEST_ASSERT_TRUE(g.score() > 0);
}

static void test_galaga_formation_reaching_bottom_loses(void)
{
    game::Galaga g(240, 240);
    for (int i = 0; i < 1000 && g.state() == game::GalagaState::playing; ++i) {
        g.step(16u, 0u, false);   // never shoot: the formation must reach the player
    }
    TEST_ASSERT_EQUAL_INT((int)game::GalagaState::lost, (int)g.state());
}

static void test_galaga_frozen_after_game_ends(void)
{
    game::Galaga g(240, 240);
    while (g.state() == game::GalagaState::playing) {
        g.step(16u, 0u, false);
    }
    const int frozen_score = g.score();
    g.step(16u, 0u, true);   // input after the game ended does nothing
    TEST_ASSERT_EQUAL_INT(frozen_score, g.score());
}

static void test_firefly_reset_starts_ready(void)
{
    game::FireflyCatch g(240, 240, 1u);
    TEST_ASSERT_EQUAL_INT(0, g.score());
    TEST_ASSERT_EQUAL_INT(game::FireflyCatch::kStartLives, g.lives());
    TEST_ASSERT_EQUAL_INT((int)game::CatchState::playing, (int)g.state());
    TEST_ASSERT_TRUE(g.flies()[0].active);   // opening firefly present
}

static void test_firefly_same_seed_is_deterministic(void)
{
    game::FireflyCatch a(240, 240, 12345u);
    game::FireflyCatch b(240, 240, 12345u);
    for (int i = 0; i < 300; ++i) {
        a.step(16u, 0u);
        b.step(16u, 0u);
    }
    TEST_ASSERT_EQUAL_INT(a.score(), b.score());
    TEST_ASSERT_EQUAL_INT(a.lives(), b.lives());
}

static void test_firefly_centered_opener_is_caught(void)
{
    // The first firefly is centered and the jar starts centered, so with no input
    // it should be caught before any later firefly reaches the bottom.
    game::FireflyCatch g(240, 240, 7u);
    for (int i = 0; i < 80; ++i) {
        g.step(16u, 0u);
    }
    TEST_ASSERT_TRUE(g.score() >= 10);
    TEST_ASSERT_EQUAL_INT(game::FireflyCatch::kStartLives, g.lives());
}

static void test_firefly_missing_costs_lives_and_ends(void)
{
    // Pin the jar to the left edge so centered/right fireflies are missed.
    game::FireflyCatch g(240, 240, 3u);
    for (int i = 0; i < 2000 && g.state() == game::CatchState::playing; ++i) {
        g.step(16u, ui::BUTTON_LEFT);
    }
    TEST_ASSERT_EQUAL_INT((int)game::CatchState::over, (int)g.state());
    TEST_ASSERT_EQUAL_INT(0, g.lives());
}

static void test_clock_format_pads_and_names(void)
{
    char buf[24];
    ui::clock_format::hhmm(9, 5, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("09:05", buf);
    ui::clock_format::hhmm(23, 59, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("23:59", buf);
    TEST_ASSERT_EQUAL_STRING("MON", ui::clock_format::weekday(1));
    TEST_ASSERT_EQUAL_STRING("JUL", ui::clock_format::month(6));
    ui::clock_format::date_str(1, 6, 12, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("MON JUL 12", buf);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_manager_no_active_is_error);
    RUN_TEST(test_manager_rejects_out_of_range);
    RUN_TEST(test_manager_rejects_empty_slot);
    RUN_TEST(test_manager_rejects_null_screen);
    RUN_TEST(test_manager_delegates_to_active_only);
    RUN_TEST(test_manager_switches_active);
    RUN_TEST(test_face_renders_eyes_open);
    RUN_TEST(test_face_blinks_briefly_then_reopens);
    RUN_TEST(test_shapes_screen_cycles_background);
    RUN_TEST(test_buttons_debounce_press_release);
    RUN_TEST(test_galaga_reset_starts_full_wave);
    RUN_TEST(test_galaga_overlaps_math);
    RUN_TEST(test_galaga_fire_spawns_one_bullet);
    RUN_TEST(test_galaga_shots_destroy_enemies_and_score);
    RUN_TEST(test_galaga_formation_reaching_bottom_loses);
    RUN_TEST(test_galaga_frozen_after_game_ends);
    RUN_TEST(test_firefly_reset_starts_ready);
    RUN_TEST(test_firefly_same_seed_is_deterministic);
    RUN_TEST(test_firefly_centered_opener_is_caught);
    RUN_TEST(test_firefly_missing_costs_lives_and_ends);
    RUN_TEST(test_clock_format_pads_and_names);
    return UNITY_END();
}
