/* Ambecade display demo.

   Cycles three screens on the ST7789 panel: changing colors and shapes, then the
   procedural animated face, then the frog bitmap. Drawing goes to an off-screen
   FramebufferCanvas and is pushed to the panel with flush(); the panel is driven
   through the same spi_bus seam the host tests mock, here backed by Arduino SPI. */

#include <Arduino.h>
#include <SPI.h>

#include "framebuffer_canvas.hpp"
#include "screen.hpp"
#include "screen_id.hpp"
#include "screen_manager.hpp"
#include "shapes_screen.hpp"
#include "procedural_face.hpp"
#include "bitmap_face.hpp"
#include "frog_face.hpp"

extern "C" {
#include "st7789.h"
#include "spi_bus.h"
}

// ---- Wiring (adjust these to match how you soldered the panel) ----
// XIAO ESP32-S3 hardware SPI: SCK = D8 (GPIO7), MOSI = D10 (GPIO9).
static constexpr int8_t kPinSck = 7;
static constexpr int8_t kPinMosi = 9;
static constexpr uint8_t kPinCs = 2;   // D1
static constexpr uint8_t kPinDc = 3;   // D2
static constexpr uint8_t kPinRst = 4;  // D3
static constexpr uint8_t kPinBacklight = 1;  // D0 (or tie the panel's BLK to 3V3)

static constexpr uint32_t kSpiHz = 40000000u;  // 40 MHz SPI clock

// ---- The hardware seam: Arduino SPI + GPIO behind the spi_bus ops table ----

static spi_bus_status_t hw_transfer_byte(void*, uint8_t value)
{
    SPI.transfer(value);
    return SPI_BUS_OK;
}

static spi_bus_status_t hw_transfer_buffer(void*, const uint8_t* data, size_t len)
{
    SPI.writeBytes(data, len);  // write-only; does not clobber the source buffer
    return SPI_BUS_OK;
}

static spi_bus_status_t hw_set_cs(void*, gpio_level_t level)
{
    digitalWrite(kPinCs, level == GPIO_LEVEL_HIGH ? HIGH : LOW);
    return SPI_BUS_OK;
}

static spi_bus_status_t hw_set_dc(void*, gpio_level_t level)
{
    digitalWrite(kPinDc, level == GPIO_LEVEL_HIGH ? HIGH : LOW);
    return SPI_BUS_OK;
}

static spi_bus_status_t hw_set_reset(void*, gpio_level_t level)
{
    digitalWrite(kPinRst, level == GPIO_LEVEL_HIGH ? HIGH : LOW);
    return SPI_BUS_OK;
}

static spi_bus_status_t hw_delay_ms(void*, uint32_t ms)
{
    delay(ms);
    return SPI_BUS_OK;
}

static const spi_bus_ops_t kBusOps = {
    hw_transfer_byte, hw_transfer_buffer, hw_set_cs, hw_set_dc, hw_set_reset, hw_delay_ms
};
static spi_bus_t g_bus = { &kBusOps, nullptr };
static st7789_driver_t g_drv;

// ---- App state: framebuffer and screens live in static storage, never the stack ----
static gfx::FramebufferCanvas<240, 240> g_canvas;
static ui::ShapesScreen g_shapes;
static ui::ProceduralFace g_face;
static ui::BitmapFace g_frog(gfx::frog_face);
static ui::ScreenManager<ui::screen_id> g_screens;

// The order screens are shown, and how long each stays up.
static const ui::screen_id kOrder[] = {
    ui::screen_id::shapes, ui::screen_id::face, ui::screen_id::bitmap
};
static constexpr uint32_t kScreenHoldMs = 3000u;

static uint32_t g_last_ms = 0u;
static uint32_t g_screen_ms = 0u;
static size_t g_screen_index = 0u;

void setup()
{
    pinMode(kPinCs, OUTPUT);
    pinMode(kPinDc, OUTPUT);
    pinMode(kPinRst, OUTPUT);
    pinMode(kPinBacklight, OUTPUT);
    digitalWrite(kPinCs, HIGH);
    digitalWrite(kPinBacklight, HIGH);  // backlight on

    SPI.begin(kPinSck, -1, kPinMosi, -1);
    SPI.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));

    // 1.54" IPS panel is Normally-Black, so it needs inversion on.
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    st7789_set_rotation(&g_drv, ST7789_ROTATION_0);

    g_screens.set_screen(ui::screen_id::shapes, &g_shapes);
    g_screens.set_screen(ui::screen_id::face, &g_face);
    g_screens.set_screen(ui::screen_id::bitmap, &g_frog);
    g_screens.set_active(kOrder[g_screen_index]);

    g_last_ms = millis();
}

void loop()
{
    const uint32_t now = millis();
    const uint32_t dt = now - g_last_ms;
    g_last_ms = now;

    g_screens.update(dt);
    g_screens.render(g_canvas);
    g_canvas.flush(&g_drv);

    // Advance to the next screen on a timer (input-driven switching comes later).
    g_screen_ms += dt;
    if (g_screen_ms >= kScreenHoldMs) {
        g_screen_ms = 0u;
        g_screen_index = (g_screen_index + 1u) % (sizeof(kOrder) / sizeof(kOrder[0]));
        g_screens.set_active(kOrder[g_screen_index]);
    }
}
