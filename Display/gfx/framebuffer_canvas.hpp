#ifndef GFX_FRAMEBUFFER_CANVAS_HPP
#define GFX_FRAMEBUFFER_CANVAS_HPP

#include <cstddef>
#include <cstdint>
#include "canvas.hpp"

extern "C" {
#include "st7789.h"
}

namespace gfx {

/* Owns a fixed W-by-H RGB565 buffer sized at compile time (no dynamic
   allocation). For large dimensions the object is large; instantiate it
   statically or globally, never on the stack. */
template <std::int16_t W, std::int16_t H>
class FramebufferCanvas : public Canvas {
    static_assert(W > 0, "canvas width must be positive");
    static_assert(H > 0, "canvas height must be positive");

public:
    std::int16_t width() const override { return W; }
    std::int16_t height() const override { return H; }

    void clear(color_t c) override
    {
        for (std::size_t i = 0u; i < kPixelCount; ++i) {
            buffer_[i] = c;
        }
    }

    void draw_pixel(std::int16_t x, std::int16_t y, color_t c) override
    {
        if (x < 0 || y < 0 || x >= W || y >= H) {
            return;
        }
        buffer_[index(x, y)] = c;
    }

    /* Efficient override: clips the rectangle once in 32-bit math, then fills
       the clipped span directly. draw_hline/vline/rect and clear route through
       this; blit is inherited from Canvas (per-pixel via draw_pixel). */
    void fill_rect(std::int16_t x, std::int16_t y,
                   std::int16_t w, std::int16_t h, color_t c) override
    {
        if (w <= 0 || h <= 0) {
            return;
        }
        std::int32_t x0 = (x < 0) ? 0 : x;
        std::int32_t y0 = (y < 0) ? 0 : y;
        std::int32_t x1 = static_cast<std::int32_t>(x) + w;
        std::int32_t y1 = static_cast<std::int32_t>(y) + h;
        if (x1 > W) { x1 = W; }
        if (y1 > H) { y1 = H; }

        for (std::int32_t yy = y0; yy < y1; ++yy) {
            for (std::int32_t xx = x0; xx < x1; ++xx) {
                buffer_[index(static_cast<std::int16_t>(xx),
                              static_cast<std::int16_t>(yy))] = c;
            }
        }
    }

    // Bounds-checked read accessor (mainly for tests); returns 0 if off-surface.
    color_t pixel_at(std::int16_t x, std::int16_t y) const
    {
        if (x < 0 || y < 0 || x >= W || y >= H) {
            return 0u;
        }
        return buffer_[index(x, y)];
    }

    // Raw read-only access to the backing buffer (row-major, W*H pixels).
    const color_t* data() const { return buffer_; }

    /* Pushes the buffer to the panel, scaling up by an integer factor when the
       panel is larger than the logical canvas (e.g. a 120x120 canvas fills a
       240x240 panel at 2x). The panel must be an exact, uniform integer multiple
       of W x H or this returns ST7789_ERR_PARAM. Returns the driver status. */
    st7789_status_t flush(st7789_driver_t* driver) const
    {
        if (driver == nullptr) {
            return ST7789_ERR_NULL;
        }
        const std::uint16_t pw = st7789_width(driver);
        const std::uint16_t ph = st7789_height(driver);
        const std::uint16_t lw = static_cast<std::uint16_t>(W);
        const std::uint16_t lh = static_cast<std::uint16_t>(H);
        if (pw == 0u || ph == 0u || pw % lw != 0u || ph % lh != 0u) {
            return ST7789_ERR_PARAM;
        }
        const std::uint16_t scale = static_cast<std::uint16_t>(pw / lw);
        if (scale == 0u || scale != ph / lh) {   // scale must be uniform on both axes
            return ST7789_ERR_PARAM;
        }

        if (scale == 1u) {
            st7789_status_t st = st7789_set_addr_window(driver, 0u, 0u, lw, lh);
            if (st != ST7789_OK) {
                return st;
            }
            return st7789_write_pixels(driver, buffer_, kPixelCount);
        }

        // Scaled: expand one logical row into a physical-width row (each pixel
        // repeated `scale` times) and push it `scale` times, one row window at a
        // time. Streaming per row keeps memory to a single small row buffer.
        if (pw > kMaxRowPixels) {
            return ST7789_ERR_PARAM;
        }
        color_t row[kMaxRowPixels];
        for (std::uint16_t ly = 0u; ly < lh; ++ly) {
            std::uint16_t px = 0u;
            for (std::uint16_t lx = 0u; lx < lw; ++lx) {
                const color_t c = buffer_[static_cast<std::size_t>(ly) * lw + lx];
                for (std::uint16_t s = 0u; s < scale; ++s) {
                    row[px++] = c;
                }
            }
            for (std::uint16_t s = 0u; s < scale; ++s) {
                const std::uint16_t py = static_cast<std::uint16_t>(ly * scale + s);
                st7789_status_t st = st7789_set_addr_window(driver, 0u, py, pw, 1u);
                if (st != ST7789_OK) {
                    return st;
                }
                st = st7789_write_pixels(driver, row, pw);
                if (st != ST7789_OK) {
                    return st;
                }
            }
        }
        return ST7789_OK;
    }

private:
    // Largest physical row the scaled flush path can stage (covers 240/320 panels).
    static constexpr std::size_t kMaxRowPixels = 320u;

    // Total pixels in the buffer.
    static constexpr std::size_t kPixelCount =
        static_cast<std::size_t>(W) * static_cast<std::size_t>(H);

    // Row-major flat index for (x, y).
    static constexpr std::size_t index(std::int16_t x, std::int16_t y)
    {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(W) +
               static_cast<std::size_t>(x);
    }

    color_t buffer_[kPixelCount] = {};
};

}  // namespace gfx

#endif  // GFX_FRAMEBUFFER_CANVAS_HPP
