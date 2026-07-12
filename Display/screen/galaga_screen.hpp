#ifndef UI_GALAGA_SCREEN_HPP
#define UI_GALAGA_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"
#include "buttons.hpp"
#include "galaga.hpp"

namespace ui {

/* Plays Galaga as a Screen: reads the shared Buttons each frame to move and
   fire, and draws the field with canvas primitives and the bitmap font. Left,
   right, and fire drive play; fire restarts after a win or loss. */
class GalagaScreen : public Screen {
public:
    GalagaScreen(const Buttons& buttons, std::int16_t field_w, std::int16_t field_h)
        : buttons_(buttons), game_(field_w, field_h) {}

    void update(std::uint32_t dt_ms) override;
    void render(gfx::Canvas& canvas) override;

private:
    const Buttons& buttons_;
    game::Galaga game_;
};

}  // namespace ui

#endif  // UI_GALAGA_SCREEN_HPP
