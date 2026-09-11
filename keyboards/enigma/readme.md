# enigma

A 2x3 macropad built around a Raspberry Pi Pico, with a 128x64 I2C OLED
that solves the usual macropad problem: forgetting what each key does.
Hold the info key and press any other key to see its name on screen,
full-screen and large; let go to go back to the idle overview. Up to 4
macro layers are supported, switched by holding two keys together, with
the current layer always shown on-screen.

* Keyboard Maintainer: [Chus Jara Peinado](https://github.com/chusjp)
* Hardware Supported: Raspberry Pi Pico (RP2040), SSD1306 128x64 I2C OLED
* Hardware Availability: custom build — see [hardware/](hardware/) for
  the KiCad schematic/PCB and [keymaps/default/](keymaps/default/) for
  wiring notes

## Layout

```
┌───┬───┬───┐
│ A │ B │ C │
├───┼───┼───┤
│ D │ E │ F │  <- F: tap = KC_F, hold = show info for the key you press next
└───┴───┴───┘
```

A-E each run a configurable macro. Hold F and press any of A-E to see
its name on the OLED instead of running it; release to return to the
idle screen. Hold A+B together (F not required) to cycle to the next
macro layer — see [keymaps/default/README.md](keymaps/default/README.md)
for how to edit macros, names, and layers via `macros.json`.

## Hardware

Designed around a socketed Pi Pico rather than a bare RP2040, so it can
be reflashed/replaced without desoldering. See
[hardware/README.md](hardware/README.md) for the KiCad project (verified
with `kicad-cli`, ERC/DRC clean) and what's still left to do (routing).

Make example for this keyboard (after setting up your build environment):

    make enigma:default

Flashing example for this keyboard:

    make enigma:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

No key currently sends `QK_BOOT`, so the reliable way to reflash is the
Pico module's own hardware BOOTSEL button:

* **BOOTSEL button**: hold the button on the Pico itself while plugging
  in USB, then flash the `.uf2` normally (`qmk flash`, or drag-and-drop
  onto the `RPI-RP2` drive it exposes)
* **Bootmagic reset**: hold down key A (matrix position 0,0, top-left)
  while plugging in the keyboard
