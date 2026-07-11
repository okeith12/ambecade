# ambecade

[![CI](https://github.com/okeith12/ambecade/actions/workflows/ci.yml/badge.svg)](https://github.com/okeith12/ambecade/actions/workflows/ci.yml)

Custom Arcade

## Display driver

`Display/` holds the ST7789 display driver and graphics layer, tested entirely on
the host (no hardware) via a mock SPI bus.

```
make -C Display test
```

- `Display/st7789/` — C driver (SPI bus seam, command opcodes, init/window/pixels)
- `Display/gfx/` — C++ canvas layer (RGB565 color, `Canvas` interface, `FramebufferCanvas`)
- `Display/test/` — Unity host tests for both layers
