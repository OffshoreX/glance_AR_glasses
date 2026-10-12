# GLANCE: Autofocal Reading Glasses (Board 2)

![GLANCE Board 2 cover](hardware/images/cover.png)

GLANCE started as AR smart glasses and became a sensing platform for **autofocal reading glasses**. An outward-facing distance sensor measures how far away the object in front of you is. Two inward-facing infrared eye sensors estimate where your eyes are pointing. Together they estimate the distance you are actually looking at, which is the signal a focus-tunable lens would need to adjust itself. The lens itself is out of scope for this project. This repo covers the electronics that do the sensing.

**Status:** PCB design is complete and passes design rule checks. Boards are being ordered for the Hack Club Half Life program. Nothing here has been fabricated or tested yet, so no measured results are claimed.

## How it works

- **Brain:** ESP32-S3-WROOM-1 module (Wi-Fi, BLE and native USB). The antenna end overhangs the board edge.
- **Eye sensing:** one 940 nm IR LED (IR333-A) and one phototransistor (PT334-6B) per eye, on an off-board eye module. Four phototransistor channels are read by an ADS1115 16-bit I2C ADC.
- **Distance sensing:** header for an outward-facing time-of-flight distance sensor module.
- **Power:** USB-C input, MCP73831 Li-ion charger, JST PH battery connector, slide switch for power, AP2112K-3.3 LDO for the 3.3 V rail.
- **Input:** four tactile buttons.

## Images

| | |
|---|---|
| ![Schematic](hardware/images/schematic.png) | ![PCB layout](hardware/images/pcb-layout.png) |
| Schematic | PCB layout (routed) |
| ![3D front](hardware/images/pcb-3d-front.png) | ![3D back](hardware/images/pcb-3d-back.png) |
| 3D view, front | 3D view, back |

## Board

- 2-layer FR-4, 37.75 x 65 mm
- Power nets on wider tracks, solid GND pour on both layers
- USB D+ and D- routed by hand as a pair. The remaining signals were autorouted with Freerouting (score 990/1000, 31 vias), then cleaned up by hand
- DRC: zero real errors at submission

## Repo layout

```
hardware/
  board2/                KiCad 10 project (glance-board2.*)
  gerbers/               Gerber and drill files for JLCPCB
  images/                README images
  glance_pcb_bom.csv     Full bill of materials with LCSC part numbers
firmware/                Firmware (ESP32-C3 code from the earlier Board 1 design)
```

Board 1 (the original ESP32-C3 AR design) is kept in the repo for history and is not part of this submission.

## Bill of materials

The full BOM with LCSC part numbers and prices is in [`hardware/glance_pcb_bom.csv`](hardware/glance_pcb_bom.csv). Parts come from LCSC and the bare PCBs from JLCPCB. Several LCSC parts have minimum orders of 5 or 10, so more parts are ordered than the board uses.

Funding note: the Half Life BOM table lists only the items covered by the $30 Tier 1 grant. The remaining parts (ADS1115, charger, LDO, connectors, switches, phototransistors) are bought in the same order and paid for personally. The ESP32-S3 module is also self-funded.

## Firmware

Board 2 firmware has not been written yet. The existing code in `firmware/` targets Board 1's ESP32-C3.

## Next steps

1. Receive the boards and check continuity with a multimeter before powering up.
2. Solder, then bring up power, USB and the ADC.
3. Read the eye sensors and the distance sensor, then estimate gaze distance.

## License

Add a license of your choice.
