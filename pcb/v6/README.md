# TransmitterPCB V6

Restored from git `fbc6006:PCB/TX/` (“Move V6 out”). This is the CAD for the **built 2014 boards** and for [`firmware/v6-ble`](../../firmware/v6-ble).

| File | What |
|---|---|
| `schematic.dch` | DipTrace schematic. U1 `ATTINY2313_SOIC`, HV Supply port **HV PWM**, **no** ICM7555. |
| `top.dip` / `bottom.dip` | DipTrace layouts (TX Top + TX Bot). |
| `bom.xlsx` | Digi-Key / eBay BOM used to buy the parts. |

`../schematic.dch` on this branch is **V7** (`8667ba7`). Do not mix it with this firmware.

Board-level writeup is in the [root README](../../readme.md).
