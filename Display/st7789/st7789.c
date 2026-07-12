/* ST7789 driver implementation.

   Every panel interaction goes through the spi_bus_t ops table (the hardware
   seam), so this file contains no direct SPI/GPIO calls and can be exercised
   against a mock. Public API and contracts live in st7789.h; this file is the
   "how". */

#include "st7789.h"

/* Datasheet-derived delays (milliseconds). Values are generous relative to the
   ST7789V minimums to stay safe across panel batches. */
#define ST7789_RESET_ASSERT_MS   10u   // RST held in each state during hardware reset
#define ST7789_RESET_RELEASE_MS  120u  // Settle after releasing RST before commands
#define ST7789_SWRESET_DELAY_MS  150u  // Wait after software reset (SWRESET)
#define ST7789_SLPOUT_DELAY_MS   120u  // Mandatory wait after leaving sleep (SLPOUT)
#define ST7789_COLMOD_DELAY_MS   10u   // Settle after setting pixel format
#define ST7789_DISPON_DELAY_MS   100u  // Settle after turning the display on

// Pixels serialized per transfer_buffer call; bounds the on-stack chunk buffer.
#define ST7789_PIXELS_PER_CHUNK  32u

// Collapses any non-OK bus status into the driver's single transport-error code.
static st7789_status_t map_bus_status(spi_bus_status_t s)
{
    return (s == SPI_BUS_OK) ? ST7789_OK : ST7789_ERR_BUS;
}

/* Frames one 4-wire SPI transaction: CS low, command byte with D/C low, then
   optional parameter bytes with D/C high, then CS high. The D/C line is how the
   panel distinguishes a command byte from its parameters. */
st7789_status_t st7789_write_command_with_data(st7789_driver_t *drvr,
                                               st7789_command_t command,
                                               const uint8_t *data,
                                               size_t len)
{
    if (drvr == NULL || drvr->bus == NULL) {
        return ST7789_ERR_NULL;
    }
    if (data == NULL && len > 0u) {
        return ST7789_ERR_NULL;
    }

    const spi_bus_ops_t *ops = drvr->bus->ops;
    void *ctx = drvr->bus->ctx;
    spi_bus_status_t s;

    s = ops->set_cs(ctx, GPIO_LEVEL_LOW);           // Select the panel
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->set_dc(ctx, GPIO_LEVEL_LOW);           // Next byte is a command
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->transfer_byte(ctx, (uint8_t)command);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }

    if (len > 0u) {
        s = ops->set_dc(ctx, GPIO_LEVEL_HIGH);      // Following bytes are data
        if (s != SPI_BUS_OK) { return map_bus_status(s); }
        s = ops->transfer_buffer(ctx, data, len);
        if (s != SPI_BUS_OK) { return map_bus_status(s); }
    }

    s = ops->set_cs(ctx, GPIO_LEVEL_HIGH);          // Deselect the panel
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    return ST7789_OK;
}

// Convenience wrapper for a parameterless command.
st7789_status_t st7789_write_command(st7789_driver_t *drvr,
                                     st7789_command_t command)
{
    return st7789_write_command_with_data(drvr, command, NULL, 0u);
}

/* Hardware reset: RST is active-low, so the sequence is idle-high, pulse low,
   release high, each step separated by a datasheet-timed delay. */
st7789_status_t st7789_reset(st7789_driver_t *drvr)
{
    if (drvr == NULL || drvr->bus == NULL) {
        return ST7789_ERR_NULL;
    }

    const spi_bus_ops_t *ops = drvr->bus->ops;
    void *ctx = drvr->bus->ctx;
    spi_bus_status_t s;

    s = ops->set_reset(ctx, GPIO_LEVEL_HIGH);       // Ensure a clean idle state
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->delay_ms(ctx, ST7789_RESET_ASSERT_MS);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->set_reset(ctx, GPIO_LEVEL_LOW);        // Assert reset
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->delay_ms(ctx, ST7789_RESET_ASSERT_MS);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->set_reset(ctx, GPIO_LEVEL_HIGH);       // Release reset
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->delay_ms(ctx, ST7789_RESET_RELEASE_MS);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }

    return ST7789_OK;
}

/* Programs the CASET/RASET window. Bounds are checked in 32-bit math to avoid
   16-bit overflow, then panel offsets are applied and each edge is packed
   big-endian (high byte first) as the controller expects. */
