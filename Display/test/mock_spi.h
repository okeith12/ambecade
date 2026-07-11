#ifndef MOCK_SPI_H
#define MOCK_SPI_H

#include <stddef.h>
#include <stdint.h>
#include "spi_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

// Cap on recorded events; sized for init-sequence tests, not full-frame pixel writes.
#define MOCK_SPI_MAX_EVENTS 512u

// Which bus primitive produced a recorded event.
typedef enum mock_evt_type {
    MOCK_EVT_BYTE,   // transfer_byte, or one byte of transfer_buffer; value = the byte
    MOCK_EVT_CS,     // set_cs;    value = gpio_level_t
    MOCK_EVT_DC,     // set_dc;    value = gpio_level_t
    MOCK_EVT_RESET,  // set_reset; value = gpio_level_t
    MOCK_EVT_DELAY   // delay_ms;  value = milliseconds
} mock_evt_type_t;

// One recorded bus interaction, in call order.
typedef struct mock_evt {
    mock_evt_type_t type;  // Which primitive fired
    uint32_t value;        // Payload, interpreted per type
} mock_evt_t;

// Recording state for one mock bus instance; caller owns the storage (no heap).
typedef struct mock_spi {
    mock_evt_t events[MOCK_SPI_MAX_EVENTS];  // Events captured, in order
    size_t count;                            // Number of events recorded so far
    uint8_t overflow;                        // Set to 1 once events[] has filled up
    uint32_t fail_on_call;                   // If non-zero, fail this call number and every call after
    uint32_t call_count;                     // Total primitive calls seen (including failed/dropped)
} mock_spi_t;

// Resets recording state to empty; call before mock_spi_bind and at the start of each test.
void mock_spi_init(mock_spi_t *m);

// Points bus at the shared mock ops table with m as its context; call after mock_spi_init.
void mock_spi_bind(spi_bus_t *bus, mock_spi_t *m);

/* Copies only the MOCK_EVT_BYTE values, in order, into out (up to max). Returns
   the total number of byte events seen (may exceed max so the caller can detect
   truncation). out may be NULL only when max is 0. */
size_t mock_spi_collect_bytes(const mock_spi_t *m, uint8_t *out, size_t max);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_SPI_H */
