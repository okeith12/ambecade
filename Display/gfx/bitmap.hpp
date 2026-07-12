#ifndef GFX_BITMAP_HPP
#define GFX_BITMAP_HPP

#include <cstddef>
#include <cstdint>
#include "color.hpp"

extern "C" {
#include "storage.h"
}

namespace gfx {

// Non-owning view of an RGB565 image in memory (row-major, width*height pixels).
struct Bitmap {
    const color_t* pixels;
    std::int16_t width;
    std::int16_t height;
};

/* Loads a raw RGB565 image (no header: exactly w*h pixels, each stored
   big-endian / high byte first) through the storage reader into buf, then
   assembles the bytes into native color_t values in place. buf must hold at
   least w*h pixels. On success fills out_bitmap; on any failure returns the
   storage_status_t and leaves out_bitmap untouched. */
storage_status_t load_bitmap(const storage_reader_t& reader, const char* path,
                             std::int16_t w, std::int16_t h,
                             color_t* buf, std::size_t buf_capacity,
                             Bitmap& out_bitmap);

}  // namespace gfx

#endif  // GFX_BITMAP_HPP
