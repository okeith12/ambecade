#include "bitmap.hpp"

namespace gfx {

storage_status_t load_bitmap(const storage_reader_t& reader, const char* path,
                             std::int16_t w, std::int16_t h,
                             color_t* buf, std::size_t buf_capacity,
                             Bitmap& out_bitmap)
{
    if (path == nullptr || buf == nullptr) {
        return STORAGE_ERR_NULL;
    }
    if (storage_reader_validate(&reader) != STORAGE_OK) {
        return STORAGE_ERR_NULL;
    }
    if (w <= 0 || h <= 0) {
        return STORAGE_ERR_RANGE;
    }

    const std::size_t pixels =
        static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    if (pixels > buf_capacity) {
        return STORAGE_ERR_RANGE;
    }

    // Read the raw bytes straight into buf's storage, then convert in place.
    std::uint8_t* bytes = reinterpret_cast<std::uint8_t*>(buf);
    const storage_status_t st =
        reader.ops->read(reader.ctx, path, bytes, pixels * 2u);
    if (st != STORAGE_OK) {
        return st;
    }

    /* Assemble big-endian byte pairs into native color_t. Ascending i is safe:
       pixel i reads only bytes[2i], bytes[2i+1] -- which are buf[i]'s own
       storage -- before overwriting buf[i], so no earlier write is clobbered. */
    for (std::size_t i = 0; i < pixels; ++i) {
        const std::uint8_t hi = bytes[2u * i];
        const std::uint8_t lo = bytes[2u * i + 1u];
        buf[i] = static_cast<color_t>(
            (static_cast<std::uint16_t>(hi) << 8) | lo);
    }

    out_bitmap.pixels = buf;
    out_bitmap.width = w;
    out_bitmap.height = h;
    return STORAGE_OK;
}

}  // namespace gfx
