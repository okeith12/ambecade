#include <unity.h>
#include "color.hpp"
#include "canvas.hpp"
#include "framebuffer_canvas.hpp"
#include "bitmap.hpp"
#include "bitmap_face.hpp"
#include "frog_face.hpp"
#include "font.hpp"

extern "C" {
#include "storage.h"
#include "mock_storage.h"
}

using namespace gfx;

void setUp(void) {}
void tearDown(void) {}

// A 2x2 RGB565 image stored big-endian: red, green / blue, white.
static const std::uint8_t kBlob2x2[] = {
    0xF8u, 0x00u,  0x07u, 0xE0u,
    0x00u, 0x1Fu,  0xFFu, 0xFFu
};

static void test_reader_validate(void)
{
    mock_storage_t m;
    storage_reader_t r;
    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    mock_storage_bind(&r, &m);
    TEST_ASSERT_EQUAL_INT(STORAGE_OK, storage_reader_validate(&r));
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_NULL, storage_reader_validate(nullptr));
}

static void test_load_success_assembles_big_endian(void)
{
    mock_storage_t m;
    storage_reader_t r;
    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    mock_storage_bind(&r, &m);

    color_t buf[4];
    Bitmap bmp;
    TEST_ASSERT_EQUAL_INT(STORAGE_OK,
        load_bitmap(r, "frog_face.rgb565", 2, 2, buf, 4, bmp));

    TEST_ASSERT_EQUAL_INT16(2, bmp.width);
    TEST_ASSERT_EQUAL_INT16(2, bmp.height);
    TEST_ASSERT_EQUAL_PTR(buf, bmp.pixels);
    TEST_ASSERT_EQUAL_HEX16(0xF800u, buf[0]);
    TEST_ASSERT_EQUAL_HEX16(0x07E0u, buf[1]);
    TEST_ASSERT_EQUAL_HEX16(0x001Fu, buf[2]);
    TEST_ASSERT_EQUAL_HEX16(0xFFFFu, buf[3]);
}

static void test_load_short_read_errors_without_overrun(void)
{
    mock_storage_t m;
    storage_reader_t r;
    mock_storage_init(&m, kBlob2x2, 6u);  // only 6 of the needed 8 bytes
    mock_storage_bind(&r, &m);

    color_t buf[4] = { 0x1111u, 0x1111u, 0x1111u, 0x1111u };
    Bitmap bmp = { nullptr, 0, 0 };
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_SHORT,
        load_bitmap(r, "frog_face.rgb565", 2, 2, buf, 4, bmp));

    // Buffer left untouched, nothing written past the request.
    TEST_ASSERT_EQUAL_HEX16(0x1111u, buf[0]);
    TEST_ASSERT_EQUAL_HEX16(0x1111u, buf[3]);
}

static void test_load_buffer_too_small(void)
{
    mock_storage_t m;
    storage_reader_t r;
    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    mock_storage_bind(&r, &m);

    color_t buf[2];  // capacity 2 pixels, image needs 4
    Bitmap bmp;
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_RANGE,
        load_bitmap(r, "frog_face.rgb565", 2, 2, buf, 2, bmp));
}

static void test_load_rejects_null_and_bad_dims(void)
{
    mock_storage_t m;
    storage_reader_t r;
    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    mock_storage_bind(&r, &m);

    color_t buf[4];
    Bitmap bmp;
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_NULL,
        load_bitmap(r, nullptr, 2, 2, buf, 4, bmp));
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_NULL,
        load_bitmap(r, "frog_face.rgb565", 2, 2, nullptr, 4, bmp));
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_RANGE,
        load_bitmap(r, "frog_face.rgb565", 0, 2, buf, 4, bmp));
}

static void test_load_propagates_not_found_and_io(void)
{
    mock_storage_t m;
    storage_reader_t r;
    color_t buf[4];
    Bitmap bmp;

    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    m.not_found = 1u;
    mock_storage_bind(&r, &m);
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_NOT_FOUND,
        load_bitmap(r, "frog_face.rgb565", 2, 2, buf, 4, bmp));

    mock_storage_init(&m, kBlob2x2, sizeof(kBlob2x2));
    m.fail_io = 1u;
    TEST_ASSERT_EQUAL_INT(STORAGE_ERR_IO,
        load_bitmap(r, "frog_face.rgb565", 2, 2, buf, 4, bmp));
}

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

static void test_font_draws_glyph_pixels(void)
{
    FramebufferCanvas<16, 8> fb;
    fb.clear(color::black);
    gfx::draw_char(fb, 'I', 0, 0, 1, color::white);

    // The top row of 'I' is " ### ": columns 1..3 lit, column 0 dark.
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(0, 0));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(1, 0));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(3, 0));
}

static void test_font_text_advances_per_glyph(void)
{
    FramebufferCanvas<32, 8> fb;
    fb.clear(color::black);
    const std::int16_t end = gfx::draw_text(fb, "HI", 0, 0, 1, color::white);
    TEST_ASSERT_EQUAL_INT16((5 + 1) * 2, end);   // two glyphs, advance 6 each
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_reader_validate);
    RUN_TEST(test_load_success_assembles_big_endian);
    RUN_TEST(test_load_short_read_errors_without_overrun);
    RUN_TEST(test_load_buffer_too_small);
    RUN_TEST(test_load_rejects_null_and_bad_dims);
    RUN_TEST(test_load_propagates_not_found_and_io);
    RUN_TEST(test_bitmap_face_scales_and_centers);
    RUN_TEST(test_bitmap_face_clears_background);
    RUN_TEST(test_frog_face_is_named_and_well_formed);
    RUN_TEST(test_frog_face_renders_through_bitmap_face);
    RUN_TEST(test_font_draws_glyph_pixels);
    RUN_TEST(test_font_text_advances_per_glyph);
    return UNITY_END();
}
