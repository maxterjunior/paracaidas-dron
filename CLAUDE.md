# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Emergency parachute deployment system for the DJI Matrice 400 drone (~6.7 kg, MTOW ~11 kg). Detects free-fall via IMU and triggers a spring-loaded parachute mechanism. The system is **independent of the drone's flight controller**.

## Key Architecture

### Hardware Design (KiCad)
- The KiCad project (KiCad 10) lives in `hardware/paracaidas-matrice400/`. The schematic v3 was rebuilt by Claude via the KiCad MCP tools at the user's request; the user refines layout/PCB in the KiCad GUI. There is no code-generation script in the repo.
- The **authoritative design spec and netlist** is [docs/esquematicos/esquematico.md](docs/esquematicos/esquematico.md): full component table, ASCII schematic, pin-by-pin netlist, oscillator config, and design notes. Treat that document as the source of truth for connectivity.
- References in the `.kicad_sch` match the doc 1:1. Any connectivity change must be made in **both** the doc and the schematic (use the KiCad MCP tools, never hand-written s-expressions), then re-run ERC. The PCB (`.kicad_pcb`) is still empty.

### System Blocks

| Block | Components | Rails |
|---|---|---|
| Power — LDO 3.3V | BT1 (LiPo 1S) → SW1 → MCP1700 (U3), C6 1µF in / C2 10µF out | 3.3V for PIC + IMU |
| Power — Boost 5V | MT3608 (U4), L1 22µH, Schottky D4 SS34, R8 750kΩ / R9 100kΩ, C5 22µF in / C4 100µF out | 5V for solenoid + buzzer |
| MCU | PIC18LF25K22 (U1), 28-SPDIP, internal oscillator 64 MHz | 3.3V |
| IMU | GY-521 module (MPU-6050) on header J3, I2C @ RC3/RC4, INT → RB0 | 3.3V |
| Deployment actuator | Solenoid 5V (SOL1 on J4) driven by AO3400A MOSFET (Q1), flyback D1 (1N4007) | 5V |
| Parachute mechanism | Spring tube; mechanical latch holds unpowered, solenoid **pulse (~300 ms) releases** | mechanical |
| Indicators | LED1 green (RA0), LED2 red (RA1), Buzzer via 2N3904 Q2 (RC0) | — |
| Debug/Programming | J1 = UART (RC6/RC7), J2 = ICSP PICKit (RB6/RB7) | — |

### Parachute Canopy
- Type: circular **Ripstop Nylon 30D sil-nylon**, **1.2–1.3 m diameter** (Cd ≈ 0.75, descent ≤6 m/s at 11 kg MTOW)
- Sizing formula: `Area = (2·m·g) / (Cd·ρ·v²)` → ~0.82 m² minimum → 1.2 m with safety margin
- Mechanism: **spring tube** — latch holds with solenoid OFF; a ~300 ms solenoid pulse pulls the pin and the spring ejects the parachute (~50–100 ms). Reset/brown-out never deploys (R2 keeps Q1 off).
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

- **Source of truth**: [docs/esquematicos/esquematico.md](docs/esquematicos/esquematico.md) (component table, net table, design notes). Keep that doc, the `.kicad_sch` and this file consistent.
- Edit the schematic only through the KiCad MCP tools (labels-based wiring), and run ERC afterwards (`kicad-cli` is at `C:/Program Files/KiCad/10.0/bin/kicad-cli.exe`).
- **5V output voltage**: set by `Vout = 0.6 × (1 + R8/R9)` = 0.6 × 8.5 = **5.1V**

## Firmware (not yet implemented)
Firmware will target PIC18LF25K22, compiled with MPLAB XC8. Source goes in `firmware/src/`, libraries in `firmware/lib/`.
