# Wireless Geiger Counter

The boards that were actually built are **TransmitterPCB V6** (2014): ATtiny2313 SOIC, nRF24L01+, SBM-20, Panasonic VL621, purple ENIG stick in an acrylic tube. Firmware for those boards is [`firmware/v6-ble`](firmware/v6-ble).

Two later drawings sit in the same tree and are easy to mix up:

- `pcb/schematic.dch` on this branch is **V7-lineage** (git `8667ba7` “Made V7 pcb main pcb”, then `58bc4fb`). QFN ATtiny2313, **ICM7555** HV, no `HV PWM` port. The DipTrace library path inside still says `TransmitterPCB\V6`.
- The ESP-12F / 18650 / Wi-Fi tube in the circular renders is a **later concept**. Not V6, not V7.

The V6 schematic (SOIC, MCU PWM, no 555) is still in git as `fbc6006:PCB/TX/schematic.dch`. It is not the file in `pcb/` today.

| Era | Hardware | Firmware |
|---|---|---|
| 2014–2015 **V6** (built) | SOIC ATtiny2313, nRF24 module, SBM-20, VL621, MCU **HV PWM** on PB2, no 555 | [`firmware/v6-ble`](firmware/v6-ble). Old ShockBurst TX/RX deleted in `050199b`; last copy `bf09a87` `code/avrTx` + `code/rPiRx`. Finalised `7bf4a14`. |
| **V7** schematic (on disk) | MLF/QFN ATtiny2313, ICM7555 boost, extra R6–R10 / C5 / D2 / Q3 | Not this firmware. |
| Later concept | ESP-12F, 18650, Wi-Fi, modular circular stack | [geigerControl](https://github.com/Sawaiz/geigerControl) (captive portal / OTA / websocket; does not count pulses) |

![][oldPrototypes]

## Hardware (V6, as built)

Silk: **GM V6** / **Geiger V6** / **Sawaiz Syed** / **2014**. Long narrow purple ENIG stick (photos; OSH Park is inferred, not in the CAD). About 20 cm radio-end to tube-end. Not the stacked circular modules in `cad/renderings/labeledSections.png`.

1. **nRF24L01+ module** on a 2×4 header. PCB meander antenna off the end, **16.000 MHz** crystal on the *module* (not a schematic part). That 16 MHz part is why BLE fakery is plausible.
2. **Main PCB:** 20-pin **SOIC** ATtiny2313, 10-pin headers along both long edges, six gold **Tag-Connect** pads, cylindrical **10 mH** inductor toward the tube.
3. **SBM-20** in fuse-clip supports. End PCB silkscreened **GM V6**. Nets `GM+` / `GM-`. Tube body on one photo is marked СССР.

`cad/Housing` is the clear acrylic / ~¾ in PVC tube with frosted caps. `cad/2.54 Breakaway` is Inventor parts for 1×10, 2×3, 2×4, holder. Those match this stick.

### Where the copper went

V6 layout **did** exist in git and was deleted: `top.dip`, `bottom.dip`, `V6 TXTop.dip`, `TXTop Gerber/*`, `TXBot Gerber/*`, `bom.xlsx` (gone in `c13fcfa` / `e0d78b0` / `2470e58`). V7 `layout.dip` + `bom.xlsx` were deleted in `050199b`. `.gitignore` still lists `*.gbr`.

Working tree today: `pcb/schematic.dch` only. You cannot regenerate the purple boards from this branch.

V6 schematic blob (recover with `git show`): `fbc6006:PCB/TX/schematic.dch`.

### V6 schematic (the built boards)

Hierarchical sheets: Assembly, MCU, HV Supply, Signal, Power, NRF24L01, ICSP, GM Tube.

HV Supply ports include **HV PWM** and HV Out. There is **no ICM7555**. U1 is **ATTINY2313_SOIC**. Programmer is **U2** TC2030-MCP-NL.

| Ref | What (V6 file) |
|---|---|
| B1 | Panasonic VL621/F9D, 3.0 V (tabbed lithium). Capacity is **not** in the files. |
| U1 | ATTINY2313_SOIC (library alt `ATINY2313_PDIP`, sic) |
| U2 | TC2030-MCP-NL (ICSP). PIC pin names on an AVR. |
| L1 | 10 mH `12LRS106C` |
| Q1 | FJN3303FBU-ND / “TRANS NPN 400V 1.5A” (also 2N4123 in the library) |
| Q2 | 2N3904 impulse NPN |
| D1 | 1N3016 in the file (alts 1N4007, 1N4937FSCT-ND 600 V 1 A) |
| C1 | 10 nF 1 kV (`1255PH-ND`) |
| C2 | 220 pF 1 kV (`1273PH-ND`) on the Geiger input |
| C3, C4 | 0.1 µF, 10 µF |
| R1 | 4.7 MΩ HV bleed / divider |
| R2, R3, R4, R5 | 22 kΩ, 100 kΩ, 10 kΩ, 10 kΩ |
| J3, J4 | HDR-1×10 debug (all MCU pins) |
| J5 | NRF24L01 8-pin (library also NRF2401) |
| J6, J7 | `GM+` / `GM-`, silk **SBM20 Suppour** |
| J2 | ICSP |

nRF module 1–8: GND, +3 V, **CN** (this is CE; firmware PD6), CSN, SCK, MOSI, MISO, IRQ. IRQ is on the sheet; firmware does not use it.

Paper crops in the 2015 pack (`MCU.jpg`, `NRF24L01.jpg`, `Impulse.jpg`) match **this** V6 sheet, not the V7 file.

### Pinout

From `bf09a87` `code/avrTx` (firmware-verified). Schematic agrees by **port names**; the Assembly netlist in the binary `.dch` was not decoded. SPI is bit-banged so ISP can keep PB5/PB6/PB7.

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Active-low NPN collector. V6 TX waited while `PIND2` low; v6-ble uses falling edge. |
| HV PWM | PB2 / OC0A | V6 HV Supply input **HV PWM**. Fast PWM ~3.91 kHz at 1 MHz. |
| nRF CE | PD6 | Schematic net **CN** |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | AIN0. Not ISP SCK. |
| nRF MOSI | PD4 | Not ISP MOSI |
| nRF MISO | PB1 | AIN1. Not ISP MISO |
| nRF IRQ | on the module sheet | Unused by `firmware/v6-ble` |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | TC2030 (and 2×3 0.1 in on some photos) |
| RESET / dW | PA2 | Programmer only |

No crystal on the TX schematic (XTAL1/XTAL2 unpopulated). Clock is firmware: 1 MHz internal, `LFUSE=0x64`, `HFUSE=0xDF`, `EFUSE=0xFF`.

J3: PA2, PD0, PD1, PA1, PA0, PD2, PD3, PD4, PD5, GND.

J4: PD6, PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, VCC.

UART (PD0/PD1) is broken out and unused. V6 C had LED leftovers on PD3 / PB3; those ports exist, not shown net-tied in the dump.

### High voltage (V6)

MCU Timer0 Fast PWM on PB2 / OC0A into the HV Supply **HV PWM** port, 10 mH (L1), 400 V NPN (Q1), 1 kV C1, rectifier D1, 4.7 MΩ R1. **No 555 on V6.** `bf09a87` `pwm_init()` used `OCR0A = 127` (“Set to 400V”); `firmware/v6-ble` uses 125. That PWM is the V6 pump, not a leftover.

The 2313 has no ADC. AIN0/AIN1 are PB0/PB1, which are nRF SCK/MISO, so the comparator cannot watch a HV divider. No HV-sense net in the schematic.

Idle sleep only: power-down would stop Timer0 and collapse the boost. SBM-20 wants ~400 V. C1/C2 are HV-side (1 kV).

### Impulse

Q2 2N3904, R2 22 kΩ, R3 100 kΩ pull-down, C2 220 pF, R4 10 kΩ pull-up to +3 V. Paper JPEG labels the input **Gieger** (that spelling is on the crop, not recovered from the `.dch`) and the output **Impulse**. Tube hit → Q2 on → Impulse low. Idle high.

SBM-20 dead time ~190 µs. Firmware does not correct it. Rough Cs-137: µSv/h ≈ CPM × 0.0057 (scanner-side; ads send raw CPM).

### Power

B1 Panasonic **VL621/F9D**, 3.0 V, rails **+3V** / GND. No 18650, no 3.3 V regulator, no fuel gauge on this schematic. Cell capacity is not in the repo. The “months on a charge” story is the ESP / 18650 concept.

Dominant loads: MCU PWM HV boost, ATtiny2313 idle at 1 MHz, nRF POWER_DOWN plus three short BLE packets per minute.

### Programming

Tag-Connect **TC2030-MCP-NL**: 1 `~MCLR/Vpp`, 2 Vdd, 3 Gnd, 4 PGD, 5 PGC, 6 `[LVP]`. AVR mapping: MOSI/MISO/SCK/RESET = **PB5 / PB6 / PB7 / PA2**, not the nRF bit-bang pins. Photos also show a 2×3 header on some boards. Same signals. Flash procedure is in [`firmware/v6-ble`](firmware/v6-ble).

### Known issues

- **Wrong schematic in `pcb/` for the built boards.** Current file is V7 (555, MLF). V6 is `fbc6006:PCB/TX/schematic.dch`. Mixing them is the usual trap.
- **Layout and gerbers were deleted.** V6 `.dip` + TXTop/TXBot gerbers are still blobs in history, not in the tree.
- **No tube-voltage telemetry.** No ADC; comparator pins are nRF SCK/MISO.
- **nRF SPI is bit-banged** so ISP can keep PB5/6/7. Do not move it onto hardware SPI.
- **Net names:** CE is **CN**; ICSP MISO is **MSIO**; silk **SBM20 Suppour**; paper **Gieger**; library **ATINY**, **UCSK**.
- **IRQ unused** by the BLE firmware.
- Matching 2014 firmware was deleted from `master` in `050199b` and never appeared in geigerControl.

## V7 schematic (file on disk, not the purple sticks)

`pcb/schematic.dch` after `8667ba7` / `58bc4fb`:

- U1 **ATTINY2313_MLF** QFN-20 4×4 (PD2 becomes package pin 4, PB2 pin 12 — firmware still uses port names).
- U2 **ICM7555** / TLC555C. HV Supply has **HV Out only** (no HV PWM port).
- U3 TC2030-MCP-NL.
- Extra: C5 1 nF, D2 1N5914, Q3 2N3903, R6 330 Ω, R7 220 kΩ, R8 100 kΩ, R9 1 kΩ, R10 labelled `15-25`.
- J1–J6 Molex/Hirose 87758-20; J10 nRF; J11/J12 GM +/−.
- Bourns 3386P exists in the **library**, not as a placed part.
- Hierarchical block **HV Connector**.

Do not flash `firmware/v6-ble` expecting a 555; that hex PWM's PB2 for **V6**.

```
pcb/schematic.dch                    V7-lineage DipTrace (not the built V6 netlist)
fbc6006:PCB/TX/schematic.dch         V6 schematic (git blob)
cad/Housing                          acrylic tube for the stick
cad/2.54 Breakaway                   header / holder Inventor parts
cad/renderings/oldPrototypes.jpg     built V6 units in those tubes
cad/renderings/labeledSections.png   later modular stack (not V6)
cad/renderings/assembledTube.png     later concept render
firmware/v6-ble/                     firmware for the built V6 boards
```

## Later concept (ESP-12F / 18650)

A single-board Wi-Fi node on an 18650, meant to last months and sit in a waterproof plastic shell. Renders only.

![][assembledTube]

![][labeledSections]

### Power

18650 Li-ion, 1.5–3.4 Ah, with protection and fuel gauge. Board rail 3.3 V.

### Wireless

ESP-12F, on-board antenna (chip antenna optional). 802.11 b/g/n, claimed hundreds of metres.

### Microcontroller

ESP8266/ESP8285, Tensilica L106 at 80 MHz, 13 GPIO, one 10-bit ADC, not 5 V tolerant. Average ~80 mA on Wi-Fi, peaks ~500 mA for a few milliseconds.

### High voltage

Schmitt-trigger oscillator boost into a Cockcroft–Walton, divided and compared to a reference with an op-amp. V6 used the microcontroller PWM pump and could not deep-sleep. Adapted from [Maxim AN3757](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3757).

## Firmware

- Built V6 boards: [`firmware/v6-ble`](firmware/v6-ble) — non-connectable BLE advertisements, device ID + CPM once a minute, HV PWM on PB2.
- Later ESP concept: [geigerControl](https://github.com/Sawaiz/geigerControl).

[oldPrototypes]: cad/renderings/oldPrototypes.jpg "Built V6 units in acrylic tubes"
[assembledTube]: cad/renderings/assembledTube.png "Later modular tube (concept)"
[labeledSections]: cad/renderings/labeledSections.png "Later modular stack (concept)"
