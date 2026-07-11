# Water Sort Puzzle for Adafruit EdgeBadge

A color sorting puzzle game written for the **Adafruit EdgeBadge** using the Arduino framework and Adafruit Arcada libraries.

Arrange the colored liquids in the tubes until each tube contains only a single color. Select a tube, choose a destination, and pour matching colors together until the puzzle is solved.

## Features

- Classic Water Sort puzzle gameplay
- Designed for the Adafruit EdgeBadge display and buttons
- Randomized puzzle generation
- Multiple colored liquids
- Animated tube selection
- Move counter
- Win detection
- Lightweight implementation suitable for embedded hardware

## Hardware

This project was developed for:

- Adafruit EdgeBadge
- 1.54" color TFT display
- Built-in buttons
- Built-in NeoPixels

## Libraries

This project uses:

- Adafruit Arcada
- Adafruit GFX

Install these libraries through the Arduino Library Manager before compiling.

## Controls

| Button | Action |
|---|---|
| Left / Right | Move cursor between tubes |
| A | Select tube / Pour liquid |
| B | Cancel selection |

Gameplay:

1. Move the cursor to a tube.
2. Press **A** to select it.
3. Move to another tube.
4. Press **A** to pour.
5. Sort all colors into matching tubes.

## Gameplay Notes

A selected tube will gently move to indicate it is the source tube.

The currently focused tube is marked with a white outline.

Only valid pours are accepted:
- Liquids can only be poured into empty tubes or matching colors.
- Tubes can only hold four layers.
