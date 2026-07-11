#ifndef ST7789_COMMANDS_H
#define ST7789_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Command opcodes from ST7789V datasheet v1.3, section 9
   newhavendisplay.com/content/datasheets/ST7789V.pdf */


typedef enum st7789_command {
    ST7789_CMD_NOP     = 0x00, // No Operation
    ST7789_CMD_SWRESET = 0x01, // Software Reset
    ST7789_CMD_SLPIN   = 0x10, // Sleep In
    ST7789_CMD_SLPOUT  = 0x11, // Sleep Out
    ST7789_CMD_NORON   = 0x13, // Normal Display Mode On
    ST7789_CMD_INVOFF  = 0x20, // Display Inversion Off
    ST7789_CMD_INVON   = 0x21, // Display Inversion On
    ST7789_CMD_DISPOFF = 0x28, // Display Off
    ST7789_CMD_DISPON  = 0x29, // Display On
    ST7789_CMD_CASET   = 0x2A, // Column Address Set
    ST7789_CMD_RASET   = 0x2B, // Row Address Set
    ST7789_CMD_RAMWR   = 0x2C, // Memory Write
    ST7789_CMD_MADCTL  = 0x36, // Memory Data Access Control
    ST7789_CMD_COLMOD  = 0x3A  // Interface Pixel Format
} st7789_command_t;


typedef enum st7789_colmod {
    ST7789_COLMOD_12BIT = 0x53, // 12 bits/pixel
    ST7789_COLMOD_16BIT = 0x55, // 16 bits/pixel
    ST7789_COLMOD_18BIT = 0x66  // 18 bits/pixel
} st7789_colmod_t;

#define ST7789_MADCTL_DEFAULT 0x00u // MADCTL default: no mirror/rotate, RGB order, top-to-bottom, left-to-right

#ifdef __cplusplus
}
#endif

#endif /* ST7789_COMMANDS_H */
