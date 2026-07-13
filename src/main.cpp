/* Ambecade display demo.

   Cycles three screens on the ST7789 panel: changing colors and shapes, then the
   procedural animated face, then the frog bitmap. Drawing goes to an off-screen
   FramebufferCanvas and is pushed to the panel with flush(); the panel is driven
   through the same spi_bus seam the host tests mock, here backed by Arduino SPI. */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#include "framebuffer_canvas.hpp"
#include "screen.hpp"
#include "screen_id.hpp"
#include "screen_manager.hpp"
#include "shapes_screen.hpp"
#include "procedural_face.hpp"
#include "bitmap_face.hpp"
#include "frog_face.hpp"
#include "buttons.hpp"
#include "galaga_screen.hpp"
#include "firefly_catch_screen.hpp"
#include "clock_screen.hpp"

extern "C" {
#include "st7789.h"
#include "spi_bus.h"
}

// ---- WiFi + time (for the clock) ----
// Credentials come from src/secrets.h (gitignored). Copy src/secrets.example.h to
// src/secrets.h and fill it in. Without it, WiFi is skipped and the clock shows a placeholder.
#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif
static const char* kWifiSsid = WIFI_SSID;
static const char* kWifiPass = WIFI_PASS;
// America/Chicago with automatic daylight saving.
static const char* kTimezone = "CST6CDT,M3.2.0,M11.1.0";

// ---- Display wiring: matches the XIAO ESP32-S3 silk labels (BLK tied to 3V3) ----
static constexpr int8_t kPinSck = D8;    // CK
static constexpr int8_t kPinMosi = D10;  // SI
static constexpr uint8_t kPinCs = D7;    // CS
static constexpr uint8_t kPinDc = D5;    // DC
static constexpr uint8_t kPinRst = D6;   // RT

// ---- Button wiring: switch to GND, INPUT_PULLUP (pressed reads LOW) ----
// Wire whatever switches you have; unwired pins float high and read as unpressed.
static constexpr uint8_t kPinSelect = D0;  // change screen
static constexpr uint8_t kPinLeft = D1;    // Galaga: move left
static constexpr uint8_t kPinRight = D2;   // Galaga: move right
static constexpr uint8_t kPinFire = D3;    // Galaga: fire / retry

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
static ui::Buttons g_buttons;
static ui::ShapesScreen g_shapes;
static ui::ProceduralFace g_face;
static ui::BitmapFace g_frog(gfx::frog_face);
static ui::GalagaScreen g_game(g_buttons, 240, 240);
static ui::FireflyCatchScreen g_firefly(g_buttons, 240, 240, 0xBEEF1234u);
static ui::ClockScreen g_clock;
static ui::ScreenManager<ui::screen_id> g_screens;

// The order screens cycle in; each one shows for a minute before the next.
static const ui::screen_id kOrder[] = {
    ui::screen_id::shapes, ui::screen_id::face, ui::screen_id::bitmap,
    ui::screen_id::game, ui::screen_id::firefly, ui::screen_id::clock
};

// Connects WiFi (if configured) and starts NTP time for the clock.
static void start_wifi_and_time()
{
    if (kWifiSsid[0] == '\0') {
        return;   // no credentials: the clock shows a placeholder
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(kWifiSsid, kWifiPass);
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000u) {
        delay(200);
    }
    if (WiFi.status() == WL_CONNECTED) {
        configTzTime(kTimezone, "pool.ntp.org", "time.nist.gov");
    }
}
static constexpr uint32_t kScreenHoldMs = 60000u;   // one minute per screen

static uint32_t g_last_ms = 0u;
static uint32_t g_screen_ms = 0u;
static size_t g_screen_index = 0u;

// Advances to the next screen and resets the auto-advance timer.
static void next_screen()
{
    g_screen_ms = 0u;
    g_screen_index = (g_screen_index + 1u) % (sizeof(kOrder) / sizeof(kOrder[0]));
    g_screens.set_active(kOrder[g_screen_index]);
}

// Reads the buttons into a debounced state; call once per frame.
static void poll_buttons(uint32_t dt)
{
    uint32_t raw = 0u;
    if (digitalRead(kPinSelect) == LOW) { raw |= ui::BUTTON_SELECT; }
    if (digitalRead(kPinLeft) == LOW)   { raw |= ui::BUTTON_LEFT; }
    if (digitalRead(kPinRight) == LOW)  { raw |= ui::BUTTON_RIGHT; }
    if (digitalRead(kPinFire) == LOW)   { raw |= ui::BUTTON_FIRE; }
    g_buttons.poll(dt, raw);
}

void setup()
{
    pinMode(kPinCs, OUTPUT);
    pinMode(kPinDc, OUTPUT);
    pinMode(kPinRst, OUTPUT);
    digitalWrite(kPinCs, HIGH);

    pinMode(kPinSelect, INPUT_PULLUP);
    pinMode(kPinLeft, INPUT_PULLUP);
    pinMode(kPinRight, INPUT_PULLUP);
    pinMode(kPinFire, INPUT_PULLUP);

    SPI.begin(kPinSck, -1, kPinMosi, -1);
    // This panel clocks in SPI mode 3 (matches the proven Adafruit bring-up).
    SPI.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE3));

    // 1.54" IPS panel is Normally-Black, so it needs inversion on.
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    // Adafruit's setRotation(2) on this 240x240 panel is MADCTL 0x00 with no
    // offset, which is our ROTATION_0 -- reproduces the proven GRAM mapping.
    st7789_set_rotation(&g_drv, ST7789_ROTATION_0);

    start_wifi_and_time();

    g_screens.set_screen(ui::screen_id::shapes, &g_shapes);
    g_screens.set_screen(ui::screen_id::face, &g_face);
    g_screens.set_screen(ui::screen_id::bitmap, &g_frog);
    g_screens.set_screen(ui::screen_id::game, &g_game);
    g_screens.set_screen(ui::screen_id::firefly, &g_firefly);
    g_screens.set_screen(ui::screen_id::clock, &g_clock);
    g_screens.set_active(kOrder[g_screen_index]);

    g_last_ms = millis();
}

void loop()
{
    const uint32_t now = millis();
    const uint32_t dt = now - g_last_ms;
    g_last_ms = now;

    poll_buttons(dt);

    g_screens.update(dt);
    g_screens.render(g_canvas);
    g_canvas.flush(&g_drv);

    // Select advances immediately; otherwise each screen auto-advances on a timer,
    // except the game, which stays put so it is not yanked away mid-play.
    if (g_buttons.was_pressed(ui::BUTTON_SELECT)) {
        next_screen();
        return;
    }
    g_screen_ms += dt;
    if (g_screen_ms >= kScreenHoldMs) {
        next_screen();
    }
}
