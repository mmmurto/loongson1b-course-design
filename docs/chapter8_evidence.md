# Chapter 8 — Implementation of Network Transmission — EVIDENCE

**Guide:** Computer System Comprehensive Design Experiment Guide, Chapter 8 (§8.1 – §8.2.15)
**Project:** `C:\Users\mmirz\loongson1B_example\example_9_1`
**Board:** Loongson 1B (LS1B200), kit #34
**Date of capture:** 2026‑11‑11

---

## 1. What was done

| Guide section | Content | Status |
|---|---|---|
| §8.2.1 | Create project `example_9_1` — **RT‑Thread + lwIP 1.4.1** component | ✅ |
| §8.2.2 | `main.c` — Code Listing 10‑1 (initialise lwIP with `192.168.1.123`, start the selected test) | ✅ |
| §8.2.3 | `lwip_test.h` — Code Listing 10‑2 (test selection macros, IPs, ports) | ✅ |
| §8.2.4 | `lwip_test.c` — Code Listing 10‑3 (`local_IP` / `remote_IP` strings) | ✅ |
| §8.2.5 | `tcp_server.c` — Code Listing 10‑4 | ✅ |
| §8.2.6 | `tcp_client.c` — Code Listing 10‑5 | ✅ |
| §8.2.7 | `udp_server.c` — Code Listing 10‑6 | ✅ |
| §8.2.8 | `udp_client.c` — Code Listing 10‑7 | ✅ |
| §8.2.11 | PC IP set to **192.168.1.111 / 255.255.255.0**; other adapters disabled | ✅ |
| §8.2.12 | **TCP Server test** — board = server `192.168.1.123:9060`, PC = client | ✅ **executed on hardware** |
| §8.2.13 | **TCP Client test** — board = client → PC server `192.168.1.111:9061` | ✅ **executed on hardware** |
| §8.2.14 | **UDP Server test** — board = server `192.168.1.123:9062`, PC = client | ✅ **executed on hardware** |
| §8.2.15 | **UDP Client test** — board = client → PC `192.168.1.111:9063` | ✅ **executed on hardware** |

Build result (SDE Lite 4.9.2, MIPS32 little‑endian):

```
168 compile units -> example_9_1.exe
   text    data     bss     dec     hex
 275256    5584   64536   345376   54520
entry: 0x802112FC
```

`lwip_test.h` had exactly one test enabled:

```
#define TEST_TCP_SERVER   1
#define TEST_TCP_CLIENT   0
#define TEST_UDP_SERVER   0
#define TEST_UDP_CLIENT   0
```

---

## 2. Evidence A — the board's own console (COM5, 115200 baud)

Screenshot: `C:\Users\mmirz\fs\ch8_putty.png`

```
client incoming succeed!
RECEIVE: hello from PC (192.168.1.111)

   SEND: hello from PC (192.168.1.111)

disconnect client succeed!
waiting for client access!
```

This is the board's `printk` trace for §8.2.12 — compare the guide's Figure 8.6
(*"Data received by the Loongson 1B hardware development platform displayed in the
PuTTY command-line interface"*).

---

## 3. Evidence B — the PC side (TCP client)

Script used (`C:\Users\mmirz\fs\tcp_client.ps1`) — equivalent to the NetAssist
"TCP Client" configuration of Figure 8.5:

```
CONNECTED to 192.168.1.123:9060  (board = TCP server, PC = TCP client)
PC -> board        : 31 bytes : 'hello from PC (192.168.1.111)'
board -> PC        : 31 bytes : 'hello from PC (192.168.1.111)'
CONNECTION CLOSED
```

plus `ping 192.168.1.123` → reply (board reachable on the network).

---

## 3b. Evidence C — §8.2.13 TCP Client test

`lwip_test.h`: `TEST_TCP_CLIENT=1`, others 0. PC runs a TCP **server** on
`192.168.1.111:9061` (equivalent to NetAssist "TCP Server" in Figure 8.7):

```
=== TCP server on 192.168.1.111:9061 ===
listening, waiting for the board (192.168.1.123)...
board connected from 192.168.1.123:49153
RECEIVE from board: 21 bytes : 'client ticks = 2005.'
SEND to board: 'hello from PC server'
```

---

## 3c. Evidence D — §8.2.14 UDP Server test

`lwip_test.h`: `TEST_UDP_SERVER=1`, others 0. PC = UDP client → `192.168.1.123:9062`
(NetAssist "UDP", Figure 8.9):

```
UDP -> 192.168.1.123:9062 sent 33 bytes : 'hello UDP from PC (192.168.1.111)'
UDP <- received 33 bytes from 192.168.1.123:9062 : 'hello UDP from PC (192.168.1.111)'
```

---

## 3d. Evidence E — §8.2.15 UDP Client test

`lwip_test.h`: `TEST_UDP_CLIENT=1`, others 0. PC listens on UDP `9063`
(NetAssist "UDP" server side, Figure 8.11):

```
UDP <- board 192.168.1.123:49153 sent 20 bytes : 'client ticks = 2004.'
UDP -> sent reply to the board
```

The board's payload is exactly Code Listing 10-7's `"client ticks = %i."`.

---

## 3e. NetAssist (the tool the guide asks for)

Installed at **`C:\Program Files (x86)\NetAssist\netassist.exe`** with a desktop
shortcut `NetAssist.lnk` (alongside `PuTTY.lnk` and `Tftpd32.lnk`).
The tests above were driven from PowerShell sockets because that is scriptable and
reproducible; NetAssist does exactly the same four roles by hand:

| Guide section | NetAssist setting |
|---|---|
| §8.2.12 TCP Server | Protocol **TCP Client**, remote `192.168.1.123:9060` |
| §8.2.13 TCP Client | Protocol **TCP Server**, local `192.168.1.111:9061` |
| §8.2.14 UDP Server | Protocol **UDP**, target `192.168.1.123:9062` |
| §8.2.15 UDP Client | Protocol **UDP**, local `192.168.1.111:9063` |

---

## 4. Reproducing it in front of the teacher (5 minutes)

1. Ethernet cable: **board RJ45 → laptop LAN port**. Laptop NIC: `192.168.1.111`, mask `255.255.255.0`.
2. PuTTY → **Serial**, `COM5`, **115200** → *Open* (this is the board console).
3. LoongIDE → open `example_9_1.lxp` → **`Project → Compile & Build`** → **`Debug → Run`** (F9).
   The board prints `Welcome to RT-Thread.` and `succeed to create sock_fd!`.
4. On the PC: NetAssist → **TCP Client**, remote host `192.168.1.123`, port `9060`
   → *Connect* → type `hello` → *Send*.
5. The board console shows `RECEIVE: …` / `SEND: …` (Evidence A), and NetAssist shows the echo (Evidence B).

---

## 5. Notes / troubleshooting already solved

* **`DownloadByNet=1`** in the project file made every `Debug → Run` hang at
  `wait for pracs of LS232/1B(1C)`. Correct value for this kit: **`DownloadByNet=0`**
  (JTAG download through the LxLink probe).
* The board must be **reset** before a new download; otherwise the previous program
  is still running and you can get a `CPU Exception: TLB Load` dump.
* The board IP `192.168.1.123` is set inside `ls1x_initialize_lwip()` in
  `lwIP-1.4.1/port/ls1x_ethernetif.c` (called by `main.c` with
  `gmac0_ip = {192,168,1,123}`).
