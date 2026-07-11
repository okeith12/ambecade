#include <unity.h>
#include <string.h>
#include "st7789.h"
#include "mock_spi.h"

static mock_spi_t g_mock;
static spi_bus_t g_bus;
static st7789_driver_t g_drv;

void setUp(void)
{
    mock_spi_init(&g_mock);
    mock_spi_bind(&g_bus, &g_mock);
}

void tearDown(void)
{
}

static void bind_incomplete_ops(spi_bus_t *bus)
{
    static spi_bus_ops_t partial;
    static uint8_t ctx;
    memset(&partial, 0, sizeof(partial));
    partial.transfer_byte = (spi_bus_status_t (*)(void *, uint8_t))0x1;
    bus->ops = &partial;
    bus->ctx = &ctx;
}

static void assert_event(size_t idx, mock_evt_type_t type, uint32_t value)
{
    TEST_ASSERT_TRUE(idx < g_mock.count);
    TEST_ASSERT_EQUAL_UINT(type, g_mock.events[idx].type);
    TEST_ASSERT_EQUAL_UINT32(value, g_mock.events[idx].value);
}

static void test_init_rejects_null_driver(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_NULL,
        st7789_init(NULL, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON));
}

static void test_init_rejects_null_bus(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_NULL,
        st7789_init(&g_drv, NULL, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON));
}

static void test_init_rejects_incomplete_ops(void)
{
    spi_bus_t bad;
    bind_incomplete_ops(&bad);
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_NULL,
        st7789_init(&g_drv, &bad, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON));
}

static void test_init_rejects_zero_dimensions(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_PARAM,
        st7789_init(&g_drv, &g_bus, 0u, 240u, 0u, 0u, ST7789_INVERSION_ON));
}

static void test_validate_accepts_complete_ops(void)
{
    TEST_ASSERT_EQUAL_INT(SPI_BUS_OK, spi_bus_validate(&g_bus));
}

static void test_init_emits_datasheet_command_sequence(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON));

    const uint8_t expected[] = {
        ST7789_CMD_SWRESET,
        ST7789_CMD_SLPOUT,
        ST7789_CMD_COLMOD, ST7789_COLMOD_16BIT,
        ST7789_CMD_MADCTL, ST7789_MADCTL_DEFAULT,
        ST7789_CMD_INVON,
        ST7789_CMD_NORON,
        ST7789_CMD_DISPON
    };

    uint8_t got[32];
    size_t n = mock_spi_collect_bytes(&g_mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
    TEST_ASSERT_FALSE(g_mock.overflow);
}

static void test_init_inversion_off_emits_invoff(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_OFF));

    const uint8_t expected[] = {
        ST7789_CMD_SWRESET,
        ST7789_CMD_SLPOUT,
        ST7789_CMD_COLMOD, ST7789_COLMOD_16BIT,
        ST7789_CMD_MADCTL, ST7789_MADCTL_DEFAULT,
        ST7789_CMD_INVOFF,
        ST7789_CMD_NORON,
        ST7789_CMD_DISPON
    };

    uint8_t got[32];
    size_t n = mock_spi_collect_bytes(&g_mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
    TEST_ASSERT_FALSE(g_mock.overflow);
}

static void test_init_performs_hardware_reset_first(void)
{
    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON));

    assert_event(0u, MOCK_EVT_RESET, GPIO_LEVEL_HIGH);
    assert_event(1u, MOCK_EVT_DELAY, 10u);
    assert_event(2u, MOCK_EVT_RESET, GPIO_LEVEL_LOW);
    assert_event(3u, MOCK_EVT_DELAY, 10u);
    assert_event(4u, MOCK_EVT_RESET, GPIO_LEVEL_HIGH);
    assert_event(5u, MOCK_EVT_DELAY, 120u);
}

static void test_write_command_frames_cs_and_dc(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    mock_spi_init(&g_mock);

    TEST_ASSERT_EQUAL_INT(ST7789_OK, st7789_write_command(&g_drv, 0xABu));

    assert_event(0u, MOCK_EVT_CS, GPIO_LEVEL_LOW);
    assert_event(1u, MOCK_EVT_DC, GPIO_LEVEL_LOW);
    assert_event(2u, MOCK_EVT_BYTE, 0xABu);
    assert_event(3u, MOCK_EVT_CS, GPIO_LEVEL_HIGH);
    TEST_ASSERT_EQUAL_UINT(4u, g_mock.count);
}

static void test_write_command_with_data_frames_correctly(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    mock_spi_init(&g_mock);

    const uint8_t data[] = { 0x11u, 0x22u, 0x33u };
    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_write_command_with_data(&g_drv, 0x2Au, data, 3u));

    assert_event(0u, MOCK_EVT_CS, GPIO_LEVEL_LOW);
    assert_event(1u, MOCK_EVT_DC, GPIO_LEVEL_LOW);
    assert_event(2u, MOCK_EVT_BYTE, 0x2Au);
    assert_event(3u, MOCK_EVT_DC, GPIO_LEVEL_HIGH);
    assert_event(4u, MOCK_EVT_BYTE, 0x11u);
    assert_event(5u, MOCK_EVT_BYTE, 0x22u);
    assert_event(6u, MOCK_EVT_BYTE, 0x33u);
    assert_event(7u, MOCK_EVT_CS, GPIO_LEVEL_HIGH);
}

