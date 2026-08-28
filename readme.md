# Wireless Geiger Counter

Firmware in this branch is for **TransmitterPCB V6**, the boards that were built in 2014. Source of truth for that hardware:

- CAD: [`pcb/v6/`](pcb/v6/) from git `fbc6006:PCB/TX/` (schematic, `top.dip`, `bottom.dip`, `bom.xlsx`)
- Firmware pinout / PWM: `bf09a87` `code/avrTx` (last working TX; finalised `7bf4a14`)
- Code now: [`firmware/v6-ble`](firmware/v6-ble)

`pcb/schematic.dch` is **V7** (`8667ba7` “Made V7 pcb main pcb”). QFN MCU, ICM7555, no `HV PWM` port. Not these boards.

The ESP-12F / 18650 / Wi-Fi tube in the circular renders is a later concept. Not V6, not this firmware.

| Era | Hardware | Firmware |
|---|---|---|
| **V6** (built, 2014) | SOIC ATtiny2313A-SU, nRF24L01+, SBM-20, OSH Park TX Top+Bot, MCU HV PWM on PB2, no 555 | [`firmware/v6-ble`](firmware/v6-ble). Old ShockBurst deleted `050199b`; last copy `bf09a87`. |
| **V7** schematic | MLF/QFN 2313, ICM7555 boost | Not this firmware. File: `pcb/schematic.dch`. |
| Later concept | ESP-12F, 18650, Wi-Fi, modular stack | [geigerControl](https://github.com/Sawaiz/geigerControl) |

![][oldPrototypes]

## Hardware (V6)

Silk: **GM V6** / **Geiger V6** / **Sawaiz Syed** / **2014**. Two purple ENIG boards (BOM: OSH Park, **TX Top + TX Bot**, 2.23 in²). Long stick, about 20 cm radio to tube. Not the circular stack in `cad/renderings/labeledSections.png`.

1. **nRF24L01+** module on a 2×4 (BOM: eBay nRF24L01+ with PCB antenna). Photos show a **16.000 MHz** crystal on the module.
2. **TX Top:** SOIC **ATTINY2313A-SU**, 10-pin headers J3/J4, ICSP, HV inductor, boost parts.
3. **TX Bot / tube end:** SBM-20 in fuse clips, silk **GM V6**. Nets `GM+` / `GM-`.

`cad/Housing` is the acrylic / ~¾ in PVC tube. `cad/2.54 Breakaway` matches the 0.1 in headers (1×10, 2×3, 2×4).

### BOM (`pcb/v6/bom.xlsx`)

Parts as purchased. Schematic library names in parentheses when they differ.

| Ref | Qty | Part (BOM) |
|---|---|---|
| PCB | 1 | OSH Park, TX Top + TX Bot, 2.22875 in² |
| U1 | 1 | Atmel **ATTINY2313A-SU**, 20-SOIC (Digi-Key ATTINY2313A-SU-ND) |
| D1 | 1 | Fairchild **RS1M**, 1 kV 1 A SMA |
| Q1 | 1 | ST **STN2580**, 400 V 1 A SOT-223 (HV switch) |
| Q2 | 1 | NXP **MMBT3904**, SOT-23 (sense / impulse) |
| L1 | 1 | Bourns **SDR1005-103KL**, 10 mH |
| C1 | 1 | Kemet **C1206C103KFRACTU**, 10 nF 1.5 kV (HV) |
| C2 | 1 | TDK **C3216C0G2J221K060AA**, 220 pF 630 V (sense) |
| R1 | 1 | 4.7 MΩ 0805 (“Gieger Resistor” in the sheet) |
| R2 | 1 | 22 kΩ 0805 |
| R3 | 1 | 100 kΩ 0805 |
| R4, R5 | 2 | 10 kΩ 0805 |
| C3 | 1 | 0.1 µF 50 V 0805 |
| C4 | 1 | 10 µF 10 V 0805 |
| — | 1 | **nRF24L01+** 2.4 GHz module |
| ICSP | 1 | Harwin **M20-9980346**, 2×3 0.1 in |
| B1 | 1 | MPD **BC12AAL**, 2×AA solder-lug holder |
| J3, J4 | 2 | Sullins **PREC010SAAN-RC**, 10-pin 0.1 in |
| Geiger | 1 | Vniitfa **SBM-20** |

Schematic `pcb/v6/schematic.dch` also has a Panasonic **VL621/F9D** 3.0 V cell symbol and a **TC2030-MCP-NL** footprint (U2). BOM bought a 2×AA holder and a 2×3 Harwin instead. Photos show Tag-Connect gold pads on some boards and a 2×3 on others. Rails in the schematic are **+3V** / GND.

No 555, no 18650, no 3.3 V regulator, no fuel gauge in the V6 BOM or schematic.

### Schematic map (`pcb/v6/schematic.dch`)

Sheets: Assembly, MCU, HV Supply, Signal, Power, NRF24L01, ICSP, GM Tube.

HV Supply ports: **HV PWM**, HV Out. NRF24 ports: IRQ, MOSI, CSN, MISO, SCK, **CN** (CE). ICSP: RST, **MSIO** (sic), SCK, MOSI.

nRF module 1–8: GND, +3 V, CN, CSN, SCK, MOSI, MISO, IRQ.

Layouts: `pcb/v6/top.dip`, `pcb/v6/bottom.dip`. Gerber zips from this era were deleted later; `.gitignore` still has `*.gbr`.

### Pinout

From `bf09a87` `code/avrTx` (`radioPinFunctions.c`, `avrTx.c`). Firmware-verified. Schematic agrees by port names.

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Active-low Q2 collector. v6-ble: falling edge. |
| HV PWM | PB2 / OC0A | V6 HV Supply **HV PWM**. `OCR0A = 127` in avrTx. |
| nRF CE | PD6 | Schematic **CN** |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | AIN0. Not ISP SCK. |
| nRF MOSI | PD4 | Not ISP MOSI |
| nRF MISO | PB1 | AIN1. Not ISP MISO |
| nRF IRQ | on the module sheet | Unused by v6-ble |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | TC2030 and/or 2×3 |
| RESET / dW | PA2 | Programmer only |

No MCU crystal on the schematic. Clock is firmware: 1 MHz internal, `LFUSE=0x64`, `HFUSE=0xDF`, `EFUSE=0xFF`.

J3: PA2, PD0, PD1, PA1, PA0, PD2, PD3, PD4, PD5, GND.

J4: PD6, PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, VCC.

### High voltage

V6 pumps HV from Timer0 Fast PWM on PB2 into **HV PWM**, then L1 10 mH, Q1 STN2580, D1 RS1M, C1 10 nF 1.5 kV, R1 4.7 MΩ. **No 555.** `firmware/v6-ble` keeps that PWM (`OCR0A = 127`). Idle sleep only: power-down stops Timer0 and the boost dies.

No ADC on the 2313. AIN0/AIN1 are nRF SCK/MISO. No HV-sense net.

SBM-20 wants ~400 V. avrTx comment on OCR0A: “Set to 400V”.

### Impulse

Q2 MMBT3904, R2 22 kΩ, R3 100 kΩ, C2 220 pF, R4 10 kΩ pull-up. Paper crop labels the input **Gieger** and the output **Impulse**. Hit → Q2 on → PD2 low.

### Programming

BOM: Harwin 2×3. Schematic: TC2030-MCP-NL (PIC pin names on an AVR). Signals: PB5 / PB6 / PB7 / PA2. Procedure in [`firmware/v6-ble`](firmware/v6-ble).

### Known issues

- **`pcb/schematic.dch` is V7.** Use [`pcb/v6/`](pcb/v6/) for this firmware.
- **BOM vs schematic** on power (2×AA holder vs VL621 symbol) and programming (2×3 vs TC2030).
- **No tube-voltage telemetry.**
- **nRF SPI is bit-banged** so ISP can keep PB5/6/7.
- Typos in the V6 files: **CN** for CE, **MSIO**, **Gieger**, **SBM20 Suppour**, **ATINY**, **UCSK**.
- ShockBurst TX/RX was deleted in `050199b` and never landed in geigerControl.

```
pcb/v6/                 V6 schematic, layout, BOM  (fbc6006)
pcb/schematic.dch       V7 schematic (not the built boards)
cad/Housing             acrylic tube
cad/2.54 Breakaway      0.1 in headers / holder
cad/renderings/oldPrototypes.jpg   V6 units in tubes
firmware/v6-ble/        firmware for V6
```

## V7 schematic (not this firmware)

`pcb/schematic.dch` after `8667ba7` / `58bc4fb`: U1 ATTINY2313_MLF QFN-20, U2 ICM7555, no HV PWM port, extra R6–R10 / C5 / D2 / Q3. Library path inside still says `TransmitterPCB\V6`. Do not flash v6-ble expecting a 555.

## Later concept (ESP-12F / 18650)

Renders only. 18650, Wi-Fi, Cockcroft–Walton + op-amp (Maxim AN3757). V6 used MCU PWM and could not deep-sleep.

![][assembledTube]

![][labeledSections]

### Power

18650 Li-ion, 1.5–3.4 Ah, protection and fuel gauge. Rail 3.3 V.

### Wireless

ESP-12F, on-board antenna. 802.11 b/g/n.

### Microcontroller

ESP8266/ESP8285, 80 MHz, 13 GPIO, 10-bit ADC, not 5 V tolerant. ~80 mA on Wi-Fi, peaks ~500 mA.

### High voltage

Schmitt-trigger boost into a Cockcroft–Walton, divided and compared to a reference. Adapted from [Maxim AN3757](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3757).

## Firmware

- V6 boards: [`firmware/v6-ble`](firmware/v6-ble) — BLE advertisements, device ID + CPM once a minute, HV PWM on PB2.
- Later ESP concept: [geigerControl](https://github.com/Sawaiz/geigerControl).

[oldPrototypes]: cad/renderings/oldPrototypes.jpg "Built V6 units in acrylic tubes"
[assembledTube]: cad/renderings/assembledTube.png "Later modular tube (concept)"
[labeledSections]: cad/renderings/labeledSections.png "Later modular stack (concept)"
