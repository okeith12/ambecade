#ifndef GFX_CANVAS_HPP
#define GFX_CANVAS_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"

namespace gfx {

/* Drawing surface in logical coordinates. Primitives clip silently at the
   canvas bounds. Only width/height/draw_pixel are required; the rest have
   default implementations built on them, which a concrete canvas MAY override
   for efficiency (e.g. a framebuffer's fast rectangular fill). */
class Canvas {
public:
    virtual ~Canvas() = default;

    // --- Required core: every canvas provides these. ---

    // size in pixels.
    virtual std::int16_t width() const = 0;
    virtual std::int16_t height() const = 0;

    // Sets a single pixel; a coordinate outside the surface is ignored.
    virtual void draw_pixel(std::int16_t x, std::int16_t y, color_t c) = 0;

    // --- Derived: default implementations in terms of the core. ---

    // Fills a solid rectangle, clipped to the surface.
    virtual void fill_rect(std::int16_t x, std::int16_t y,
                           std::int16_t w, std::int16_t h, color_t c)
    {
        if (w <= 0 || h <= 0) {
            return;
        }
        for (std::int16_t row = 0; row < h; ++row) {
            for (std::int16_t col = 0; col < w; ++col) {
                draw_pixel(static_cast<std::int16_t>(x + col),
                           static_cast<std::int16_t>(y + row), c);
            }
        }
    }

    // Fills the whole surface with one color.
    virtual void clear(color_t c)
    {
        fill_rect(0, 0, width(), height(), c);
    }

    // Horizontal / vertical run of pixels (1-thick fills).
    virtual void draw_hline(std::int16_t x, std::int16_t y,
                            std::int16_t w, color_t c)
    {
        fill_rect(x, y, w, 1, c);
    }
    virtual void draw_vline(std::int16_t x, std::int16_t y,
                            std::int16_t h, color_t c)
    {
        fill_rect(x, y, 1, h, c);
    }

    // One-pixel rectangle outline (two hlines + two vlines).
    virtual void draw_rect(std::int16_t x, std::int16_t y,
                           std::int16_t w, std::int16_t h, color_t c)
    {
        if (w <= 0 || h <= 0) {
            return;
        }
        draw_hline(x, y, w, c);
        draw_hline(x, static_cast<std::int16_t>(y + h - 1), w, c);
        draw_vline(x, y, h, c);
        draw_vline(static_cast<std::int16_t>(x + w - 1), y, h, c);
    }

    /* Copies a w-by-h block of RGB565 pixels (row-major) to (x, y),
       clipping at canvas bounds. src must hold w*h pixels. */
    virtual void blit(const color_t* src, std::int16_t x, std::int16_t y,
                      std::int16_t w, std::int16_t h)
    {
        if (src == nullptr || w <= 0 || h <= 0) {
            return;
        }
        for (std::int16_t row = 0; row < h; ++row) {
            for (std::int16_t col = 0; col < w; ++col) {
                std::size_t i = static_cast<std::size_t>(row) *
                                    static_cast<std::size_t>(w) +
                                static_cast<std::size_t>(col);
                draw_pixel(static_cast<std::int16_t>(x + col),
                           static_cast<std::int16_t>(y + row), src[i]);
            }
        }
    }
};

}  // namespace gfx

#endif  // GFX_CANVAS_HPP
