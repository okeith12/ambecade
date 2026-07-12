#ifndef UI_SHAPES_SCREEN_HPP
#define UI_SHAPES_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"

namespace ui {

/* A demo screen that cycles the background color and slides a square across the
   surface, to prove color and shape drawing on the panel. All geometry is
   derived from canvas.width()/height(), so it scales to any canvas. */
class ShapesScreen : public Screen {
public:
    void update(std::uint32_t dt_ms) override;
    void render(gfx::Canvas& canvas) override;

    // Elapsed animation time in milliseconds (exposed for tests).
    std::uint32_t elapsed() const { return elapsed_ms_; }

private:
    static constexpr std::uint32_t kColorStepMs = 500u;  // background advances each step
    std::uint32_t elapsed_ms_ = 0u;
};

}  // namespace ui

#endif  // UI_SHAPES_SCREEN_HPP
