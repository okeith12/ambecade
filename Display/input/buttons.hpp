#ifndef UI_BUTTONS_HPP
#define UI_BUTTONS_HPP

#include <cstdint>

namespace ui {

// Logical buttons as a bitmask so several can be held at once.
enum Button : std::uint32_t {
    BUTTON_NONE   = 0u,
    BUTTON_LEFT   = 1u << 0,
    BUTTON_RIGHT  = 1u << 1,
    BUTTON_FIRE   = 1u << 2,
    BUTTON_SELECT = 1u << 3
};

/* Debounces raw button samples fed once per frame. A button counts as pressed
   only after its raw level holds steady for the debounce window, which rejects
   contact chatter. Pure logic: main feeds the GPIO reading, tests feed synthetic
   samples, so no hardware is needed to test it. */
class Buttons {
public:
    // Feeds one raw sample (bitmask of buttons currently down), advanced by dt_ms.
    void poll(std::uint32_t dt_ms, std::uint32_t raw_mask)
    {
        edges_ = 0u;
        if (raw_mask != candidate_) {
            candidate_ = raw_mask;   // level changed: restart the settle timer
            settle_ms_ = 0u;
            return;
        }
        if (stable_ == candidate_) {
            return;                  // already settled at this value
        }
        settle_ms_ += dt_ms;
        if (settle_ms_ >= kDebounceMs) {
            edges_ = candidate_ & ~stable_;   // buttons newly down on this settle
            stable_ = candidate_;
        }
    }

    // Buttons currently held (debounced).
    std::uint32_t held() const { return stable_; }

    // Buttons that went down on the most recent settle (rising edges).
    std::uint32_t just_pressed() const { return edges_; }

    // True while the given button is held.
    bool is_held(Button b) const { return (stable_ & b) != 0u; }

    // True on the frame the given button first goes down.
    bool was_pressed(Button b) const { return (edges_ & b) != 0u; }

private:
    static constexpr std::uint32_t kDebounceMs = 20u;
    std::uint32_t stable_ = 0u;
    std::uint32_t candidate_ = 0u;
    std::uint32_t settle_ms_ = 0u;
    std::uint32_t edges_ = 0u;
};

}  // namespace ui

#endif  // UI_BUTTONS_HPP
