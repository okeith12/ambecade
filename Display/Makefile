# Host unit tests: `make test` builds and runs every suite.
# Fetches Unity and the percyus_displays package once, into test/vendor/.

CC        := cc
CXX       := c++
UNITY_DIR := test/vendor/unity
UNITY_SRC := $(UNITY_DIR)/src
PKG_DIR   := test/vendor/percyus_displays
PKG_STAMP := $(PKG_DIR)/library.json

CFLAGS    := -std=c11 -Wall -Wextra -g -O0
CXXFLAGS  := -std=c++17 -Wall -Wextra -g -O0

# Package headers first, then this project's own screens and assets.
INCLUDES  := -I$(PKG_DIR)/st7789 -I$(PKG_DIR)/gfx -I$(PKG_DIR)/screen \
             -I$(PKG_DIR)/storage -I$(PKG_DIR)/input \
             -Iscreen -Igfx -Itest -I$(UNITY_SRC)

BUILD_DIR := build

SCREEN_BIN := $(BUILD_DIR)/test_screen
BITMAP_BIN := $(BUILD_DIR)/test_bitmap

.PHONY: test compile clean

test: compile
	@echo "=== screen suite ==="
	@./$(SCREEN_BIN)
	@echo "=== bitmap suite ==="
	@./$(BITMAP_BIN)

compile: $(SCREEN_BIN) $(BITMAP_BIN)

# Screen suite: this project's screens and games (pure C++ plus Unity).
$(SCREEN_BIN): $(BUILD_DIR)/unity.o $(BUILD_DIR)/procedural_face.o $(BUILD_DIR)/shapes_screen.o \
               $(BUILD_DIR)/galaga.o $(BUILD_DIR)/firefly_catch.o $(BUILD_DIR)/test_screen.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Bitmap suite: this project's assets and asset-backed screens, linked against
# the package's bitmap loader, font, and storage seam.
$(BITMAP_BIN): $(BUILD_DIR)/unity.o $(BUILD_DIR)/storage.o $(BUILD_DIR)/bitmap.o \
               $(BUILD_DIR)/font.o $(BUILD_DIR)/bitmap_face.o \
               $(BUILD_DIR)/anniversary_screen.o $(BUILD_DIR)/test_bitmap.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/unity.o: $(UNITY_SRC)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# --- package sources ---
$(BUILD_DIR)/storage.o: $(PKG_DIR)/storage/storage.c | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/bitmap.o: $(PKG_DIR)/gfx/bitmap.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/font.o: $(PKG_DIR)/gfx/font.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- this project's sources ---
$(BUILD_DIR)/procedural_face.o: screen/procedural_face.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/shapes_screen.o: screen/shapes_screen.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/galaga.o: screen/galaga.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/firefly_catch.o: screen/firefly_catch.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/bitmap_face.o: screen/bitmap_face.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/anniversary_screen.o: screen/anniversary_screen.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- test sources ---
$(BUILD_DIR)/test_screen.o: test/test_screen.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/test_bitmap.o: test/test_bitmap.cpp | $(PKG_STAMP) $(UNITY_SRC)/unity.c $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Vendor the shared display package locally the first time it's needed.
$(PKG_STAMP):
	@echo "percyus_displays not found -- cloning into $(PKG_DIR)..."
	@git clone --depth 1 https://github.com/okeith12/percyus_displays.git $(PKG_DIR)

# Vendor Unity locally the first time it's needed.
$(UNITY_SRC)/unity.c:
	@echo "Unity not found -- cloning ThrowTheSwitch/Unity into $(UNITY_DIR)..."
	@git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git $(UNITY_DIR)

clean:
	@rm -rf $(BUILD_DIR)