static void test_addr_window_emits_caset_raset(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    mock_spi_init(&g_mock);

    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_set_addr_window(&g_drv, 0u, 0u, 240u, 240u));

    const uint8_t expected[] = {
        ST7789_CMD_CASET, 0x00u, 0x00u, 0x00u, 0xEFu,
        ST7789_CMD_RASET, 0x00u, 0x00u, 0x00u, 0xEFu
    };
    uint8_t got[16];
    size_t n = mock_spi_collect_bytes(&g_mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
}

static void test_addr_window_applies_offsets(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 80u, ST7789_INVERSION_ON);
    mock_spi_init(&g_mock);

    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_set_addr_window(&g_drv, 0u, 0u, 240u, 240u));

    const uint8_t expected[] = {
        ST7789_CMD_CASET, 0x00u, 0x00u, 0x00u, 0xEFu,
        ST7789_CMD_RASET, 0x00u, 0x50u, 0x01u, 0x3Fu
    };
    uint8_t got[16];
    size_t n = mock_spi_collect_bytes(&g_mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
}

static void test_addr_window_rejects_zero_size(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_PARAM,
        st7789_set_addr_window(&g_drv, 0u, 0u, 0u, 10u));
}

static void test_write_pixels_emits_ramwr_then_big_endian(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    mock_spi_init(&g_mock);

    const uint16_t pixels[] = { 0xF800u, 0x07E0u };
    TEST_ASSERT_EQUAL_INT(ST7789_OK,
        st7789_write_pixels(&g_drv, pixels, 2u));

    const uint8_t expected[] = {
        ST7789_CMD_RAMWR,
        0xF8u, 0x00u,
        0x07u, 0xE0u
    };
    uint8_t got[16];
    size_t n = mock_spi_collect_bytes(&g_mock, got, sizeof(got));

    TEST_ASSERT_EQUAL_UINT(sizeof(expected), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, got, sizeof(expected));
    TEST_ASSERT_FALSE(g_mock.overflow);
}

static void test_bus_error_propagates(void)
{
    g_drv.bus = &g_bus;
    g_drv.width = 240u;
    g_drv.height = 240u;
    g_drv.col_offset = 0u;
    g_drv.row_offset = 0u;

    g_mock.fail_on_call = 1u;
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_BUS, st7789_reset(&g_drv));
}

static void test_addr_window_rejects_out_of_bounds(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_PARAM,
        st7789_set_addr_window(&g_drv, 1u, 0u, 240u, 240u));
    TEST_ASSERT_EQUAL_INT(ST7789_ERR_PARAM,
        st7789_set_addr_window(&g_drv, 0u, 1u, 240u, 240u));
}

static void test_sleep_wake_emit_correct_commands(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 240u, 0u, 0u, ST7789_INVERSION_ON);

    mock_spi_init(&g_mock);
    TEST_ASSERT_EQUAL_INT(ST7789_OK, st7789_sleep(&g_drv));
    assert_event(2u, MOCK_EVT_BYTE, ST7789_CMD_SLPIN);

    mock_spi_init(&g_mock);
    TEST_ASSERT_EQUAL_INT(ST7789_OK, st7789_wake(&g_drv));
    assert_event(2u, MOCK_EVT_BYTE, ST7789_CMD_SLPOUT);

    mock_spi_init(&g_mock);
    TEST_ASSERT_EQUAL_INT(ST7789_OK, st7789_display_off(&g_drv));
    assert_event(2u, MOCK_EVT_BYTE, ST7789_CMD_DISPOFF);

    mock_spi_init(&g_mock);
    TEST_ASSERT_EQUAL_INT(ST7789_OK, st7789_display_on(&g_drv));
    assert_event(2u, MOCK_EVT_BYTE, ST7789_CMD_DISPON);
}

static void test_getters_report_dimensions(void)
{
    st7789_init(&g_drv, &g_bus, 240u, 320u, 0u, 0u, ST7789_INVERSION_ON);
    TEST_ASSERT_EQUAL_UINT16(240u, st7789_width(&g_drv));
    TEST_ASSERT_EQUAL_UINT16(320u, st7789_height(&g_drv));
    TEST_ASSERT_EQUAL_UINT16(0u, st7789_width(NULL));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_rejects_null_driver);
    RUN_TEST(test_init_rejects_null_bus);
    RUN_TEST(test_init_rejects_incomplete_ops);
    RUN_TEST(test_init_rejects_zero_dimensions);
    RUN_TEST(test_validate_accepts_complete_ops);
    RUN_TEST(test_init_emits_datasheet_command_sequence);
    RUN_TEST(test_init_inversion_off_emits_invoff);
    RUN_TEST(test_init_performs_hardware_reset_first);
    RUN_TEST(test_write_command_frames_cs_and_dc);
    RUN_TEST(test_write_command_with_data_frames_correctly);
    RUN_TEST(test_addr_window_emits_caset_raset);
    RUN_TEST(test_addr_window_applies_offsets);
    RUN_TEST(test_addr_window_rejects_zero_size);
    RUN_TEST(test_addr_window_rejects_out_of_bounds);
    RUN_TEST(test_write_pixels_emits_ramwr_then_big_endian);
    RUN_TEST(test_bus_error_propagates);
    RUN_TEST(test_sleep_wake_emit_correct_commands);
    RUN_TEST(test_getters_report_dimensions);
    return UNITY_END();
}
