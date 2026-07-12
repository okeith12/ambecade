#ifndef UI_BITMAP_FACE_HPP
#define UI_BITMAP_FACE_HPP

#include <cstdint>
#include "canvas.hpp"
#include "screen.hpp"
#include "bitmap.hpp"

namespace ui {

/* A screen that displays a preloaded bitmap, centered on the canvas and blitted
   each render. The asset-based counterpart to ProceduralFace. Holds only a
   Bitmap view -- the pixel storage is owned by the caller. */
class BitmapFace : public Screen {
public:
    explicit BitmapFace(const gfx::Bitmap& bitmap) : bitmap_(bitmap) {}

    // Swaps in a different bitmap to display.
    void set_bitmap(const gfx::Bitmap& bitmap) { bitmap_ = bitmap; }

    void update(std::uint32_t dt_ms) override;   // static image: no-op
    void render(gfx::Canvas& canvas) override;

private:
    gfx::Bitmap bitmap_;
};

}  // namespace ui

#endif  // UI_BITMAP_FACE_HPP
