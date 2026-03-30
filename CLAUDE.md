# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Emergency parachute deployment system for the DJI Matrice 400 drone (~6.7 kg, MTOW ~11 kg). Detects free-fall via IMU and triggers a spring-loaded parachute mechanism. The system is **independent of the drone's flight controller**.

## Key Architecture

### Hardware Design (KiCad)
- Project lives in `hardware/paracaidas-matrice400/`
- The schematic (`.kicad_sch`) is **code-generated** by `gen_sch.py` — do not hand-edit the `.kicad_sch` file; regenerate it instead
- PCB layout is in `.kicad_pcb`; project settings in `.kicad_pro`

### Schematic Generator (`gen_sch.py`)
`gen_sch.py` produces the full KiCad schematic as a `.kicad_sch` s-expression file. Its structure:

1. **Pin position tables** — symbol-coordinate pin offsets for every component type (R, C, L, NMOS, NPN, PIC, MPU-6050, etc.), using the KiCad Y-inversion convention (`abs_y = cy - rpy`)
2. **`COMPS` list** — each entry is `(lib_id, ref, value, cx, cy, angle, pin_map, footprint)` defining absolute placement
3. **`NETS` dict** — connectivity as `{net_name: [(ref, pin), ...]}` — this is the authoritative netlist
4. **`NC` list** — explicitly no-connect pins
5. **`lib_symbols()`** — inline KiCad symbol definitions (geometry + pins) for all used parts
6. **`build_schematic()`** — assembles the s-expression file from the above

To regenerate the schematic after changes:
```bash
cd hardware/paracaidas-matrice400
python gen_sch.py
```

### System Blocks

| Block | Components | Rails |
|---|---|---|
| Power — LDO 3.3V | BT1 (LiPo 1S) → SW1 → MCP1700 (U3) | 3.3V for PIC + IMU |
| Power — Boost 5V | MT3608 (U4), L1 22µH, R_FB1 750kΩ, R_FB2 100kΩ | 5V for solenoid + buzzer |
| MCU | PIC18LF25K22 (U1), 28-DIP, internal oscillator 64 MHz | 3.3V |
| IMU | MPU-6050 GY-521 (U2), I2C @ RC3/RC4, INT → RB0 | 3.3V |
| Deployment actuator | Solenoid 5V (SOL1) driven by IRLZ44N MOSFET (Q1), flyback D1 (1N4007) | 5V |
| Parachute mechanism | Spring tube; solenoid acts as latch (energized=held, triggered=released) | mechanical |
| Indicators | LED1 green (RA0), LED2 red (RA1), Buzzer via 2N3904 Q2 (RC0) | — |
| Debug/Programming | J1 = UART (RC6/RC7), J2 = ICSP PICKit (RB6/RB7) | — |

### Parachute Canopy
- Type: circular **Ripstop Nylon 30D sil-nylon**, **1.2–1.3 m diameter** (Cd ≈ 0.75, descent ≤6 m/s at 11 kg MTOW)
- Sizing formula: `Area = (2·m·g) / (Cd·ρ·v²)` → ~0.82 m² minimum → 1.2 m with safety margin
- Mechanism: **spring tube** — solenoid holds latch; when triggered, spring ejects parachute (~50–100 ms)
- Sources: Fruity Chutes, Mars Parachutes, AliExpress "drone parachute 1.2m ripstop"
- Details in `docs/investigacion/modulos.md`

### Free-fall Detection Logic
- Threshold: `|√(ax²+ay²+az²)| < 0.3g` sustained for >100 ms
- Integer math only (PIC18 has no FPU)
- First-order IIR filter to reject motor vibration
- MPU-6050 I2C address: 0x68 (AD0 tied to GND)

### PIC18LF25K22 Configuration Bits
```c
#pragma config FOSC    = INTIO67   // Internal oscillator, RA6/RA7 as GPIO
#pragma config PLLCFG  = ON        // PLL x4 → 64 MHz
#pragma config MCLRE   = EXTMCLR  // MCLR pin enabled (SW2 reset)
#pragma config LVP     = OFF
#pragma config WDTEN   = OFF       // Enable in production
```

## Working with the KiCad Schematic

- **To view/edit**: open `paracaidas-matrice400.kicad_sch` in KiCad 7+
- **To modify connectivity or placement**: edit `gen_sch.py` (COMPS, NETS, or pin tables), then regenerate
- **Y-axis convention**: KiCad schematic uses screen-Y-down; symbols use math-Y-up. The generator handles this with `abs_y = cy - rpy`
- **5V output voltage**: set by `Vout = 0.6 × (1 + R_FB1/R_FB2)` = 0.6 × 8.5 = **5.1V**

## Firmware (not yet implemented)
Firmware will target PIC18LF25K22, compiled with MPLAB XC8. Source goes in `firmware/src/`, libraries in `firmware/lib/`.
