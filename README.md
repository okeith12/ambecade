# ambecade

[![CI](https://github.com/okeith12/ambecade/actions/workflows/ci.yml/badge.svg)](https://github.com/okeith12/ambecade/actions/workflows/ci.yml)

A custom arcade device: a Seeed **XIAO ESP32-S3** driving a 1.54" **ST7789** 240x240 display.

## Display

`Display/` is a hand-written ST7789 driver and a small graphics-and-screens stack. It is built
in layers with a hardware seam, so every layer is unit-tested on a laptop before it touches a board.

```
screens  ── face · frog · shapes · Galaga ── input · font · ScreenManager
gfx      ── Canvas · FramebufferCanvas · color · bitmap
driver   ── ST7789 commands · address window · pixels
seam     ── spi_bus ops table ──► real SPI  |  mock (tests)
```


| Path | What |
| --- | --- |
| `Display/st7789/` | C driver: SPI-bus seam, command opcodes, init, address window, pixels, rotation |
| `Display/gfx/` | C++ canvas: RGB565 color, `Canvas`, `FramebufferCanvas` (integer-scaling `flush`), `bitmap`, 5x7 `font` |
| `Display/screen/` | `Screen` + `ScreenManager`, the animated face, frog bitmap, shapes, and Galaga |
| `Display/input/` | debounced `Buttons` |
| `Display/test/` | Unity host tests for every layer |
| `src/main.cpp` | firmware: wires the seam to Arduino SPI and runs the screens |

### Screens

The device cycles **shapes → face → frog → Galaga → Firefly Catch → clock**. A **Select** button
switches screens; the games hold their screen so they are not interrupted mid-play.

- **Shapes** — a sliding square over a cycling background.
- **Face** — a procedural face that blinks (open a few seconds, brief shut).
- **Frog** — a self-describing pixel-art bitmap, scaled to fill the screen.
- **Galaga** — move **left/right**, **fire** up at a marching formation; score and game-over drawn
  with the bitmap font. Pure game logic, unit-tested; **fire** restarts after a win or loss.
- **Firefly Catch** — slide the jar to catch fireflies falling from random spots; miss and lose a
  life. Seeded RNG, unit-tested; **fire** restarts.
- **Clock** — a big HH:MM and date, set over WiFi/NTP. Add credentials via a gitignored secrets
  file (`cp src/secrets.example.h src/secrets.h`, then fill it in); without it the clock shows a
  placeholder.

### Test 

```
make -C Display test
```

### Flash

```
pio run -e seeed_xiao_esp32s3 -t upload
```

### Wiring (XIAO ESP32-S3)

| Display | Pin | | Button (to GND) | Pin |
| --- | --- | --- | --- | --- |
| SCK | D8 | | Select (change screen) | D0 |
| SDA / MOSI | D10 | | Left | D1 |
| CS | D7 | | Right | D2 |
| DC | D5 | | Fire / retry | D3 |
| RST | D6 | | | |
| BLK | 3V3 | | | |

Buttons use `INPUT_PULLUP`, so one Select switch is enough to
cycle the screens; add Left/Right/Fire to play Galaga.
