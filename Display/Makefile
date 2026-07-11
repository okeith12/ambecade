# Host-based unit tests for the ST7789 driver.
#
# Pure software: NO hardware required. These tests compile and run on your Mac.
# The MockSpiBus stands in for the real SPI bus and GPIO lines, recording every
# byte the driver emits so the tests can assert the datasheet sequence.
#
#   make test     build and run the test suite (fetches Unity on first run)
#   make compile  just compile
#   make clean    remove build artifacts

CC        := cc
UNITY_DIR := test/vendor/unity
UNITY_SRC := $(UNITY_DIR)/src

CFLAGS    := -std=c11 -Wall -Wextra -g -O0
INCLUDES  := -Ist7789 -Itest -I$(UNITY_SRC)

SRCS := \
	st7789/st7789.c \
	st7789/spi_bus.c \
	test/mock_spi.c \
	test/test_st7789.c \
	$(UNITY_SRC)/unity.c

BUILD_DIR := build
BIN       := $(BUILD_DIR)/test_st7789

.PHONY: test compile clean

test: compile
	@echo "=== running st7789 test suite (no hardware needed) ==="
	@./$(BIN)

compile: $(UNITY_SRC)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(BIN)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Vendor Unity locally the first time it's needed.
$(UNITY_SRC)/unity.c:
	@echo "Unity not found -- cloning ThrowTheSwitch/Unity into $(UNITY_DIR)..."
	@git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git $(UNITY_DIR)

clean:
	@rm -rf $(BUILD_DIR)
