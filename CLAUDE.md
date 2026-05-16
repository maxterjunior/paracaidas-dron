# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Emergency parachute deployment system for the DJI Matrice 400 drone (~6.7 kg, MTOW ~11 kg). Detects free-fall via IMU and triggers a spring-loaded parachute mechanism. The system is **independent of the drone's flight controller**.

## Key Architecture

### Hardware Design (KiCad)
- The KiCad project will live in `hardware/paracaidas-matrice400/` and is **hand-authored by the user** directly in the KiCad GUI (KiCad 7+) — there is no code-generation step. The previous `gen_sch.py` generator has been removed.
- The **authoritative design spec and netlist** is [docs/esquematicos/esquematico.md](docs/esquematicos/esquematico.md): full component table, ASCII schematic, pin-by-pin netlist, oscillator config, and design notes. Treat that document as the source of truth for connectivity.
- The `.kicad_sch` / `.kicad_pcb` / `.kicad_pro` files are not yet in the repo — the user is creating them. Do **not** generate or hand-write `.kicad_sch` s-expressions; if a design change is needed, update `docs/esquematicos/esquematico.md` and describe the change so the user can apply it in KiCad.

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

- **Design authoring is done by the user** in the KiCad GUI. Claude does not create or edit `.kicad_sch` / `.kicad_pcb` files.
- **Source of truth**: [docs/esquematicos/esquematico.md](docs/esquematicos/esquematico.md). To propose a connectivity or component change, edit that document (component table, netlist, design notes) and clearly describe the delta so the user can mirror it in KiCad. Keep that doc and this file consistent.
- The `kicad` skill can still be used to **analyze/review** KiCad files once the user has added them to `hardware/paracaidas-matrice400/`.
- **5V output voltage**: set by `Vout = 0.6 × (1 + R_FB1/R_FB2)` = 0.6 × 8.5 = **5.1V**

## Firmware (not yet implemented)
Firmware will target PIC18LF25K22, compiled with MPLAB XC8. Source goes in `firmware/src/`, libraries in `firmware/lib/`.
