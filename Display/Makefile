# Host unit tests: `make test` builds and runs both suites (fetches Unity once).

CC        := cc
CXX       := c++
UNITY_DIR := test/vendor/unity
UNITY_SRC := $(UNITY_DIR)/src

CFLAGS    := -std=c11 -Wall -Wextra -g -O0
CXXFLAGS  := -std=c++17 -Wall -Wextra -g -O0
INCLUDES  := -Ist7789 -Igfx -Itest -I$(UNITY_SRC)

BUILD_DIR := build


COMMON_OBJS := \
	$(BUILD_DIR)/st7789.o \
	$(BUILD_DIR)/spi_bus.o \
	$(BUILD_DIR)/mock_spi.o \
	$(BUILD_DIR)/unity.o

DRIVER_BIN := $(BUILD_DIR)/test_st7789
CANVAS_BIN := $(BUILD_DIR)/test_canvas

.PHONY: test compile clean

test: compile
	@echo "=== driver suite ==="
	@./$(DRIVER_BIN)
	@echo "=== canvas suite ==="
	@./$(CANVAS_BIN)

compile: $(DRIVER_BIN) $(CANVAS_BIN)


$(DRIVER_BIN): $(COMMON_OBJS) $(BUILD_DIR)/test_st7789.o
	$(CC) $(CFLAGS) $^ -o $@

# Canvas suite links the C objects with the C++ test; use the C++ driver to link.
$(CANVAS_BIN): $(COMMON_OBJS) $(BUILD_DIR)/test_canvas.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/st7789.o: st7789/st7789.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/spi_bus.o: st7789/spi_bus.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/mock_spi.o: test/mock_spi.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/unity.o: $(UNITY_SRC)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/test_st7789.o: test/test_st7789.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/test_canvas.o: test/test_canvas.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Vendor Unity locally the first time it's needed.
$(UNITY_SRC)/unity.c:
	@echo "Unity not found -- cloning ThrowTheSwitch/Unity into $(UNITY_DIR)..."
	@git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git $(UNITY_DIR)

clean:
	@rm -rf $(BUILD_DIR)
