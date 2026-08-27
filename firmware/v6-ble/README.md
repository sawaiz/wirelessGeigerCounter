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

## Hardware pinout

Taken from the last working firmware (`bf09a87` `code/avrTx/nrf24/radioPinFunctions.c` and `avrTx.c`). SPI is bit-banged. Hardware SPI pins PB5/PB6/PB7 are left as pulled-up inputs so ISP still works.

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Active low. Firmware uses **falling edge** (idle sleep allows edges; power-down would not). |
| HV PWM | PB2 / OC0A | Fast PWM, clk/1, ~3.91 kHz at 1 MHz |
| nRF CE | PD6 | |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | Not the ISP SCK pin |
| nRF MOSI | PD4 | Not the ISP MOSI pin |
| nRF MISO | PB1 | Not the ISP MISO pin |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | Tag-Connect TC2030 on the V6 board |
| RESET / dW | PA2 | Do not drive |

Clock: 1 MHz internal (CKDIV8). Fuses copied from the old Makefile:

```
LFUSE = 0x64
HFUSE = 0xDF
EFUSE = 0xFF
MCU   = attiny2313
F_CPU = 1000000UL
```

`0x64` is the factory-ish 8 MHz oscillator /8. Do not "speed up" the chip without also changing `F_CPU`, Timer1's 1 Hz reload, and PWM frequency.

## High voltage

The 2014 TX (`Software/1975 Greifswald/...c` and later `code/avrTx/avrTx.c`) pumped HV in software:

```c
TCCR0A = WGM00 | WGM01 | COM0A1;  /* Fast PWM, non-inverting OC0A */
TCCR0B = CS00;                    /* no prescale */
OCR0A  = 125;                     /* "Set to 400V" */
DDRB  |= PB2;
```

That block is still in `main.c` as `hv_pwm_init()`, setpoint `HV_PWM_OCR`.

**Numbers**

- PWM frequency = F_CPU / 256 = 1 000 000 / 256 ≈ **3.91 kHz**
- Duty = OCR0A / 256. Default 125/256 ≈ **48.8%**
- The 2313 has **no ADC**. AIN0/AIN1 are PB0/PB1, which are nRF SCK/MISO, so the analog comparator cannot watch a HV divider either.
- Monitoring is not possible in software on this MCU. Regulation is open-loop PWM, and/or the ICM7555 + op-amp loop on the schematic if that part is populated.

Because Timer0 must keep running, the chip uses **`SLEEP_MODE_IDLE`**, not power-down. Power-down would stop the PWM and the boost would collapse. That is the same reason the original README said MCU-driven HV could not deep-sleep.

Tune in `main.c`:

```c
#define HV_PWM_OCR     125u
#define DEVICE_ID      0x33
#define REPORT_SECONDS 60u
```

## Pulse counting

The V6 "Impulse" sheet is a 2N3904 (or similar) with 22 kΩ into the base, 10 kΩ pull-up on the collector to 3 V. A tube hit pulls **Impulse / PD2 low**.

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

Dominant loads, in order:

1. HV boost (MCU PWM and/or 555). Firmware cannot turn this off without losing the tube bias.
2. ATtiny2313 **idle** at 1 MHz, ~0.2–0.5 mA typical at 3 V.
3. nRF24: POWER_DOWN ~1 µA, plus three short 1 Mbps packets per minute (~11 mA for a few milliseconds). Average from the radio is ~1 µA.

The V6 cell in the schematic is a Panasonic **VL621** (~5.5 mAh). At 0.5 mA idle+HV that is hours, not months. The 18650 / "months on a charge" story in the root README is the later ESP design. Do not expect that from this firmware on a VL621.

Watchdog is 2 s, petted from the 1 Hz ISR, so a stuck radio wait resets the chip.

## Build and flash

```bash
sudo apt install gcc-avr avr-libc avrdude
cd firmware/v6-ble
make
make flashUsbtiny    # or: make flashIce
make fuses           # only if this chip is blank / wrong
```

`geiger_ble.hex` in this folder is a prebuilt image from that Makefile.

Programmer targets match the old V6 Makefile: `usbtiny` (including usbNanoISP-style) and `atmelice_isp`. The board has a Tag-Connect **TC2030**.

ISP wiring is the usual TC2030: MISO/MOSI/SCK/RESET on PB6/PB5/PB7/PA2, not the nRF bit-bang pins.

## Files

```
firmware/v6-ble/
  main.c           source (one translation unit, keeps the 2 KB budget)
  Makefile         attiny2313, hex, flash, fuses
  geiger_ble.hex   prebuilt
  README.md        this file
```

## Limitations

- Broadcast only. No GATT, no pairing, no Pi ShockBurst RX (that packet format is gone).
- Open-loop HV. No tube-voltage telemetry.
- CPM is pulses in the last 60 s, not a sliding window and not dead-time corrected.
- nRF24L01 (non-plus) is a worse BLE fake; the modules on the 2014 boards look like the **plus** (16 MHz crystal).
- Grinberg's BLE encode is **non-commercial** unless you get his permission. Link: https://dmitry.gr/?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery
