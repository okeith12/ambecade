#include <unity.h>
#include "color.hpp"
#include "canvas.hpp"
#include "framebuffer_canvas.hpp"
#include "bitmap.hpp"
#include "bitmap_face.hpp"
#include "frog_face.hpp"
#include "seahorse.hpp"
#include "tiara.hpp"
#include "heart.hpp"
#include "anniversary.hpp"
#include "anniversary_screen.hpp"

using namespace gfx;

void setUp(void) {}
void tearDown(void) {}

static void test_bitmap_face_scales_and_centers(void)
{
    const color_t px[4] = { color::red, color::green, color::blue, color::white };
    Bitmap bmp = { px, 2, 2 };
    ui::BitmapFace face(bmp);

    FramebufferCanvas<4, 4> fb;
    face.render(fb);

    // 2x2 scaled 2x fills the 4x4 canvas: each source pixel becomes a 2x2 block.
    TEST_ASSERT_EQUAL_HEX16(color::red,   fb.pixel_at(0, 0));
    TEST_ASSERT_EQUAL_HEX16(color::red,   fb.pixel_at(1, 1));
    TEST_ASSERT_EQUAL_HEX16(color::green, fb.pixel_at(3, 0));
    TEST_ASSERT_EQUAL_HEX16(color::blue,  fb.pixel_at(0, 3));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(3, 3));
}

static void test_bitmap_face_clears_background(void)
{
    // A 2x2 bitmap on a 5x5 canvas scales 2x (4x4) and leaves a 1px margin that
    // must show the background, proving the screen clears before it draws.
    const color_t px[4] = { color::red, color::red, color::red, color::red };
    Bitmap bmp = { px, 2, 2 };
    ui::BitmapFace face(bmp, color::blue);

    FramebufferCanvas<5, 5> fb;
    fb.clear(color::white);   // stale content from a previous screen
    face.render(fb);

    TEST_ASSERT_EQUAL_HEX16(color::blue, fb.pixel_at(4, 4));  // margin is background
    TEST_ASSERT_EQUAL_HEX16(color::red,  fb.pixel_at(0, 0));  // bitmap drawn on top
}

static void test_frog_face_is_named_and_well_formed(void)
{
    // The asset describes itself: its name says what it is, its size travels with it.
    TEST_ASSERT_EQUAL_INT16(12, frog_face.width);
    TEST_ASSERT_EQUAL_INT16(12, frog_face.height);
    // Pixels land where the art places them.
    TEST_ASSERT_EQUAL_HEX16(frog::kWhite, frog_face.pixels[2 * 12 + 2]);   // left eye white
    TEST_ASSERT_EQUAL_HEX16(frog::kPupil, frog_face.pixels[3 * 12 + 3]);   // left pupil
    TEST_ASSERT_EQUAL_HEX16(frog::kGreen, frog_face.pixels[5 * 12 + 5]);   // body
    TEST_ASSERT_EQUAL_HEX16(frog::kBackground, frog_face.pixels[0]);       // corner
}

static void test_frog_face_renders_through_bitmap_face(void)
{
    ui::BitmapFace face(frog_face);
    FramebufferCanvas<12, 12> fb;
    fb.clear(color::black);
    face.render(fb);

    // 12x12 centered in a 12x12 canvas -> offset 0; the eye white shows at (2,2).
    TEST_ASSERT_EQUAL_HEX16(frog::kWhite, fb.pixel_at(2, 2));
    TEST_ASSERT_EQUAL_HEX16(frog::kGreen, fb.pixel_at(5, 5));
}

static void test_seahorse_asset_well_formed(void)
{
    TEST_ASSERT_EQUAL_INT16(18, seahorse.width);
    TEST_ASSERT_EQUAL_INT16(28, seahorse.height);
    TEST_ASSERT_EQUAL_HEX16(sea::kBody,    seahorse.pixels[5 * 18 + 5]);   // yellow body
    TEST_ASSERT_EQUAL_HEX16(sea::kOutline, seahorse.pixels[3 * 18 + 5]);   // black outline
    TEST_ASSERT_EQUAL_HEX16(sea::kFin,     seahorse.pixels[2 * 18 + 6]);   // blue fin
    TEST_ASSERT_EQUAL_HEX16(sea::kBg,      seahorse.pixels[0]);            // transparent corner
    TEST_ASSERT_EQUAL_HEX16(sea::kBg,      seahorse_key);                  // key matches background
}

static void test_heart_asset_well_formed(void)
{
    TEST_ASSERT_EQUAL_INT16(7, heart.width);
    TEST_ASSERT_EQUAL_INT16(7, heart.height);
    TEST_ASSERT_EQUAL_HEX16(hrt::kPink, heart.pixels[3 * 7 + 3]);   // filled center
    TEST_ASSERT_EQUAL_HEX16(hrt::kBg,   heart.pixels[0]);           // transparent corner
    TEST_ASSERT_EQUAL_HEX16(hrt::kBg,   heart_key);
}

