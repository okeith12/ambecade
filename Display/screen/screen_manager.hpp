#ifndef UI_SCREEN_MANAGER_HPP
#define UI_SCREEN_MANAGER_HPP

#include <cstddef>
#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"

namespace ui {

// Result of a ScreenManager operation (closed set, per project convention).
enum class screen_status {
    ok,             // succeeded
    err_range,      // id is outside the registry capacity
    err_null,       // a null Screen* was supplied
    err_empty,      // set_active on a slot with no screen registered
    err_no_active   // update/render with no active screen
};

/* Fixed-capacity registry of Screen* indexed by a scoped-enum id. Capacity is
   derived from the id enum's `count` sentinel, so a slot can never be out of
   range by construction (no drifting capacity template argument). Heap-free; the
   caller owns the Screen objects. Delegates update/render to the active screen.
   Out-of-range ids never crash -- they return a screen_status. */
template <typename Id>
class ScreenManager {
    // Every app id enum ends in a `count` sentinel that names the slot count.
    static constexpr std::size_t Capacity = static_cast<std::size_t>(Id::count);
    static_assert(Capacity > 0, "id enum must define a positive count sentinel");

public:
    // Registers (or replaces) the screen at slot id.
    screen_status set_screen(Id id, Screen* screen)
    {
        std::size_t i = index(id);
        if (i >= Capacity) {
            return screen_status::err_range;
        }
        if (screen == nullptr) {
            return screen_status::err_null;
        }
        screens_[i] = screen;
        return screen_status::ok;
    }

    // Selects the active screen; the slot must hold a registered screen.
    screen_status set_active(Id id)
    {
        std::size_t i = index(id);
        if (i >= Capacity) {
            return screen_status::err_range;
        }
        if (screens_[i] == nullptr) {
            return screen_status::err_empty;
        }
        active_ = i;
        has_active_ = true;
        return screen_status::ok;
    }

    // Advances the active screen by dt_ms.
    screen_status update(std::uint32_t dt_ms)
    {
        if (!has_active_) {
            return screen_status::err_no_active;
        }
        screens_[active_]->update(dt_ms);
        return screen_status::ok;
    }

    // Renders the active screen to the canvas.
    screen_status render(gfx::Canvas& canvas)
    {
        if (!has_active_) {
            return screen_status::err_no_active;
        }
        screens_[active_]->render(canvas);
        return screen_status::ok;
    }

    // True once a screen has been made active.
    bool has_active() const { return has_active_; }

private:
    static std::size_t index(Id id) { return static_cast<std::size_t>(id); }

    Screen* screens_[Capacity] = {};
    std::size_t active_ = 0u;
    bool has_active_ = false;
};

}  // namespace ui

#endif  // UI_SCREEN_MANAGER_HPP
