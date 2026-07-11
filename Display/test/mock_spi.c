#include "mock_spi.h"

static spi_bus_status_t record(mock_spi_t *m, mock_evt_type_t type, uint32_t value)
{
    if (m == NULL) {
        return SPI_BUS_ERR_NULL;
    }
    m->call_count++;
    if (m->fail_on_call != 0u && m->call_count >= m->fail_on_call) {
        return SPI_BUS_ERR_TRANSFER;
    }
    if (m->count >= MOCK_SPI_MAX_EVENTS) {
        m->overflow = 1u;
        return SPI_BUS_ERR_TRANSFER;
    }
    m->events[m->count].type = type;
    m->events[m->count].value = value;
    m->count++;
    return SPI_BUS_OK;
}

static spi_bus_status_t mock_transfer_byte(void *ctx, uint8_t value)
{
    return record((mock_spi_t *)ctx, MOCK_EVT_BYTE, value);
}

static spi_bus_status_t mock_transfer_buffer(void *ctx, const uint8_t *data, size_t len)
{
    mock_spi_t *m = (mock_spi_t *)ctx;
    if (data == NULL) {
        return SPI_BUS_ERR_NULL;
    }
    for (size_t i = 0u; i < len; i++) {
        spi_bus_status_t s = record(m, MOCK_EVT_BYTE, data[i]);
        if (s != SPI_BUS_OK) {
            return s;
        }
    }
    return SPI_BUS_OK;
}

static spi_bus_status_t mock_set_cs(void *ctx, gpio_level_t level)
{
    return record((mock_spi_t *)ctx, MOCK_EVT_CS, (uint32_t)level);
}

static spi_bus_status_t mock_set_dc(void *ctx, gpio_level_t level)
{
    return record((mock_spi_t *)ctx, MOCK_EVT_DC, (uint32_t)level);
}

static spi_bus_status_t mock_set_reset(void *ctx, gpio_level_t level)
{
    return record((mock_spi_t *)ctx, MOCK_EVT_RESET, (uint32_t)level);
}

static spi_bus_status_t mock_delay_ms(void *ctx, uint32_t ms)
{
    return record((mock_spi_t *)ctx, MOCK_EVT_DELAY, ms);
}

/* Single shared, stateless ops table. All per-instance state lives in the
   mock_spi_t passed as ctx, so one const table serves every mock instance. */
static const spi_bus_ops_t mock_ops = {
    .transfer_byte = mock_transfer_byte,
    .transfer_buffer = mock_transfer_buffer,
    .set_cs = mock_set_cs,
    .set_dc = mock_set_dc,
    .set_reset = mock_set_reset,
    .delay_ms = mock_delay_ms
};

void mock_spi_init(mock_spi_t *m)
{
    if (m == NULL) {
        return;
    }
    m->count = 0u;
    m->overflow = 0u;
    m->fail_on_call = 0u;
    m->call_count = 0u;
}

void mock_spi_bind(spi_bus_t *bus, mock_spi_t *m)
{
    if (bus == NULL || m == NULL) {
        return;
    }
    bus->ops = &mock_ops;
    bus->ctx = m;
}

size_t mock_spi_collect_bytes(const mock_spi_t *m, uint8_t *out, size_t max)
{
    size_t n = 0u;
    if (m == NULL) {
        return 0u;
    }
    for (size_t i = 0u; i < m->count; i++) {
        if (m->events[i].type == MOCK_EVT_BYTE) {
            if (out != NULL && n < max) {
                out[n] = (uint8_t)m->events[i].value;
            }
            n++;
        }
    }
    return n;
}
