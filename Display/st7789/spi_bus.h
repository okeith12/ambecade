#ifndef SPI_BUS_H
#define SPI_BUS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Result of a bus operation; SPI_BUS_OK is success, the rest are failures.
typedef enum spi_bus_status {
    SPI_BUS_OK = 0,            // Operation succeeded
    SPI_BUS_ERR_NULL,          // A required pointer (bus or ops entry) was NULL
    SPI_BUS_ERR_TRANSFER,      // The underlying SPI/GPIO transfer failed
    SPI_BUS_ERR_UNIMPLEMENTED  // Op is not wired up (e.g. a stub in a mock)
} spi_bus_status_t;

// Logical level for a control line (CS, D/C, RST).
typedef enum gpio_level {
    GPIO_LEVEL_LOW = 0,   // Line driven low
    GPIO_LEVEL_HIGH = 1   // Line driven high
} gpio_level_t;

// The hardware seam: one function pointer per primitive the driver needs.
// A real board fills these with SPI/GPIO calls; a mock fills them with recorders.
typedef struct spi_bus_ops {
    spi_bus_status_t (*transfer_byte)(void *ctx, uint8_t value);            // Shift one byte out over SPI
    spi_bus_status_t (*transfer_buffer)(void *ctx,
                                        const uint8_t *data, size_t len);   // Shift len bytes out over SPI
    spi_bus_status_t (*set_cs)(void *ctx, gpio_level_t level);              // Drive the chip-select line
    spi_bus_status_t (*set_dc)(void *ctx, gpio_level_t level);              // Drive data/command select (low=cmd, high=data)
    spi_bus_status_t (*set_reset)(void *ctx, gpio_level_t level);           // Drive the panel reset line
    spi_bus_status_t (*delay_ms)(void *ctx, uint32_t ms);                   // Block for ms milliseconds
} spi_bus_ops_t;

// A bus instance: the ops table plus an opaque context passed back to each op.
typedef struct spi_bus {
    const spi_bus_ops_t *ops;  // Borrowed function table (caller keeps it alive)
    void *ctx;                 // Implementation-defined handle passed to every op
} spi_bus_t;

/* Returns SPI_BUS_OK only if bus, ops, and every required function pointer
   are non-null. Call once before first use to guard against a partially
   initialized ops table. */
spi_bus_status_t spi_bus_validate(const spi_bus_t *bus);

#ifdef __cplusplus
}
#endif

#endif /* SPI_BUS_H */
