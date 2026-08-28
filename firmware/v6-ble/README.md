# V6 BLE Geiger firmware

Firmware for the **built TransmitterPCB V6** boards. Hardware, schematic, pinout, HV, and known board issues live in the [root README](../../readme.md).

This tree is not the ESP8266 / Wi-Fi firmware in [geigerControl](https://github.com/Sawaiz/geigerControl), and it is not the old Nordic ShockBurst link to a Raspberry Pi. The same nRF24 spoofs a Bluetooth LE advertisement so a phone can read device ID and counts without a custom receiver.

Built size (avr-gcc, `-Os`, attiny2313): **986 bytes flash / 10 bytes SRAM** of 2048 / 128.

## What the node does

1. Timer0 Fast PWM on PB2 runs continuously. On V6 that **is** the HV pump (`OCR0A = 125`, "Set to 400V"). Power-down would stop it. See the root README for V6 vs the V7 555 schematic.
2. Each falling edge on PD2 (INT0) is one GM pulse. The impulse stage is an NPN that pulls the line low.
3. The MCU idles. Timer1 ticks 1 Hz. After 60 s it snapshots the pulse count (that number *is* CPM) and clears the counter without dropping counts that arrive during the radio burst.
4. The nRF24 wakes, sends the same non-connectable BLE advertisement on channels 37, 38, and 39, then goes to POWER_DOWN again.

A phone running nRF Connect (or any BLE scanner) sees a device named **GM**. It will not pair. The nRF24L01+ cannot accept BLE connections.

## Pinout (firmware view)

| Function | ATtiny2313 | Notes |
|---|---|---|
| GM impulse | PD2 / INT0 | Falling edge. Idle sleep allows edges; power-down would not. |
| HV PWM | PB2 / OC0A | Fast PWM, clk/1, ~3.91 kHz |
| nRF CE | PD6 | Schematic net CN |
| nRF CSN | PD5 | |
| nRF SCK | PB0 | Not ISP SCK |
| nRF MOSI | PD4 | Not ISP MOSI |
| nRF MISO | PB1 | Not ISP MISO |
| ISP MOSI/MISO/SCK | PB5 / PB6 / PB7 | Leave as pulled-up inputs |
| RESET / dW | PA2 | Do not drive except via the programmer |

```
LFUSE = 0x64
HFUSE = 0xDF
EFUSE = 0xFF
MCU   = attiny2313
F_CPU = 1000000UL
```

`0x64` is the factory-ish 8 MHz oscillator /8. Do not speed up the chip without also changing `F_CPU`, Timer1's 1 Hz reload, and PWM frequency.

## High voltage PWM

V6 pumps HV from Timer0 on PB2 (`HV PWM` on that schematic). The 555 lives on the **V7** file in `pcb/schematic.dch`, not on the purple sticks. `bf09a87` used `OCR0A = 127`; this tree uses 125.

```c
TCCR0A = WGM00 | WGM01 | COM0A1;  /* Fast PWM, non-inverting OC0A */
TCCR0B = CS00;                    /* no prescale */
OCR0A  = 125;                     /* "Set to 400V" */
DDRB  |= PB2;
```

That is `hv_pwm_init()`, setpoint `HV_PWM_OCR`. Frequency = F_CPU / 256 ≈ **3.91 kHz**. Duty 125/256 ≈ **48.8%**.

Because Timer0 must keep running, the chip uses **`SLEEP_MODE_IDLE`**, not power-down. Power-down would stop the PWM and collapse HV.

Tune in `main.c`:

```c
#define HV_PWM_OCR     125u
#define DEVICE_ID      0x33
#define REPORT_SECONDS 60u
```

Board-level HV (inductor, Q1, V6 vs V7) is in the root README.

## Pulse counting

INT0 falling edge, one increment per ISR. 16-bit `pulses` is `volatile`; the reporter does `cli(); n = pulses; pulses = 0; sei();` so a pulse during the BLE burst lands in the next minute instead of being dropped.

Analog front-end (2N3904, 22 k / 100 k / 10 k, 220 pF) is in the root README. This firmware does not correct SBM-20 dead time. Ads send raw CPM, not µSv/h.

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

Grinberg's BLE encode is **non-commercial** unless you get his permission.

## Power (firmware)

HV stays on, so the chip idles rather than powering down. nRF is POWER_DOWN between ads (~1 µA plus three 1 Mbps packets per minute). Watchdog is 2 s, petted from the 1 Hz ISR. Cell (VL621, 3.0 V) is in the root README. Capacity is not in the CAD.

## Build and flash

```bash
sudo apt install gcc-avr avr-libc avrdude
cd firmware/v6-ble
make
make flashUsbtiny    # or: make flashIce
make fuses           # only if this chip is blank / wrong
```

`geiger_ble.hex` in this folder is a prebuilt image from that Makefile.

Programmer targets match the old V6 Makefile: `usbtiny` (including usbNanoISP-style) and `atmelice_isp`. Connector (TC2030 vs 2×3 ISP) is in the root README. Same pins: PB5/PB6/PB7/PA2.

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
- Do not enable power-down / PWM-off without accepting HV collapse.
- nRF24L01 (non-plus) is a worse BLE fake; the modules on the 2014 boards look like the **plus** (16 MHz crystal).
