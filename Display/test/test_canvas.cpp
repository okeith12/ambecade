#include <unity.h>
#include "color.hpp"
#include "canvas.hpp"
#include "framebuffer_canvas.hpp"

extern "C" {
#include "mock_spi.h"
#include "st7789.h"
}

using namespace gfx;

void setUp(void) {}
void tearDown(void) {}

static void test_rgb565_packing(void)
{
    TEST_ASSERT_EQUAL_HEX16(0xF800u, rgb565(255u, 0u, 0u));
    TEST_ASSERT_EQUAL_HEX16(0x07E0u, rgb565(0u, 255u, 0u));
    TEST_ASSERT_EQUAL_HEX16(0x001Fu, rgb565(0u, 0u, 255u));
    TEST_ASSERT_EQUAL_HEX16(0xFFFFu, rgb565(255u, 255u, 255u));
}

static void test_dimensions(void)
{
    FramebufferCanvas<32, 24> fb;
    TEST_ASSERT_EQUAL_INT16(32, fb.width());
    TEST_ASSERT_EQUAL_INT16(24, fb.height());
}

static void test_clear_fills_all(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::blue);
    for (std::int16_t y = 0; y < 8; ++y) {
        for (std::int16_t x = 0; x < 8; ++x) {
            TEST_ASSERT_EQUAL_HEX16(color::blue, fb.pixel_at(x, y));
        }
    }
}

static void test_draw_pixel_sets_only_target(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.draw_pixel(3, 4, color::red);
    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(3, 4));
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(3, 3));
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(2, 4));
}

static void test_draw_pixel_out_of_bounds_ignored(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.draw_pixel(-1, 4, color::red);
    fb.draw_pixel(8, 4, color::red);
    fb.draw_pixel(4, -1, color::red);
    fb.draw_pixel(4, 8, color::red);
    for (std::int16_t y = 0; y < 8; ++y) {
        for (std::int16_t x = 0; x < 8; ++x) {
            TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(x, y));
        }
    }
}

static void test_fill_rect_region(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.fill_rect(2, 2, 3, 3, color::green);

    for (std::int16_t y = 0; y < 8; ++y) {
        for (std::int16_t x = 0; x < 8; ++x) {
            bool inside = (x >= 2 && x < 5 && y >= 2 && y < 5);
            color_t expected = inside ? color::green : color::black;
            TEST_ASSERT_EQUAL_HEX16(expected, fb.pixel_at(x, y));
        }
    }
}

static void test_fill_rect_clips_negative_origin(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.fill_rect(-2, -2, 4, 4, color::red);

    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(0, 0));
    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(1, 1));
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(2, 2));
}

static void test_fill_rect_clips_far_edge(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.fill_rect(6, 6, 10, 10, color::red);

    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(6, 6));
    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(7, 7));
}

static void test_draw_rect_outline_only(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    fb.draw_rect(1, 1, 4, 4, color::white);

    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(1, 1));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(4, 1));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(1, 4));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(4, 4));
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(2, 2));
    TEST_ASSERT_EQUAL_HEX16(color::black, fb.pixel_at(3, 3));
}

static void test_blit_copies_with_clipping(void)
{
    FramebufferCanvas<8, 8> fb;
    fb.clear(color::black);
    const color_t src[4] = { color::red, color::green, color::blue, color::white };
    fb.blit(src, 6, 6, 2, 2);

    TEST_ASSERT_EQUAL_HEX16(color::red, fb.pixel_at(6, 6));
    TEST_ASSERT_EQUAL_HEX16(color::green, fb.pixel_at(7, 6));
    TEST_ASSERT_EQUAL_HEX16(color::blue, fb.pixel_at(6, 7));
    TEST_ASSERT_EQUAL_HEX16(color::white, fb.pixel_at(7, 7));
}

static void test_flush_rejects_null_driver(void)
{
    FramebufferCanvas<2, 2> fb;
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_NULL, fb.flush(nullptr));
}

static void test_flush_rejects_size_mismatch(void)
{
    mock_spi_t mock;
    spi_bus_t bus;
    st7789_driver_t drv;
    mock_spi_init(&mock);
    mock_spi_bind(&bus, &mock);
    st7789_init(&drv, &bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);

    FramebufferCanvas<2, 2> fb;
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_PARAM, fb.flush(&drv));
}

static void test_flush_emits_window_and_pixels(void)
{
    mock_spi_t mock;
    spi_bus_t bus;
    st7789_driver_t drv;
    mock_spi_init(&mock);
    mock_spi_bind(&bus, &mock);
    st7789_init(&drv, &bus, 2u, 2u, 0u, 0u, ST7789_INVERSION_ON);

    FramebufferCanvas<2, 2> fb;
    fb.draw_pixel(0, 0, 0xF800u);
    fb.draw_pixel(1, 0, 0x07E0u);
    fb.draw_pixel(0, 1, 0x001Fu);
    fb.draw_pixel(1, 1, 0xFFFFu);

    mock_spi_init(&mock);
    TEST_ASSERT_EQUAL_INT(ST7789_OK, fb.flush(&drv));

    const std::uint8_t expected[] = {
        ST7789_CMD_CASET, 0x00u, 0x00u, 0x00u, 0x01u,
        ST7789_CMD_RASET, 0x00u, 0x00u, 0x00u, 0x01u,
        ST7789_CMD_RAMWR,
        0xF8u, 0x00u,
        0x07u, 0xE0u,
        0x00u, 0x1Fu,
        0xFFu, 0xFFu
    };
    std::uint8_t got[32];
    std::size_t n = mock_spi_collect_bytes(&mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
    TEST_ASSERT_FALSE(mock.overflow);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_rgb565_packing);
    RUN_TEST(test_dimensions);
    RUN_TEST(test_clear_fills_all);
    RUN_TEST(test_draw_pixel_sets_only_target);
    RUN_TEST(test_draw_pixel_out_of_bounds_ignored);
    RUN_TEST(test_fill_rect_region);
    RUN_TEST(test_fill_rect_clips_negative_origin);
    RUN_TEST(test_fill_rect_clips_far_edge);
    RUN_TEST(test_draw_rect_outline_only);
    RUN_TEST(test_blit_copies_with_clipping);
    RUN_TEST(test_flush_rejects_null_driver);
    RUN_TEST(test_flush_rejects_size_mismatch);
    RUN_TEST(test_flush_emits_window_and_pixels);
    return UNITY_END();
}