st7789_status_t st7789_set_addr_window(st7789_driver_t *drvr,
                                       uint16_t x, uint16_t y,
                                       uint16_t w, uint16_t h)
{
    if (drvr == NULL) {
        return ST7789_ERR_NULL;
    }
    if (w == 0u || h == 0u) {
        return ST7789_ERR_PARAM;
    }
    if ((uint32_t)x + (uint32_t)w > (uint32_t)drvr->width) {
        return ST7789_ERR_PARAM;
    }
    if ((uint32_t)y + (uint32_t)h > (uint32_t)drvr->height) {
        return ST7789_ERR_PARAM;
    }

    // Translate visible-area coordinates into GRAM coordinates via the offsets.
    uint16_t x0 = (uint16_t)(x + drvr->col_offset);
    uint16_t x1 = (uint16_t)(x0 + w - 1u);
    uint16_t y0 = (uint16_t)(y + drvr->row_offset);
    uint16_t y1 = (uint16_t)(y0 + h - 1u);

    uint8_t col_args[4];
    col_args[0] = (uint8_t)(x0 >> 8);     // XS high byte
    col_args[1] = (uint8_t)(x0 & 0xFFu);  // XS low byte
    col_args[2] = (uint8_t)(x1 >> 8);     // XE high byte
    col_args[3] = (uint8_t)(x1 & 0xFFu);  // XE low byte

    uint8_t row_args[4];
    row_args[0] = (uint8_t)(y0 >> 8);     // YS high byte
    row_args[1] = (uint8_t)(y0 & 0xFFu);  // YS low byte
    row_args[2] = (uint8_t)(y1 >> 8);     // YE high byte
    row_args[3] = (uint8_t)(y1 & 0xFFu);  // YE low byte

    st7789_status_t st;
    st = st7789_write_command_with_data(drvr, ST7789_CMD_CASET, col_args, 4u);
    if (st != ST7789_OK) { return st; }
    st = st7789_write_command_with_data(drvr, ST7789_CMD_RASET, row_args, 4u);
    if (st != ST7789_OK) { return st; }

    return ST7789_OK;
}

// MADCTL orientation bits (Sitronix ST7789V datasheet section 9.1.28).
#define ST7789_MADCTL_MY 0x80u  // Row address order (top-to-bottom flip)
#define ST7789_MADCTL_MX 0x40u  // Column address order (left-to-right flip)
#define ST7789_MADCTL_MV 0x20u  // Row/column exchange (transpose the axes)

/* Sets the panel orientation by writing MADCTL. The 240x240 panel is square, so
   the visible width/height are unchanged; a non-square panel would also swap
   drvr->width and drvr->height on the 90/270 cases. */
st7789_status_t st7789_set_rotation(st7789_driver_t *drvr,
                                    st7789_rotation_t rotation)
{
    if (drvr == NULL) {
        return ST7789_ERR_NULL;
    }

    uint8_t madctl;
    switch (rotation) {
        case ST7789_ROTATION_0:   madctl = 0x00u; break;
        case ST7789_ROTATION_90:  madctl = (uint8_t)(ST7789_MADCTL_MV | ST7789_MADCTL_MX); break;
        case ST7789_ROTATION_180: madctl = (uint8_t)(ST7789_MADCTL_MX | ST7789_MADCTL_MY); break;
        case ST7789_ROTATION_270: madctl = (uint8_t)(ST7789_MADCTL_MV | ST7789_MADCTL_MY); break;
        default: return ST7789_ERR_PARAM;
    }

    return st7789_write_command_with_data(drvr, ST7789_CMD_MADCTL, &madctl, 1u);
}

/* Issues RAMWR, then streams pixels in fixed-size chunks. Each RGB565 value is
   split into bytes by hand (high byte first) so output is correct regardless of
   host endianness -- never memcpy a uint16_t array to the bus. */
st7789_status_t st7789_write_pixels(st7789_driver_t *drvr,
                                    const uint16_t *pixels,
                                    size_t count)
{
    if (drvr == NULL || drvr->bus == NULL || pixels == NULL) {
        return ST7789_ERR_NULL;
    }

    const spi_bus_ops_t *ops = drvr->bus->ops;
    void *ctx = drvr->bus->ctx;
    spi_bus_status_t s;

    s = ops->set_cs(ctx, GPIO_LEVEL_LOW);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->set_dc(ctx, GPIO_LEVEL_LOW);           // RAMWR is a command
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->transfer_byte(ctx, (uint8_t)ST7789_CMD_RAMWR);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    s = ops->set_dc(ctx, GPIO_LEVEL_HIGH);          // Pixel bytes are data
    if (s != SPI_BUS_OK) { return map_bus_status(s); }

    uint8_t chunk[ST7789_PIXELS_PER_CHUNK * 2u];
    size_t i = 0u;
    while (i < count) {
        size_t n = 0u;
        while (n < ST7789_PIXELS_PER_CHUNK && i < count) {
            chunk[n * 2u]      = (uint8_t)(pixels[i] >> 8);   // High byte first
            chunk[n * 2u + 1u] = (uint8_t)(pixels[i] & 0xFFu);
            n++;
            i++;
        }
        s = ops->transfer_buffer(ctx, chunk, n * 2u);
        if (s != SPI_BUS_OK) { return map_bus_status(s); }
    }

    s = ops->set_cs(ctx, GPIO_LEVEL_HIGH);
    if (s != SPI_BUS_OK) { return map_bus_status(s); }
    return ST7789_OK;
}

