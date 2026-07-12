# Host unit tests: `make test` builds and runs every suite (fetches Unity once).

CC        := cc
CXX       := c++
UNITY_DIR := test/vendor/unity
UNITY_SRC := $(UNITY_DIR)/src

CFLAGS    := -std=c11 -Wall -Wextra -g -O0
CXXFLAGS  := -std=c++17 -Wall -Wextra -g -O0
INCLUDES  := -Ist7789 -Igfx -Iscreen -Istorage -Iinput -Itest -I$(UNITY_SRC)

BUILD_DIR := build

# C objects shared by the C test binaries (driver + mock + Unity).
COMMON_OBJS := \
	$(BUILD_DIR)/st7789.o \
	$(BUILD_DIR)/spi_bus.o \
	$(BUILD_DIR)/mock_spi.o \
	$(BUILD_DIR)/unity.o

DRIVER_BIN := $(BUILD_DIR)/test_st7789
CANVAS_BIN := $(BUILD_DIR)/test_canvas
SCREEN_BIN := $(BUILD_DIR)/test_screen
BITMAP_BIN := $(BUILD_DIR)/test_bitmap

.PHONY: test compile clean

test: compile
	@echo "=== driver suite ==="
	@./$(DRIVER_BIN)
	@echo "=== canvas suite ==="
	@./$(CANVAS_BIN)
	@echo "=== screen suite ==="
	@./$(SCREEN_BIN)
	@echo "=== bitmap suite ==="
	@./$(BITMAP_BIN)

compile: $(DRIVER_BIN) $(CANVAS_BIN) $(SCREEN_BIN) $(BITMAP_BIN)

# Driver suite is pure C.
$(DRIVER_BIN): $(COMMON_OBJS) $(BUILD_DIR)/test_st7789.o
	$(CC) $(CFLAGS) $^ -o $@

# Canvas suite links the C objects with the C++ test; link with the C++ driver.
$(CANVAS_BIN): $(COMMON_OBJS) $(BUILD_DIR)/test_canvas.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Screen suite is pure C++ (plus Unity); no driver/mock needed.
$(SCREEN_BIN): $(BUILD_DIR)/unity.o $(BUILD_DIR)/procedural_face.o $(BUILD_DIR)/shapes_screen.o \
               $(BUILD_DIR)/galaga.o $(BUILD_DIR)/test_screen.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Bitmap suite: C storage seam + mock, C++ loader/face/font/test.
$(BITMAP_BIN): $(BUILD_DIR)/unity.o $(BUILD_DIR)/storage.o $(BUILD_DIR)/mock_storage.o \
               $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/bitmap_face.o $(BUILD_DIR)/font.o \
               $(BUILD_DIR)/test_bitmap.o
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
$(BUILD_DIR)/procedural_face.o: screen/procedural_face.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/shapes_screen.o: screen/shapes_screen.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/galaga.o: screen/galaga.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/test_screen.o: test/test_screen.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/storage.o: storage/storage.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/mock_storage.o: test/mock_storage.c | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/bitmap.o: gfx/bitmap.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/font.o: gfx/font.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/bitmap_face.o: screen/bitmap_face.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/test_bitmap.o: test/test_bitmap.cpp | $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Vendor Unity locally the first time it's needed.
$(UNITY_SRC)/unity.c:
	@echo "Unity not found -- cloning ThrowTheSwitch/Unity into $(UNITY_DIR)..."
	@git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git $(UNITY_DIR)

clean:
	@rm -rf $(BUILD_DIR)
