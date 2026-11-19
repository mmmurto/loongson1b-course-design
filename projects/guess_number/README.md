# Final project — `guess_number`
## "Guess the Number" game on the Loongson 1B

An interactive serial game: the board picks a secret number between 1 and 100
(seeded from the hardware tick counter), the player types guesses on the serial
port, and the board answers **HIGHER / LOWER / CORRECT**.

Chosen as the final project because it is the same difficulty as a calculator but
demonstrates more of the system at once: **UART input parsing**, **arithmetic and
comparison**, **hardware timing (random seed)**, **GPIO output on 8 LEDs**, and
**timing control (`delay_ms`) for the win animation**.

---

## 1. How to run it

1. Power the board, connect the blue USB↔RS232 adapter to the board's **UART5**
   socket, and open **PuTTY → Serial → COM7, 115200**.
2. In LoongIDE open `guess_number.lxp` and choose **`Debug → Run`** (F9).
3. The board prints:

```
=== Loongson 1B : Guess the Number ===

  NEW GAME - I picked a number between 1 and 100.
  You have 7 tries. Enter a guess.
  guess>
```

## 2. Commands

| Input | Meaning |
|---|---|
| `1` … `100` | a guess |
| `NEW` | start a new game (new secret number) |
| `HELP` | command list |

## 3. Behaviour

* Secret number: `(get_clock_ticks() ^ (get_clock_ticks() << 7)) % 100 + 1`
  — different on every run because it is seeded from the hardware timer.
* Seven tries; after each guess the board answers **HIGHER** or **LOWER** and
  prints `tries left: N`.
* On a win: `CORRECT! you found it in N tries`, then the eight LEDs run a
  chase animation before showing the number in binary.
* If the tries run out: `out of tries! the number was NN`.
* **The LEDs always show the last guess in binary** (LED1 = bit0 … LED8 = bit7,
  active low, Table 8.14 wiring).

## 4. Captured session (evidence)

```
guess> 50   LOWER
tries left: 6
guess> 25   LOWER
tries left: 5
...
guess> 15   HIGHER
tries left: 3
guess> 17   CORRECT!
you found it in 6 tries. type NEW for another game
guess> NEW
  NEW GAME - I picked a number between 1 and 100.
  You have 7 tries. Enter a guess.
  guess> 25   HIGHER
  tries left: 6
```

Screenshots: `report\screenshots\project_guess_number_start.png` and
`report\screenshots\project_guess_number.png`.

## 5. Where the code is

`main.c` (≈250 lines, single file):

| Function | Purpose |
|---|---|
| `gpio_init()` | configures GPIO 9,11,15,17,20,22,6,7 as outputs |
| `show_bits()` | displays a value on the 8 LEDs in binary |
| `uart_init()` / `uart_puts()` / `uart_putc()` | serial on **UART5** (`devUART5`) |
| `new_game()` | seeds and picks the secret, resets tries |
| `help()` | prints the command list |
| `win_effect()` | LED chase animation on a win |
| `handle()` | command parsing, comparison, replies |
| `main()` | init + read‑character loop with local echo |

## 6. Build

* LoongIDE project `guess_number.lxp`, MCU **LS1B200 (LS232)**,
  toolchain **SDE Lite 4.9.2 for MIPS**, `-O1`, bare‑metal (`None`)
* 84 compile units → `Build\guess_number.exe`, `text 55112`, entry `0x80201030`
* Project settings needed for reliable download: `DownloadByNet=0`,
  `SetBPAtStart=0`, `ResetAtStart=1`

## 7. Note on the serial port

The blue CH340 adapter is wired to the board's **UART5** socket, so the program
uses `devUART5`. (Chapter 6's manual text says UART3; with this kit's adapter the
working port is UART5 — using `devUART3` produces no output.)
