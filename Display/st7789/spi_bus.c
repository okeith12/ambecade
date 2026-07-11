/* spi_bus seam helpers.

   The bus contract itself is just data (the ops table in spi_bus.h); the only
   behavior that belongs here is validation. spi_bus_validate is a guard callers
   run once before first use so a partially wired ops table fails loudly at init
   instead of crashing on the first null function pointer. */

#include "spi_bus.h"

// Returns SPI_BUS_OK only when the bus, its ops table, and all six ops are set.
spi_bus_status_t spi_bus_validate(const spi_bus_t *bus)
{
    if (bus == NULL) {
        return SPI_BUS_ERR_NULL;
    }
    if (bus->ops == NULL) {
        return SPI_BUS_ERR_NULL;
    }
    if (bus->ops->transfer_byte == NULL ||
        bus->ops->transfer_buffer == NULL ||
        bus->ops->set_cs == NULL ||
        bus->ops->set_dc == NULL ||
        bus->ops->set_reset == NULL ||
        bus->ops->delay_ms == NULL) {
        return SPI_BUS_ERR_NULL;
    }
    return SPI_BUS_OK;
}
