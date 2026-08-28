# V6 BLE Geiger firmware

Firmware for the **built TransmitterPCB V6** boards in this repo: ATtiny2313, nRF24L01+ (or nRF24L01), SBM-20, 3.0 V cell.

It is not the ESP8266 / Wi-Fi firmware in [geigerControl](https://github.com/Sawaiz/geigerControl), and it is not the old Nordic ShockBurst link to a Raspberry Pi. Those V6 boards talk nRF24, not Wi-Fi. This tree makes the same radio spoof a Bluetooth LE advertisement so a phone can read device ID and counts without a custom receiver.

Built size (avr-gcc, `-Os`, attiny2313): **986 bytes flash / 10 bytes SRAM** of 2048 / 128.

## What the node does

1. Timer0 Fast PWM on PB2 runs continuously. That is the original MCU high-voltage pump (`OCR0A = 125` was marked "Set to 400V" in the 2014 TX).
2. Each falling edge on PD2 (INT0) is one GM pulse. The V6 impulse stage is an NPN that pulls the line low.
3. The MCU idles. Timer1 ticks 1 Hz. After 60 s it snapshots the pulse count (that number *is* CPM) and clears the counter without dropping counts that arrive during the radio burst.
4. The nRF24 wakes, sends the same non-connectable BLE advertisement on channels 37, 38, and 39, then goes to POWER_DOWN again.

A phone running nRF Connect (or any BLE scanner) sees a device named **GM**. It will not pair. The nRF24L01+ cannot accept BLE connections.

## How this relates to the rest of the repo

| Era | What | Where |
|---|---|---|
| 2014–2015 V6 | ATtiny2313 + nRF24L01 ShockBurst TX, Pi RX into MariaDB | Deleted from `master` in `050199b` ("moved to geiger control"). Last copy: `bf09a87` `code/avrTx` and `code/rPiRx`. TX+RX "finalised" in `7bf4a14`. |
| 2015 poster | AVR node + wireless + Pi; Wi-Fi listed as next gen | CAD renderings of the later modular tube |
| 2016 `geigerControl` | ESP8266 captive-portal / OTA / websocket | Does not count pulses and does not run on these V6 boards |
| This branch | Same V6 pinout, BLE advertising instead of ShockBurst | `firmware/v6-ble/` |

The schematic in `pcb/schematic.dch` is DipTrace **TransmitterPCB V6**: ATtiny2313, nRF24L01, ICM7555 HV, SBM-20, Panasonic VL621 3.0 V. The README at the repo root describes a later ESP-12F / 18650 design that is not what these boards are.

## The PCB this firmware is for

Built boards (2014 photos, silk **GM V6** / **Geiger V6** / **Sawaiz Syed** / **2014**) are a long, narrow **OSH Park purple** stick, not the later stacked circular modules in `cad/renderings/labeledSections.png`.

Rough layout, radio end to tube end, about 20 cm overall:

1. **nRF24L01+ module** on a 2×4 header. PCB trace (meander) antenna hanging off the end, **16.000 MHz** crystal on the module. That 16 MHz part is why BLE fakery is plausible; a non-plus nRF24L01 is a worse fake.
2. **Main PCB**: SOIC ATtiny2313, SMD passives, HV inductor, 555 / boost parts, debug headers along the long edges, programming header near the radio.
3. **SBM-20** in fuse-clip supports. A small end PCB is silkscreened **GM V6**. Tube anode/cathode are `GM+` / `GM-`.

`cad/Housing` is the clear **acrylic tube** (about ¾ in) with frosted caps that the 2014 units sat in (`cad/renderings/oldPrototypes.jpg`). `cad/2.54 Breakaway` is Inventor parts for the 0.1 in headers (2×3, 2×4, 1×10, tube holder). Those match the stick. The exploded circular stack labeled High Voltage / Motherboard / Modular / Wireless is a **later** mechanical concept; do not flash this firmware onto that.

Source CAD for the copper is incomplete:

- `pcb/schematic.dch` is present (DipTrace schematic, original path `…/TransmitterPCB/V6/DipTrace/`).
- There is **no** `.dip` PCB layout, **no** Gerbers, **no** OSH Park job in the repo. You cannot regenerate the purple boards from git. `58bc4fb` updated the schematic; layout never landed here.

## Schematic map

`pcb/schematic.dch` is a hierarchical DipTrace file. Sheets / blocks pulled from it:

| Block | Role |
|---|---|
| Assembly | Top |
| Power | Panasonic **VL621/F9D** 3.0 V lithium cell (`B1`) |
| MCU | **U1** ATtiny2313, all Port A/B/D nets, debug headers |
| HV Supply | **U2** ICM7555 (TLC555C footprint), **L1** 10 mH, **Q1** 400 V NPN |
| Signal / Impulse | **Q2** 2N3904 pulse shaper, `Impulse` → PD2 |
| NRF24L01 | 8-pin module: GND, +3 V, CN (CE), CSN, SCK, MOSI, MISO, IRQ |
| ICSP | **U3** Tag-Connect **TC2030-MCP-NL** |
| GM Tube / HV Connector | `GM+`, `GM-`, SBM-20 supports (silk in the file: "SBM20 Suppour") |

### Parts worth knowing

Values are what the schematic actually stores. Library alternates are in parentheses.

| Ref | What |
|---|---|
| B1 | Panasonic VL621/F9D, 3.0 V rechargeable lithium (tabbed coin, ~5.5 mAh class) |
| U1 | ATtiny2313. Schematic footprint **ATTINY2313_MLF** QFN-20 4×4; library also has `ATINY2313_PDIP` (sic). Built boards look **SOIC-20**, not QFN. |
| U2 | ICM7555 / TLC555C, SOIC-8. This is the on-board HV oscillator. |
| U3 | TC2030-MCP-NL (Tag-Connect, no legs) |
| L1 | 10 mH (`NL56505` / `12LRS106C`) |
| Q1 | HV switch, Digi-Key **FJN3303FBU-ND** (400 V NPN, SOT-223). Library also lists 2N4123 / "TRANS NPN 400V 1.5A". |
| Q2 | 2N3904, impulse NPN |
| D1 | 600 V fast recovery 1 A (`1N4937FSCT-ND`; 1N4007 also in the file) |
| D2 | 1N5914 (3.6 V zener) on the 3 V rail |
| C1 | 10 nF / 1 kV (`1255PH-ND`) |
| C2 | 220 pF / 1 kV (`1273PH-ND`) on the Geiger input |
| C3, C4, C5 | 0.1 µF, 10 µF, 1 nF |
| R1 | 4.7 MΩ (HV bleed / divider) |
| R2, R3, R4 | 22 kΩ, 100 kΩ, 10 kΩ (impulse: base, base pull-down, collector pull-up) |
| R6–R9 | 330 Ω, 220 kΩ, 100 kΩ, 1 kΩ (555 / boost neighbourhood) |
| R10 | labelled `15-25` in the file (likely the HV trim range) |
| — | Bourns **3386P** trimpot (HV setpoint) |
| J11 / J12 | `GM+` / `GM-` |
| nRF header | HDR-2×4 |
| Debug | Hirose **87758-20** 20-pos 2 mm headers (all MCU pins). The 2015 "MCU" sheet draws these as two 10-pin rows. |

nRF module pin order on that sheet: 1 GND, 2 +3 V, 3 **CN** (this is CE; firmware `PD6`), 4 CSN, 5 SCK, 6 MOSI, 7 MISO, 8 IRQ. IRQ is wired. This firmware does not use it.

## Hardware pinout

Taken from the last working firmware (`bf09a87` `code/avrTx/nrf24/radioPinFunctions.c` and `avrTx.c`) and the V6 MCU sheet. SPI is bit-banged. Hardware SPI pins PB5/PB6/PB7 are left as pulled-up inputs so ISP still works.

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Active low. Firmware uses **falling edge** (idle sleep allows edges; power-down would not). |
| HV PWM | PB2 / OC0A | Fast PWM, clk/1, ~3.91 kHz at 1 MHz. See [High voltage](#high-voltage). |
| nRF CE | PD6 | Schematic net **CN** |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | Not the ISP SCK pin |
| nRF MOSI | PD4 | Not the ISP MOSI pin |
| nRF MISO | PB1 | Not the ISP MISO pin |
| nRF IRQ | (on the module sheet) | Not used by this firmware |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | TC2030 and/or 2×3 0.1 in |
| RESET / dW | PA2 | Do not drive except via the programmer |

Clock: 1 MHz internal (CKDIV8). Fuses copied from the old Makefile:

```
LFUSE = 0x64
HFUSE = 0xDF
EFUSE = 0xFF
MCU   = attiny2313
F_CPU = 1000000UL
```

`0x64` is the factory-ish 8 MHz oscillator /8. Do not "speed up" the chip without also changing `F_CPU`, Timer1's 1 Hz reload, and PWM frequency.

Debug headers on the MCU sheet (20 pins total):

**J3-style row:** PA2 (Reset), PD0 (RXD), PD1 (TXD), PA1 (XTAL2), PA0 (XTAL1), PD2 (INT0), PD3 (INT1), PD4, PD5, GND.

**J4-style row:** PD6, PB0, PB1, PB2, PB3, PB4, PB5 (MOSI), PB6 (MISO), PB7 (SCK), VCC (+3 V).

UART is broken out (PD0/PD1) but unused here.

## High voltage

Two stories sit on the same board, and both are real.

**Schematic (V6):** ICM7555 (U2) running a boost into 10 mH (L1) and a 400 V NPN (Q1), 600 V diode (D1), 1 kV caps, 4.7 MΩ bleed (R1), Bourns 3386 trim. That is a **555 boost**, not a Cockcroft–Walton. The CW multiplier + op-amp loop in the root README is the later ESP-era design (Maxim AN3757). Do not mix them.

**2014 firmware:** Timer0 Fast PWM on PB2 / OC0A, `OCR0A = 125`, comment "Set to 400V":

```c
TCCR0A = WGM00 | WGM01 | COM0A1;  /* Fast PWM, non-inverting OC0A */
TCCR0B = CS00;                    /* no prescale */
OCR0A  = 125;                     /* "Set to 400V" */
DDRB  |= PB2;
```

That block is still in `main.c` as `hv_pwm_init()`, setpoint `HV_PWM_OCR`.

The original root README says the MCU pump could not deep-sleep (PWM had to keep running, ~80 mA class on that older silicon), which is why the 555 exists. This firmware still PWM's PB2 because:

- the last known-good V6 TX did, and we do not have copper to prove PB2 is disconnected from the boost
- if the 555 is the only pump, PB2 is wasted toggling; if PB2 still drives the switch, turning PWM off collapses HV

Without a `.dip` there is no honest way to pick. Measure tube voltage before changing `hv_pwm_init()`.

**Numbers (MCU PWM path)**

- PWM frequency = F_CPU / 256 = 1 000 000 / 256 ≈ **3.91 kHz**
- Duty = OCR0A / 256. Default 125/256 ≈ **48.8%**
- The 2313 has **no ADC**. AIN0/AIN1 are PB0/PB1, which are nRF SCK/MISO, so the analog comparator cannot watch a HV divider either.
- Monitoring is not possible in software on this MCU. Regulation is open-loop PWM, and/or the ICM7555 + trim loop on the schematic if that part is populated.

Because Timer0 must keep running, the chip uses **`SLEEP_MODE_IDLE`**, not power-down. Power-down would stop the PWM and the boost would collapse.

Tune in `main.c`:

```c
#define HV_PWM_OCR     125u
#define DEVICE_ID      0x33
#define REPORT_SECONDS 60u
```

SBM-20 wants on the order of 400 V. 1 kV-rated C1/C2 are the HV-side parts; the 3.6 V zener is **not** a tube clamp.

## Pulse counting

The V6 Impulse sheet is Q2 2N3904, R2 22 kΩ into the base, R3 100 kΩ base pull-down, C2 220 pF (1 kV) on the "Gieger" (sic) node, R4 10 kΩ pull-up on the collector to 3 V. A tube hit turns Q2 on and pulls **Impulse / PD2 low**. Idle is high.

INT0 is set to falling edge. The ISR is one increment. 16-bit `pulses` is `volatile`; the reporter does `cli(); n = pulses; pulses = 0; sei();` so a pulse during the BLE burst is not lost (it lands in the *next* minute, which is correct, rather than being dropped).

SBM-20 dead time is ~190 µs. At background (~20 CPM) that is nothing. At 10 000 CPM:

```
m = 10000/60 ≈ 166.7 s⁻¹
n = m / (1 - m τ) ≈ 172.2 s⁻¹    (τ = 190e-6)
```

about 3% under-count. This firmware does not correct dead time.

Rough dose (Cs-137, typical SBM-20 figure ~174 CPM per µSv/h):

```
µSv/h ≈ CPM × 0.0057
```

The advertisement sends raw CPM, not µSv/h, so the conversion stays on the scanner.

## BLE advertisement (the nRF24 hack)

The nRF24L01+ is **not** a Bluetooth radio. BLE advertising happens to be 1 Mbps GFSK on 2402 / 2426 / 2480 MHz. Following [Dmitry Grinberg, Faking Bluetooth LE](https://dmitry.gr/?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery):

- nRF hardware CRC off (BLE wants CRC24)
- Auto-ack / Enhanced ShockBurst off
- 4-byte address = bit-reversed access address `0x8E89BED6`
- Software CRC24 (init `0x555555`), data whitening, bit reverse
- ADV_NONCONN_IND only. Connections are impossible (nRF payload ≤ 32 bytes; CONNECT_REQ is longer; hopping is too slow)

RF channels: nRF `RF_CH` 2, 26, 80 → 2402, 2426, 2480 MHz → BLE 37, 38, 39.

### On-air PDU (before whitening)

| Offset | Bytes | Value | Meaning |
|---|---|---|---|
| 0 | 1 | `0x42` | ADV_NONCONN_IND, random TxAdd (works on Android; `0x40` is the iOS-leaning variant) |
| 1 | 1 | `0x14` | 20 bytes follow |
| 2–7 | 6 | `C2 E1 6E 52 00 <id>` | static-looking random MAC; last byte is `DEVICE_ID` |
| 8–10 | 3 | `02 01 06` | flags: LE-only, general discoverable |
| 11–14 | 4 | `03 08 47 4D` | short name `"GM"` |
| 15–21 | 7 | `06 FF FF FF <id> <cpm_lo> <cpm_hi>` | manufacturer `0xFFFF`, then id + CPM little-endian |
| 22–24 | 3 | CRC24 | filled in by `btLePacketEncode` |

**nRF Connect:** scan → **GM** → manufacturer data `FF FF <id> <cpm_lo> <cpm_hi>`.

Example: id `0x33`, 20 CPM → `33 14 00`.

Change PDU type to `0x40` in `ble_advertise()` if iOS scanners ignore `0x42`.

## Power

The V6 cell in the schematic is a Panasonic **VL621/F9D** (~5.5 mAh, 3.0 V). D2 (1N5914, 3.6 V) sits on that rail. There is no 18650, no fuel gauge, no 3.3 V buck.

Dominant loads, in order:

1. HV boost (MCU PWM and/or 555). Firmware cannot turn this off without losing the tube bias.
2. ATtiny2313 **idle** at 1 MHz, ~0.2–0.5 mA typical at 3 V.
3. nRF24: POWER_DOWN ~1 µA, plus three short 1 Mbps packets per minute (~11 mA for a few milliseconds). Average from the radio is ~1 µA.

At 0.5 mA idle+HV the VL621 is **hours, not months**. The 18650 / "months on a charge" story in the root README is the later ESP design.

Watchdog is 2 s, petted from the 1 Hz ISR, so a stuck radio wait resets the chip.

## Programming

Schematic programmer is Tag-Connect **TC2030-MCP-NL** (U3): `~MCLR/Vpp`, PGC, PGD, `[LVP]`, Gnd. That is MOSI/MISO/SCK/RESET on **PB5 / PB6 / PB7 / PA2**, not the nRF bit-bang pins.

Some built boards instead show a **2×3 0.1 in ISP** next to the radio, or the long debug headers used as a programming fixture. Same signals.

```bash
sudo apt install gcc-avr avr-libc avrdude
cd firmware/v6-ble
make
make flashUsbtiny    # or: make flashIce
make fuses           # only if this chip is blank / wrong
```

`geiger_ble.hex` in this folder is a prebuilt image from that Makefile.

Programmer targets match the old V6 Makefile: `usbtiny` (including usbNanoISP-style) and `atmelice_isp`.

## Known hardware / repo issues

- **No PCB layout in git.** Schematic only. Do not assume pin-swap "fixes" without continuity on a real board.
- **Two HV controllers.** 555 boost on copper vs Timer0 PWM in firmware. Measure before changing either.
- **No tube-voltage telemetry.** 2313 has no ADC; comparator pins are nRF SCK/MISO.
- **U1 footprint mismatch.** Schematic QFN-20 MLF vs SOIC-20 on the purple boards vs PDIP in the library. The firmware does not care; a respin would.
- **nRF SPI is bit-banged** so ISP can keep the hardware SPI pins. Do not "simplify" onto PB5/6/7.
- **IRQ is unused.** Fine for advertise-and-sleep. A future ShockBurst RX would want it.
- **Silk / name typos in the DipTrace file:** "Gieger", "SBM20 Suppour", "ATINY2313", "MSIO", net **CN** for CE.
- **Root README and `cad/renderings/labeledSections.png` / `assembledTube.png` are the later ESP-12F / 18650 / Wi-Fi tube.** Flashing this hex there will do nothing useful.
- **Matching 2014 firmware was deleted** from `master` in `050199b` and never actually appeared in geigerControl. This branch is the replacement for the boards that still exist.
- **VL621 is tiny.** Expect runtime in hours with HV on, not the ESP-era "months".
- **nRF24L01 (non-plus)** is a worse BLE fake; the modules on the 2014 boards look like the **plus** (16 MHz crystal).
- Grinberg's BLE encode is **non-commercial** unless you get his permission. Link: https://dmitry.gr/?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery

## Files

```
firmware/v6-ble/
  main.c           source (one translation unit, keeps the 2 KB budget)
  Makefile         attiny2313, hex, flash, fuses
  geiger_ble.hex   prebuilt
  README.md        this file

pcb/schematic.dch  DipTrace TransmitterPCB V6 (no .dip layout)
cad/Housing        acrylic tube for the stick
cad/2.54 Breakaway header / holder Inventor parts
cad/renderings/oldPrototypes.jpg   built V6 units in those tubes
cad/renderings/labeledSections.png later modular stack (not this board)
```

## Limitations

- Broadcast only. No GATT, no pairing, no Pi ShockBurst RX (that packet format is gone).
- Open-loop HV. No tube-voltage telemetry.
- CPM is pulses in the last 60 s, not a sliding window and not dead-time corrected.
- Do not enable power-down / PWM-off without accepting HV collapse.
