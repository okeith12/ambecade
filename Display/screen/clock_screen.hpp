#ifndef UI_CLOCK_SCREEN_HPP
#define UI_CLOCK_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"

namespace ui {

/* A day clock that reads the system time (set over WiFi/NTP in main) and draws a
   big HH:MM with the date below. Until the time is synced it shows a placeholder. */
class ClockScreen : public Screen {
public:
    void update(std::uint32_t dt_ms) override;   // reads the clock in render
    void render(gfx::Canvas& canvas) override;
};

}  // namespace ui

#endif  // UI_CLOCK_SCREEN_HPP
