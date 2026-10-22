# Embedded Space Invaders

A Space Invaders game running bare-metal on an ATmega328P (Arduino Uno R3), written in AVR C/C++ without the Arduino library. The microcontroller's timers, GPIO, ADC, and SPI hardware are controlled directly through registers.

**[▶ Watch the demo](https://youtu.be/eGb0SGQPO30)**

Built solo as the final project for CS120B (Introduction to Embedded Systems) at UC Riverside.

## Gameplay

- Press the joystick to start. Five aliens appear at the top of the screen and your ship at the bottom.
- Tilt the joystick left or right to move the ship. Holding it keeps the ship moving.
- Press the joystick to fire. Each press fires exactly one bullet, and at most three bullets can be on screen at once.
- The aliens sweep right, step down, sweep left, step down, and repeat.
- Destroy all five aliens to win. If any alien reaches your row, you lose. Press the joystick to return to the menu.
- A 16x2 character LCD shows the menu, "Playing...", and win/lose messages, while the color LCD shows the game.
- A melody loops on the buzzer while you're on the menu or end screens, and goes silent during play.

## Hardware

| Component | Purpose |
|---|---|
| Arduino Uno R3 (ATmega328P) | Microcontroller |
| ST7735 1.8" color LCD | Game display (SPI) |
| LCD1602 16x2 character LCD | Menu and status messages |
| Analog joystick module | Movement (ADC) and firing (push button) |
| Passive buzzer | Music (Timer1 PWM) |

### Wiring

| Component pin | Uno pin | AVR pin |
|---|---|---|
| ST7735 SCK | D13 | PB5 |
| ST7735 SDA / MOSI | D11 | PB3 |
| ST7735 CS | D10 | PB2 |
| ST7735 A0 / DC | A0 | PC0 |
| ST7735 RESET | A1 | PC1 |
| Joystick X axis | A2 | PC2 (ADC channel 2) |
| Joystick button | A3 | PC3 (internal pull-up, active low) |
| Buzzer | D9 | PB1 (OC1A) |
| LCD1602 | See `include/LCD.h` | Port D |

## Architecture

The game is built as eight **concurrent synchronous state machines (SynchSMs)**. A Timer2 interrupt fires every 50 ms (the GCD, or greatest common divisor, of all task periods), and the scheduler runs each task whose period has elapsed.

| Task | Period | Role |
|---|---|---|
| `JOYSTICK` | 50 ms | Reads the X axis through the ADC and sets left/right flags |
| `JOYSTICK_BUTTON` | 50 ms | Debounces the button and turns each press into a single pulse |
| `CONTROLLER` | 50 ms | Game mode (menu, playing, win, lose), bullet/alien collisions, win/lose checks |
| `SONG` | 50 ms | Steps through the melody, with a short gap between notes |
| `BULLETS` | 100 ms | Spawns, moves, and redraws up to three bullets |
| `SHIP` | 300 ms | Moves and redraws the ship |
| `ALIENS` | 750 ms | Moves the alien formation in its sweep-and-descend pattern |
| `LCD` | 500 ms | Updates the 16x2 status display |

Tasks communicate through shared state: the input tasks set flags, the game tasks read them and update positions, and the `CONTROLLER` task watches everything and switches the game's mode. Each task's period controls its speed; for example, the alien period sets the game's difficulty.

## Debugging Notes

Two separate display flicker problems came up during development.

### 1. SPI clock speed

The display flickered whenever sprites were redrawn. Rewiring RESET and adding a filter capacitor on A0 made no difference. The fix was the SPI clock speed:

| SPI clock | Result |
|---|---|
| 125 kHz | Reliable, but every redraw was visibly slow |
| 1 MHz | Reliable, flickered about half the time |
| **2 MHz** | **Reliable, no visible flicker** |
| 4 MHz | Too fast for the module: screen went solid white |

2 MHz (`SPR0` with the `SPI2X` double-speed bit) was fast enough that pixel writes finished before the redraw became visible, and slow enough for the module to receive data reliably.

### 2. PWM interference from the buzzer

After the first fix, flicker came back as soon as the buzzer was re-enabled. Timer1's PWM output on PB1 switches rapidly and sits right next to the SPI pins (PB2, PB3, PB5), and that switching was interfering with the SPI signals.

Instead of rewiring, the fix was in software: `setTone()` connects Timer1 to PB1 (sets `COM1A1`) only when a note plays, and `offTone()` disconnects it. The pin stays quiet whenever the buzzer is silent, including all of gameplay.

## Building

Requires [PlatformIO](https://platformio.org/) (the VS Code extension or the CLI).

```bash
git clone git@github.com:darreckcraiu/Embedded_Space_Invaders.git
cd Embedded_Space_Invaders
pio run -t upload
```

## Credits

Provided by the CS120B course staff:

- `timerISR.h`: Timer2 interrupt and scheduler setup
- `spiAVR.h`: SPI initialization (**modified**: clock changed to 2 MHz; see Debugging Notes)
- `serialatmega.h`: UART serial output for debugging
- `LCD.h`: LCD1602 character display driver
- `periph.h`: ADC helpers
- `helper.h`: bit manipulation helpers
- `st7735.h`: display driver template, **completed by me** (display commands implemented from the ST7735 datasheet, plus sprite drawing)
- `main.cpp` template: task struct, scheduler loop, melody data, and the `setTone()`/`offTone()` skeleton

All eight state machines, the game logic, and the flicker fixes are my own work.
