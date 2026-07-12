#include <unity.h>
#include "canvas.hpp"
#include "color.hpp"
#include "framebuffer_canvas.hpp"
#include "screen.hpp"
#include "screen_manager.hpp"
#include "procedural_face.hpp"

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
    return UNITY_END();
}