static void test_tiara_asset_well_formed(void)
{
    TEST_ASSERT_EQUAL_INT16(22, tiara.width);
    TEST_ASSERT_EQUAL_INT16(15, tiara.height);
    TEST_ASSERT_EQUAL_HEX16(crown::kGold,    tiara.pixels[6 * 22 + 2]);    // gold body
    TEST_ASSERT_EQUAL_HEX16(crown::kGem,     tiara.pixels[10 * 22 + 9]);   // center pink gem
    TEST_ASSERT_EQUAL_HEX16(crown::kOutline, tiara.pixels[6 * 22 + 1]);    // black outline
    TEST_ASSERT_EQUAL_HEX16(crown::kBg,      tiara.pixels[0]);             // background corner
}

static void test_tiara_renders_through_bitmap_face(void)
{
    ui::BitmapFace face(tiara);
    FramebufferCanvas<22, 15> fb;
    fb.clear(color::black);
    face.render(fb);
    // 22x15 centered in a 22x15 canvas -> offset 0.
    TEST_ASSERT_EQUAL_HEX16(crown::kGold, fb.pixel_at(2, 6));
    TEST_ASSERT_EQUAL_HEX16(crown::kGem,  fb.pixel_at(9, 10));
}

static void test_anniversary_days_until_counts_forward(void)
{
    TEST_ASSERT_EQUAL_INT(0,   ui::anniversary::days_until(2026, 8, 16, 8, 16));  // today
    TEST_ASSERT_EQUAL_INT(1,   ui::anniversary::days_until(2026, 8, 15, 8, 16));  // tomorrow
    TEST_ASSERT_EQUAL_INT(1,   ui::anniversary::days_until(2026, 12, 31, 1, 1));  // new year
    TEST_ASSERT_EQUAL_INT(364, ui::anniversary::days_until(2026, 8, 17, 8, 16));  // rolled over
}

static void test_anniversary_days_until_handles_leap(void)
{
    // 2028 is a leap year: Feb 28 -> Mar 1 spans Feb 29 (two days).
    TEST_ASSERT_EQUAL_INT(2, ui::anniversary::days_until(2028, 2, 28, 3, 1));
    // 2027 is not a leap year: Feb 28 -> Mar 1 is one day.
    TEST_ASSERT_EQUAL_INT(1, ui::anniversary::days_until(2027, 2, 28, 3, 1));
}

static void test_anniversary_labels(void)
{
    char buf[16];
    ui::anniversary::date_label(8, 16, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("AUG 16", buf);
    ui::anniversary::countdown_label(0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("TODAY", buf);
    ui::anniversary::countdown_label(1, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("1 DAY", buf);
    ui::anniversary::countdown_label(42, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("42 DAYS", buf);
}

static void test_anniversary_screen_bops_up_and_down(void)
{
    ui::AnniversaryScreen s(seahorse, 8, 16, seahorse_key);
    const std::int16_t amp = ui::AnniversaryScreen::kBopAmp;
    TEST_ASSERT_EQUAL_INT16(0, s.bob_offset());     // phase 0: centered
    s.update(300u);                                  // quarter cycle: peak up
    TEST_ASSERT_EQUAL_INT16(-amp, s.bob_offset());
    s.update(300u);                                  // half cycle: centered
    TEST_ASSERT_EQUAL_INT16(0, s.bob_offset());
    s.update(300u);                                  // three-quarter: peak down
    TEST_ASSERT_EQUAL_INT16(amp, s.bob_offset());
    s.update(300u);                                  // full cycle wraps back to 0
    TEST_ASSERT_EQUAL_INT16(0, s.bob_offset());
}

static void test_anniversary_screen_draws_seahorse(void)
{
    ui::AnniversaryScreen s(seahorse, 8, 16, seahorse_key);
    FramebufferCanvas<80, 140> fb;
    fb.clear(color::white);   // stale content from a previous screen

    s.render(fb);

    TEST_ASSERT_EQUAL_HEX16(sea::kBg, fb.pixel_at(0, 0));   // background cleared the white
    bool found_body = false;
    for (std::int16_t y = 0; y < 140 && !found_body; ++y) {
        for (std::int16_t x = 0; x < 80; ++x) {
            if (fb.pixel_at(x, y) == sea::kBody) { found_body = true; break; }
        }
    }
    TEST_ASSERT_TRUE(found_body);   // the seahorse actually painted
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bitmap_face_scales_and_centers);
    RUN_TEST(test_bitmap_face_clears_background);
    RUN_TEST(test_frog_face_is_named_and_well_formed);
    RUN_TEST(test_frog_face_renders_through_bitmap_face);
    RUN_TEST(test_seahorse_asset_well_formed);
    RUN_TEST(test_heart_asset_well_formed);
    RUN_TEST(test_tiara_asset_well_formed);
    RUN_TEST(test_tiara_renders_through_bitmap_face);
    RUN_TEST(test_anniversary_days_until_counts_forward);
    RUN_TEST(test_anniversary_days_until_handles_leap);
    RUN_TEST(test_anniversary_labels);
    RUN_TEST(test_anniversary_screen_bops_up_and_down);
    RUN_TEST(test_anniversary_screen_draws_seahorse);
    return UNITY_END();
}
