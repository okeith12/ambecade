#ifndef UI_SCREEN_ID_HPP
#define UI_SCREEN_ID_HPP

#include <cstddef>

namespace ui {

// Closed set of screen slots for the app; values are contiguous from 0.
enum class screen_id : std::size_t {
    face = 0,
    count       // sentinel: number of screens (keep last)
};

// Registry capacity the ScreenManager needs to hold every app screen.
constexpr std::size_t screen_count = static_cast<std::size_t>(screen_id::count);

}  // namespace ui

#endif  // UI_SCREEN_ID_HPP
