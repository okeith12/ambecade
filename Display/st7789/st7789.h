#ifndef ST7789_H
#define ST7789_H

#include <stddef.h>
#include <stdint.h>
#include "spi_bus.h"
#include "st7789_commands.h"

#ifdef __cplusplus
extern "C" {
#endif

// driver call results
typedef enum st7789_status {
    ST7789_OK = 0,       // Operation succeeded
    ST7789_ERR_NULL,     // A required pointer argument was NULL
    ST7789_ERR_BUS,      // Underlying SPI/GPIO transport reported a failure
    ST7789_ERR_PARAM     // Argument out of range (zero size, window off-panel)
} st7789_status_t;

// Color inversion setting applied during init; panel-dependent, not a preference.
typedef enum st7789_inversion {
    ST7789_INVERSION_OFF = 0,  // Send INVOFF; for Normally-White panels
    ST7789_INVERSION_ON        // Send INVON; required by Normally-Black panels (e.g. 1.54" IPS)
} st7789_inversion_t;

// Driver handle: a borrowed bus plus panel geometry; members are read-only after st7789_init().
typedef struct st7789_driver {
    const spi_bus_t *bus;  // Borrowed transport seam (caller keeps it alive)
    uint16_t width;        // Visible panel width in pixels
    uint16_t height;       // Visible panel height in pixels
    uint16_t col_offset;   // X offset from GRAM origin to the visible area
    uint16_t row_offset;   // Y offset from GRAM origin to the visible area
} st7789_driver_t;

// Binds the bus, records geometry, then runs hardware reset and the datasheet
// power-on sequence. inversion selects INVON/INVOFF to match the panel type.
st7789_status_t st7789_init(st7789_driver_t *drvr,
                            const spi_bus_t *bus,
                            uint16_t width,
                            uint16_t height,
                            uint16_t col_offset,
                            uint16_t row_offset,
                            st7789_inversion_t inversion);

// Pulses the RST line with datasheet-timed delays to hardware-reset the panel.
st7789_status_t st7789_reset(st7789_driver_t *drvr);

// Sends a single command byte (D/C low, no parameters).
st7789_status_t st7789_write_command(st7789_driver_t *drvr,
                                     st7789_command_t command);

// Sends a command byte followed by len parameter bytes (D/C low then high).
st7789_status_t st7789_write_command_with_data(st7789_driver_t *drvr,
                                               st7789_command_t command,
                                               const uint8_t *data,
                                               size_t len);

// Sets the GRAM write window; w/h must be non-zero and x+w/y+h must stay on-panel or ST7789_ERR_PARAM.
st7789_status_t st7789_set_addr_window(st7789_driver_t *drvr,
                                       uint16_t x, uint16_t y,
                                       uint16_t w, uint16_t h);

// Streams count RGB565 pixels (not bytes) to GRAM after RAMWR, high byte first.
st7789_status_t st7789_write_pixels(st7789_driver_t *drvr,
                                    const uint16_t *pixels,
                                    size_t count);

// Enters sleep mode (SLPIN) to cut panel power draw.
st7789_status_t st7789_sleep(st7789_driver_t *drvr);

// Leaves sleep mode (SLPOUT) and resumes normal operation.
st7789_status_t st7789_wake(st7789_driver_t *drvr);

// Turns the display output on (DISPON).
st7789_status_t st7789_display_on(st7789_driver_t *drvr);

// Turns the display output off (DISPOFF) without sleeping.
st7789_status_t st7789_display_off(st7789_driver_t *drvr);

// Returns the configured panel width, or 0 if drvr is NULL.
uint16_t st7789_width(const st7789_driver_t *drvr);

// Returns the configured panel height, or 0 if drvr is NULL.
uint16_t st7789_height(const st7789_driver_t *drvr);

#ifdef __cplusplus
}
#endif

#endif /* ST7789_H */