/* Validates arguments, stores geometry, then runs the datasheet power-on
   sequence: hardware reset -> SWRESET -> SLPOUT -> COLMOD(16bpp) -> MADCTL ->
   inversion (per panel) -> NORON -> DISPON, with the required settle delays. */
st7789_status_t st7789_init(st7789_driver_t *drvr,
                            const spi_bus_t *bus,
                            uint16_t width,
                            uint16_t height,
                            uint16_t col_offset,
                            uint16_t row_offset,
                            st7789_inversion_t inversion)
{
    if (drvr == NULL || bus == NULL) {
        return ST7789_ERR_NULL;
    }
    if (spi_bus_validate(bus) != SPI_BUS_OK) {  // Reject a partial ops table
        return ST7789_ERR_NULL;
    }
    if (width == 0u || height == 0u) {
        return ST7789_ERR_PARAM;
    }

    drvr->bus = bus;
    drvr->width = width;
    drvr->height = height;
    drvr->col_offset = col_offset;
    drvr->row_offset = row_offset;

    const spi_bus_ops_t *ops = drvr->bus->ops;
    void *ctx = drvr->bus->ctx;
    st7789_status_t st;

    st = st7789_reset(drvr);                     // Hardware reset (RST pin)
    if (st != ST7789_OK) { return st; }

    st = st7789_write_command(drvr, ST7789_CMD_SWRESET);  // Software reset
    if (st != ST7789_OK) { return st; }
    if (ops->delay_ms(ctx, ST7789_SWRESET_DELAY_MS) != SPI_BUS_OK) {
        return ST7789_ERR_BUS;
    }

    st = st7789_write_command(drvr, ST7789_CMD_SLPOUT);   // Leave sleep
    if (st != ST7789_OK) { return st; }
    if (ops->delay_ms(ctx, ST7789_SLPOUT_DELAY_MS) != SPI_BUS_OK) {
        return ST7789_ERR_BUS;
    }

    uint8_t colmod = (uint8_t)ST7789_COLMOD_16BIT;        // RGB565
    st = st7789_write_command_with_data(drvr, ST7789_CMD_COLMOD, &colmod, 1u);
    if (st != ST7789_OK) { return st; }
    if (ops->delay_ms(ctx, ST7789_COLMOD_DELAY_MS) != SPI_BUS_OK) {
        return ST7789_ERR_BUS;
    }

    uint8_t madctl = ST7789_MADCTL_DEFAULT;               // Default orientation
    st = st7789_write_command_with_data(drvr, ST7789_CMD_MADCTL, &madctl, 1u);
    if (st != ST7789_OK) { return st; }

    // Panel-dependent: Normally-Black panels need INVON, Normally-White INVOFF.
    st7789_command_t inv_cmd = (inversion == ST7789_INVERSION_ON)
                                   ? ST7789_CMD_INVON
                                   : ST7789_CMD_INVOFF;
    st = st7789_write_command(drvr, inv_cmd);
    if (st != ST7789_OK) { return st; }

    st = st7789_write_command(drvr, ST7789_CMD_NORON);    // Normal display mode
    if (st != ST7789_OK) { return st; }

    st = st7789_write_command(drvr, ST7789_CMD_DISPON);   // Display on
    if (st != ST7789_OK) { return st; }
    if (ops->delay_ms(ctx, ST7789_DISPON_DELAY_MS) != SPI_BUS_OK) {
        return ST7789_ERR_BUS;
    }

    return ST7789_OK;
}

// Enters sleep (SLPIN).
st7789_status_t st7789_sleep(st7789_driver_t *drvr)
{
    return st7789_write_command(drvr, ST7789_CMD_SLPIN);
}

// Leaves sleep (SLPOUT).
st7789_status_t st7789_wake(st7789_driver_t *drvr)
{
    return st7789_write_command(drvr, ST7789_CMD_SLPOUT);
}

// Turns the display output on (DISPON).
st7789_status_t st7789_display_on(st7789_driver_t *drvr)
{
    return st7789_write_command(drvr, ST7789_CMD_DISPON);
}

// Turns the display output off (DISPOFF).
st7789_status_t st7789_display_off(st7789_driver_t *drvr)
{
    return st7789_write_command(drvr, ST7789_CMD_DISPOFF);
}

// Null-safe geometry accessors.
uint16_t st7789_width(const st7789_driver_t *drvr)
{
    return (drvr == NULL) ? 0u : drvr->width;
}

uint16_t st7789_height(const st7789_driver_t *drvr)
{
    return (drvr == NULL) ? 0u : drvr->height;
}
