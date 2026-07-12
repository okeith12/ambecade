#ifndef UI_PROCEDURAL_FACE_HPP
#define UI_PROCEDURAL_FACE_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"

namespace ui {

/* A face generated at runtime from canvas primitives (no assets) -- the
   procedural counterpart to a BitmapFace. The eyes stay open most of the time
   and shut for a brief moment each cycle, the way a real blink reads, rather
   than a 50/50 square wave. All geometry is derived from
   canvas.width()/height() -- never a hardcoded resolution -- so it scales to any
   canvas. */
class ProceduralFace : public Screen {
public:
    void update(std::uint32_t dt_ms) override;
    void render(gfx::Canvas& canvas) override;

    // True while the eyes are drawn closed (exposed for tests).
    bool blinking() const { return phase_ms_ >= kEyesOpenMs; }

private:
    static constexpr std::uint32_t kEyesOpenMs = 3000u;   // eyes open this long
    static constexpr std::uint32_t kEyesShutMs = 150u;    // then shut this briefly
    static constexpr std::uint32_t kCycleMs = kEyesOpenMs + kEyesShutMs;
    std::uint32_t phase_ms_ = 0u;   // position within the open-then-shut cycle
};

}  // namespace ui

#endif  // UI_PROCEDURAL_FACE_HPP
