# Final project — `led_register`
## Serial-controlled 8-bit Binary LED Register (Loongson 1B)

A small interactive system that exercises **UART input parsing + GPIO output +
number formatting** on the board, and is verifiable both in PuTTY and with the
eight LEDs.

### Why this instead of a calculator
A calculator only prints text. This project drives **hardware** (the 8 LEDs wired
per Table 8.14) *and* speaks over the serial port, so the demo shows:

* serial line parsing (`SET 42`, `SHIFT L`, `INC`, …)
* decimal / hex / binary conversion inside the program
* real GPIO output on 8 pins, active-low (`gpio_write(pin, 0)` = LED on)

### Commands (typed in PuTTY, UART3 = COM7, 115200)

| Command | Effect |
|---|---|
| `<number>`, `0x2A`, `SET <number>` | load the register, LEDs show it in binary |
| `INC` / `DEC` | +1 / -1 |
| `SHIFT L` / `SHIFT R` | `value << 1` / `value >> 1` |
| `NOT` | invert all 8 bits |
| `RESET` | register = 0 |
| `BIN` | print the value again |
| `HELP` | command list |

Every command answers with, for example:

```
  DEC= 42  HEX=0x2A  BIN=00101010
  register> 
```

and the LEDs light accordingly (LED1 = bit0 = LSB … LED8 = bit7).

### Hardware
* LEDs: LED1..LED8 = GPIO **9, 11, 15, 17, 20, 22, 6, 7** (Table 8.14), VCC → 3.3 V
* Serial: blue USB↔RS232 adapter on the board's UART3 socket → `COM7` @ 115200
* Download: LoongIDE `Debug → Run` over the LxLink JTAG probe

### Build
* Project `C:\Users\mmirz\loongson1B_example\led_register` (`led_register.lxp`)
* 84 compile units, `text 54552`, entry `0x80201030`
* Toolchain SDE Lite 4.9.2 for MIPS, `-O1`, `DownloadByNet=0`, `SetBPAtStart=0`

### Demo script (what to show the teacher)
1. Power the board, open PuTTY (COM7, 115200), `Debug → Run` from LoongIDE
2. `HELP` → command list
3. `SET 42` → LEDs show `00101010`, board replies `DEC= 42  HEX=0x2A  BIN=00101010`
4. `SHIFT L` → `DEC= 84  HEX=0x54  BIN=01010100` (LEDs shift)
5. `NOT` → `DEC=171  HEX=0xAB  BIN=10101011`
6. `RESET` → all LEDs off, `DEC=  0  HEX=0x00  BIN=00000000`

### Files
* `main.c` — the whole program (GPIO init, UART init, command parser, binary display)
* `led_register.lxp` — LoongIDE project
* `Build\led_register.exe` — linked MIPS image used by `Debug → Run`
