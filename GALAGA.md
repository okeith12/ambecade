# Playing Galaga on ambecade

Quick guide to wire the controls and run the Galaga screen.

## 1. Wire the buttons

Each button is a tactile switch from one pin to **GND**. The pins use `INPUT_PULLUP`, so an
unwired pin just reads as "not pressed" — you can add switches one at a time.

| Button | XIAO pin | Does |
| --- | --- | --- |
| Select | D0 | change screens (cycles shapes -> face -> frog -> Galaga) |
| Left | D1 | move the ship left |
| Right | D2 | move the ship right |
| Fire | D3 | shoot up / retry after game over |

Display stays as wired: SCK=D8, SDA/MOSI=D10, CS=D7, DC=D5, RST=D6, BLK=3V3.

(To change any pin, edit the `kPin*` / `kBtn*` constants at the top of `src/main.cpp`.)

## 2. Build and flash

```bash
cd ~/Code/Ambecade
pio run -e seeed_xiao_esp32s3 -t upload
```

Watch serial if you want:

```bash
pio device monitor -e seeed_xiao_esp32s3
```

## 3. Play

1. Tap **Select (D0)** until you reach the **Galaga** screen. It stays put (no auto-advance).
2. **Left/Right (D1/D2)** move, **Fire (D3)** shoots.
3. Clear the formation to win, or lose if it reaches you. **Fire** restarts.
4. Tap **Select** any time to leave the game and go back to cycling screens.

## Notes

- With only the one Select switch wired, you can still cycle through all screens; you just can't
  move/shoot until Left/Right/Fire are added.
- Enemies, ship, and bullets are drawn as simple colored rectangles for now; the score and
  "GAME OVER" use the built-in 5x7 font.
- The whole game is host-tested (`make -C Display test`), so logic changes are safe to verify on
  the laptop before flashing.
