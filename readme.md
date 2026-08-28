# Wireless Geiger Counter

Two generations live in this repo. The boards that were actually built, and the only schematic on disk, are **TransmitterPCB V6** (2014): ATtiny2313, nRF24L01+, SBM-20, Panasonic VL621, OSH Park purple stick in an acrylic tube. Firmware for those boards is [`firmware/v6-ble`](firmware/v6-ble).

The ESP-12F / 18650 / Wi-Fi tube in the renders below is a **later concept**. It is not what `pcb/schematic.dch` describes, and it is not what the 2014 units are.

| Era | Hardware | Firmware |
|---|---|---|
| 2014–2015 V6 (built) | ATtiny2313 + nRF24L01+ + SBM-20, VL621, acrylic tube | [`firmware/v6-ble`](firmware/v6-ble) (BLE ads). Old ShockBurst TX/RX lived here until `050199b`; last copy `bf09a87` `code/avrTx` + `code/rPiRx`. |
| Later concept | ESP-12F, 18650, Wi-Fi, modular circular stack | [geigerControl](https://github.com/Sawaiz/geigerControl) (captive portal / OTA / websocket; does not count pulses) |

![][oldPrototypes]

## Hardware (V6, as built)

Silk on the assembled sticks: **GM V6** / **Geiger V6** / **Sawaiz Syed** / **2014**. Long narrow OSH Park purple board, about 20 cm radio-end to tube-end, not the stacked circular modules in `cad/renderings/labeledSections.png`.

1. **nRF24L01+ module** on a 2×4 header. PCB meander antenna off the end, **16.000 MHz** crystal on the module (needed for the BLE advertisement hack).
2. **Main PCB:** SOIC ATtiny2313, SMD passives, HV inductor, 555 / boost, debug headers along the long edges, programming header near the radio.
3. **SBM-20** in fuse-clip supports. End PCB silkscreened **GM V6**. Anode/cathode nets `GM+` / `GM-`.

`cad/Housing` is the clear acrylic tube (~¾ in) with frosted caps. `cad/2.54 Breakaway` is Inventor parts for the 0.1 in headers (2×3, 2×4, 1×10, tube holder). Those match this stick.

Copper source is incomplete:

- `pcb/schematic.dch` is DipTrace **TransmitterPCB V6** (original path `…/TransmitterPCB/V6/DipTrace/`).
- There is **no** `.dip` layout, **no** Gerbers, **no** OSH Park job. You cannot regenerate the purple boards from git. `58bc4fb` updated the schematic; layout never landed here.

### Schematic map

`pcb/schematic.dch` is hierarchical. Sheets / blocks:

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

Values below are what the schematic stores. Library alternates in parentheses.

| Ref | What |
|---|---|
| B1 | Panasonic VL621/F9D, 3.0 V rechargeable lithium (tabbed coin, ~5.5 mAh class) |
| U1 | ATtiny2313. Footprint **ATTINY2313_MLF** QFN-20 4×4; library also has `ATINY2313_PDIP` (sic). Built boards look **SOIC-20**, not QFN. |
| U2 | ICM7555 / TLC555C, SOIC-8. On-board HV oscillator. |
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
| Debug | Hirose **87758-20** 20-pos 2 mm headers (all MCU pins). The 2015 MCU sheet draws these as two 10-pin rows. |

nRF module pin order: 1 GND, 2 +3 V, 3 **CN** (CE; firmware `PD6`), 4 CSN, 5 SCK, 6 MOSI, 7 MISO, 8 IRQ. IRQ is wired; current firmware does not use it.

### Pinout

From the last working TX (`bf09a87` `code/avrTx`) and the V6 MCU sheet. SPI is bit-banged so ISP can keep PB5/PB6/PB7.

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Active low (NPN collector). Falling edge. |
| HV PWM | PB2 / OC0A | Fast PWM ~3.91 kHz at 1 MHz. See High voltage. |
| nRF CE | PD6 | Schematic net **CN** |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | Not ISP SCK |
| nRF MOSI | PD4 | Not ISP MOSI |
| nRF MISO | PB1 | Not ISP MISO |
| nRF IRQ | on the module sheet | Unused by `firmware/v6-ble` |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | TC2030 and/or 2×3 0.1 in |
| RESET / dW | PA2 | Programmer only |

Clock: 1 MHz internal (CKDIV8). Fuses from the old Makefile: `LFUSE=0x64`, `HFUSE=0xDF`, `EFUSE=0xFF`.

Debug headers (20 pins):

- **J3-style:** PA2 (Reset), PD0 (RXD), PD1 (TXD), PA1 (XTAL2), PA0 (XTAL1), PD2 (INT0), PD3 (INT1), PD4, PD5, GND.
- **J4-style:** PD6, PB0, PB1, PB2, PB3, PB4, PB5 (MOSI), PB6 (MISO), PB7 (SCK), VCC (+3 V).

UART (PD0/PD1) is broken out and unused.

### High voltage

Two stories sit on the same board, and both are real.

**Schematic:** ICM7555 (U2) boost into 10 mH (L1) and a 400 V NPN (Q1), 600 V diode (D1), 1 kV caps, 4.7 MΩ bleed (R1), Bourns 3386 trim. That is a **555 boost**, not a Cockcroft–Walton. The CW multiplier + op-amp loop in [Later concept](#later-concept-esp-12f--18650) is the ESP-era design (Maxim AN3757).

**2014 firmware** (and `firmware/v6-ble`): Timer0 Fast PWM on PB2 / OC0A, `OCR0A = 125` ("Set to 400V"), ~3.91 kHz, ~49% duty. The 2313 has **no ADC**. AIN0/AIN1 are PB0/PB1 (nRF SCK/MISO), so the comparator cannot watch a HV divider either. No tube-voltage telemetry.

The 555 exists because MCU PWM cannot deep-sleep. Current firmware still PWM's PB2: the last known-good TX did, and there is no `.dip` to prove PB2 is disconnected from the boost. If the 555 is the only pump, PB2 is wasted toggling; if PB2 still drives the switch, turning PWM off collapses HV. Measure tube voltage before changing either.

SBM-20 wants ~400 V. C1/C2 (1 kV) are HV-side. The 3.6 V zener is a **3 V rail** clamp, not a tube clamp.

### Impulse

Q2 2N3904, R2 22 kΩ into the base, R3 100 kΩ pull-down, C2 220 pF (1 kV) on the "Gieger" (sic) node, R4 10 kΩ collector pull-up to 3 V. A tube hit turns Q2 on and pulls **Impulse / PD2 low**. Idle is high.

SBM-20 dead time ~190 µs. At ~20 CPM that is nothing. Firmware does not correct dead time. Rough Cs-137 figure: µSv/h ≈ CPM × 0.0057 (conversion belongs on the scanner; ads send raw CPM).

### Power

Panasonic **VL621/F9D** (~5.5 mAh, 3.0 V). D2 (1N5914, 3.6 V) on that rail. No 18650, no fuel gauge, no 3.3 V buck.

Dominant loads: HV boost (555 and/or MCU PWM), then ATtiny2313 idle at 1 MHz (~0.2–0.5 mA at 3 V), then nRF24 POWER_DOWN plus three short BLE packets per minute. At 0.5 mA the VL621 is **hours, not months**.

### Programming

Tag-Connect **TC2030-MCP-NL** (U3): `~MCLR/Vpp`, PGC, PGD, `[LVP]`, Gnd. MOSI/MISO/SCK/RESET = **PB5 / PB6 / PB7 / PA2**, not the nRF bit-bang pins. Some built boards use a **2×3 0.1 in ISP** next to the radio, or the long debug headers as a fixture. Same signals. Flash procedure is in [`firmware/v6-ble`](firmware/v6-ble).

### Known issues

- **No PCB layout in git.** Schematic only. Do not assume pin-swap fixes without continuity on a real board.
- **Two HV controllers.** 555 on copper vs Timer0 PWM in firmware. Measure before changing either.
- **No tube-voltage telemetry.** 2313 has no ADC; comparator pins are nRF SCK/MISO.
- **U1 footprint mismatch.** Schematic QFN-20 MLF vs SOIC-20 on the purple boards vs PDIP in the library.
- **nRF SPI is bit-banged** so ISP can keep the hardware SPI pins. Do not move it onto PB5/6/7.
- **IRQ is unused** by the BLE firmware.
- **DipTrace typos:** "Gieger", "SBM20 Suppour", "ATINY2313", "MSIO", net **CN** for CE.
- **VL621 is tiny.** Hours with HV on, not the ESP-era months.
- Matching 2014 firmware was deleted from `master` in `050199b` and never appeared in geigerControl.

```
pcb/schematic.dch                    DipTrace TransmitterPCB V6 (no .dip)
cad/Housing                          acrylic tube for the stick
cad/2.54 Breakaway                   header / holder Inventor parts
cad/renderings/oldPrototypes.jpg     built V6 units in those tubes
cad/renderings/labeledSections.png   later modular stack (not V6)
cad/renderings/assembledTube.png     later concept render
firmware/v6-ble/                     firmware for the built boards
```

## Later concept (ESP-12F / 18650)

A single-board Wi-Fi node on an 18650, meant to last months and sit in a waterproof plastic shell. Renders only; not the V6 schematic.

![][assembledTube]

![][labeledSections]

### Power

18650 Li-ion, 1.5–3.4 Ah, with protection and fuel gauge. Board rail 3.3 V.

### Wireless

ESP-12F, on-board antenna (chip antenna optional). 802.11 b/g/n, claimed hundreds of metres.

### Microcontroller

ESP8266/ESP8285, Tensilica L106 at 80 MHz, 13 GPIO, one 10-bit ADC, not 5 V tolerant. Average ~80 mA on Wi-Fi, peaks ~500 mA for a few milliseconds.

### High voltage

Schmitt-trigger oscillator boost into a Cockcroft–Walton, divided and compared to a reference with an op-amp. The previous (V6) design used the microcontroller to pump HV and could not deep-sleep. Adapted from [Maxim AN3757](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3757).

## Firmware

- Built V6 boards: [`firmware/v6-ble`](firmware/v6-ble) — non-connectable BLE advertisements, device ID + CPM once a minute, HV PWM kept on PB2.
- Later ESP concept: [geigerControl](https://github.com/Sawaiz/geigerControl).

[oldPrototypes]: cad/renderings/oldPrototypes.jpg "Built V6 units in acrylic tubes"
[assembledTube]: cad/renderings/assembledTube.png "Later modular tube (concept)"
[labeledSections]: cad/renderings/labeledSections.png "Later modular stack (concept)"
