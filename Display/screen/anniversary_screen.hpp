#ifndef UI_ANNIVERSARY_SCREEN_HPP
#define UI_ANNIVERSARY_SCREEN_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"
#include "bitmap.hpp"
#include "color.hpp"

namespace ui {

/* Anniversary screen: a sprite (the seahorse) that bops gently up and down over a
   solid background, with the anniversary date and a live "days until" countdown
   below it. The sprite's transparent key pixels are skipped so it floats on the
   background. Geometry is derived from canvas.width()/height(), so it scales to
   any canvas. The countdown reads the system clock in render(); the bop math and
   the countdown math are pure and unit-tested. */
class AnniversaryScreen : public Screen {
public:
    AnniversaryScreen(const gfx::Bitmap& sprite, int ann_month, int ann_day,
                      gfx::color_t sprite_key,
                      const char* tagline = "TILL WE CELEBRATE OUR FAMILY",
                      gfx::color_t background = gfx::rgb565(26, 14, 26))
        : sprite_(sprite), ann_month_(ann_month), ann_day_(ann_day),
          sprite_key_(sprite_key), tagline_(tagline), background_(background) {}

    void update(std::uint32_t dt_ms) override;   // advances the bop phase
    void render(gfx::Canvas& canvas) override;

    // Current vertical bop offset in logical pixels (exposed for tests): a
    // triangle wave 0 -> -amp (up) -> 0 -> +amp (down) -> 0 over one cycle.
    std::int16_t bob_offset() const;

    // Full period of one bop, and its peak amplitude (pre-scale), for tests.
    static constexpr std::uint32_t kBopMs = 1200u;
    static constexpr std::int16_t kBopAmp = 4;

private:
    gfx::Bitmap sprite_;
    int ann_month_;
    int ann_day_;
    gfx::color_t sprite_key_;
    const char* tagline_;
    gfx::color_t background_;
    std::uint32_t phase_ms_ = 0u;   // position within the bop cycle
};

}  // namespace ui

#endif  // UI_ANNIVERSARY_SCREEN_HPP
