#ifndef UI_FIREFLY_CATCH_SCREEN_HPP
#define UI_FIREFLY_CATCH_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"
#include "buttons.hpp"
#include "firefly_catch.hpp"

namespace ui {

/* Plays Firefly Catch as a Screen: reads the shared Buttons to slide the jar
   left and right, and draws the fireflies, jar, and score over a night sky.
   Fire restarts after a loss. */
class FireflyCatchScreen : public Screen {
public:
    FireflyCatchScreen(const Buttons& buttons, std::int16_t field_w, std::int16_t field_h,
                       std::uint32_t seed)
        : buttons_(buttons), game_(field_w, field_h, seed) {}

    void update(std::uint32_t dt_ms) override;
    void render(gfx::Canvas& canvas) override;

private:
    const Buttons& buttons_;
    game::FireflyCatch game_;
};

}  // namespace ui

#endif  // UI_FIREFLY_CATCH_SCREEN_HPP
