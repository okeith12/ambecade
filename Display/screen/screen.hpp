#ifndef UI_SCREEN_HPP
#define UI_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"

namespace ui {

/* An updatable, drawable screen. Screens advance their own state over time and
   render through a gfx::Canvas only -- they never touch the display driver, and
   they read canvas.width()/height() rather than assuming a fixed resolution. */
class Screen {
public:
    virtual ~Screen() = default;

    // Advance internal state by dt_ms milliseconds (animation, timers).
    virtual void update(std::uint32_t dt_ms) = 0;

    // Draw the current state to the canvas.
    virtual void render(gfx::Canvas& canvas) = 0;
};

}  // namespace ui

#endif  // UI_SCREEN_HPP
