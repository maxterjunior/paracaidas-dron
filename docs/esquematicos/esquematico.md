# Esquemático — Sistema Paracaídas para DJI Matrice 400

## Componentes

| Ref | Componente | Descripción |
|-----|------------|-------------|
| U1 | PIC18LF25K22 | MCU principal (28-pin DIP) |
| U2 | MPU-6050 (GY-521) | IMU — detección caída libre |
| U3 | MCP1700-3302E | LDO regulador 3.3V |
| U4 | MT3608 | Step-up boost 3.7V → 5V |
| Q1 | IRLZ44N | MOSFET N-channel driver solenoid |
| Q2 | 2N3904 | NPN transistor driver buzzer |
| D1 | 1N4007 | Diodo flyback protección solenoid |
| BT1 | LiPo 1S 1000mAh | Batería 3.7V |
| SOL1 | Solenoid 5V | Actuador despliegue resorte |
| LED1 | LED verde | Indicador sistema activo |
| LED2 | LED rojo | Indicador despliegue/fallo |
| BZ1 | Buzzer activo 5V | Alarma sonora |
| SW1 | Switch ON/OFF | Encendido sistema |
| SW2 | Pulsador NO | Reset MCLR (desarrollo) |
| L1 | Inductor 22µH 1A | MT3608 step-up |
| R1 | 10kΩ | Pull-up MCLR |
| R2 | 10kΩ | Pull-down Gate Q1 |
| R3 | 330Ω | Limitador corriente LED1 |
| R4 | 330Ω | Limitador corriente LED2 |
| R5 | 4.7kΩ | Pull-up I2C SCL |
| R6 | 4.7kΩ | Pull-up I2C SDA |
| R7 | 1kΩ | Base Q2 (driver buzzer) |
| R_FB1 | 750kΩ | Feedback MT3608 (parte alta) |
| R_FB2 | 100kΩ | Feedback MT3608 (parte baja) |
| C1 | 100nF | Desacople VDD PIC (junto al pin 20) |
| C2 | 10µF | Filtro salida LDO 3.3V |
| C3 | 100nF | Desacople VCC MPU-6050 (junto al pin VCC) |
| C4 | 100µF | Filtro salida MT3608 5V |
| C5 | 100nF | Desacople entrada MT3608 |

---

## Diagrama de Bloques

```
                    ┌──────────────────────────────────────────────────┐
                    │              SISTEMA PARACAÍDAS                  │
                    │                                                  │
  ┌──────────┐      │  ┌──────────────────┐   ┌────────────────────┐   │
  │  LiPo 1S │──SW1─┼─►│ MT3608 Step-Up   │──►│     5V RAIL        │   │
  │ 3.7V     │      │  │ L1=22µH          │   │  SOL1 / BZ1        │   │
  │ 1000mAh  │      │  │ R_FB1=750kΩ      │   └────────────────────┘   │
  └──────────┘      │  │ R_FB2=100kΩ      │                            │
                    │  └──────────────────┘   ┌────────────────────┐   │
                    │       │                 │     3.3V RAIL      │   │
                    │  ┌────▼──────────┐  ───►│  PIC18LF25K22      │   │
                    │  │ MCP1700-3302E │      │  MPU-6050          │   │
                    │  │ LDO 3.3V      │      └────────────────────┘   │
                    │  └───────────────┘                               │
                    └──────────────────────────────────────────────────┘
```

---

## Diagrama Esquemático ASCII

```
                                        3.3V
                                         │
                              C2 10µF   C1 100nF
BT1+ ──SW1──┬────────────────────────────┤
            │                           MCP1700 (U3)
            │  C5 100nF                 IN──BT1+
            ├──┤├──GND                  OUT──3.3V
            │                           GND──GND
            │
            ├──[MT3608 U4]──────────────────► 5V
            │   IN   SW──L1(22µH)──OUT        │
            │   GND                    C4 100µF┤
            │   EN──IN+                       GND
            │   FB──R_FB1(750kΩ)──OUT
            │      └──R_FB2(100kΩ)──GND
            │
BT1- ───────┴──────────────────────────────── GND


                    ┌─────── R1 10kΩ ──── 3.3V
                    │
                    ├─────── SW2 ─────── GND  ← Reset (pulsador NO)
                    │
                    │MCLR(1)┐
                             │  PIC18LF25K22 (U1)
        3.3V──C1(100nF)──GND─┤VDD(20)
                    GND───── ┤VSS(8)(19)
                             │
        MPU-6050 (U2)        │
        ┌──────────┐         │
3.3V──►─┤VCC       │  R5     │
       C3┤GND  SCL─┼──┤├─3.3V┤RC3/SCL1(14)  ← I2C clock
  100nF  │     SDA─┼──┤├─3.3V┤RC4/SDA1(15)  ← I2C data
         │     INT─┼─────────┤RB0/INT0(21)  ← Interrupción HW
         │     AD0─┼──GND    │         R6
         └──────────┘    3.3V┤          └──┤├── SDA pull-up
                             │
                             ┤RC2/CCP1(13) ──────────────────────► Gate Q1
                             │                                        │
                             ┤RA0(2) ── R3 330Ω ── LED1(verde) ── GND
                             │
                             ┤RA1(3) ── R4 330Ω ── LED2(rojo)  ── GND
                             │
                             ┤RC0(11) ── R7 1kΩ ── Base Q2(2N3904)
                             │                        │
                             ┤RC6/TX1(17) ─► USB-UART RX  (debug)
                             ┤RC7/RX1(18) ◄─ USB-UART TX  (debug)
                             │
                             ┤RB6/PGC(27) ─► PICKit PGC
                             ┤RB7/PGD(28) ─► PICKit PGD
                             └─────────────────────────


MOSFET Q1 (IRLZ44N):          NPN Q2 (2N3904) — Driver Buzzer:
  Gate  ─── RC2(PIC)             Base      ── R7 ── RC0(PIC)
  Gate  ─── R2(10kΩ) ─── GND    Emitter   ── GND
  Drain ─── SOL1(-)              Collector ── BZ1(-) ── BZ1(+) ── 5V
  Drain ─── Ánodo D1
  Source─── GND

SOL1 (+) ──► 5V
SOL1 (-) ──► Drain Q1
D1: Cátodo → 5V  /  Ánodo → Drain Q1   ← flyback
```

---

## Tabla de Conexiones (Netlist)

### PIC18LF25K22 — Asignación de Pines

| Pin | Nombre | Función | Conectado a |
|-----|--------|---------|-------------|
| 1 | MCLR | Reset | R1 10kΩ → 3.3V + SW2 → GND |
| 2 | RA0 | GPIO OUT | R3 330Ω → LED1 verde → GND |
| 3 | RA1 | GPIO OUT | R4 330Ω → LED2 rojo → GND |
| 8 | VSS | GND | GND |
| 9 | RA7/OSC1 | NC | Oscilador interno activo |
| 10 | RA6/OSC2 | NC | Oscilador interno activo |
| 11 | RC0 | GPIO OUT | R7 1kΩ → Base Q2 (buzzer) |
| 13 | RC2 | GPIO OUT | Gate Q1 IRLZ44N (solenoid) |
| 14 | RC3/SCL1 | I2C Clock | MPU-6050 SCL + R5 4.7kΩ → 3.3V |
| 15 | RC4/SDA1 | I2C Data | MPU-6050 SDA + R6 4.7kΩ → 3.3V |
| 17 | RC6/TX1 | UART TX | USB-UART RX (debug) |
| 18 | RC7/RX1 | UART RX | USB-UART TX (debug) |
| 19 | VSS | GND | GND |
| 20 | VDD | Alimentación | 3.3V + C1 100nF → GND |
| 21 | RB0/INT0 | INT externo | MPU-6050 INT (data ready) |
| 27 | RB6/PGC | ICSP Clock | PICKit PGC |
| 28 | RB7/PGD | ICSP Data | PICKit PGD |

### MPU-6050 (GY-521)

| Pin | Conectado a |
|-----|-------------|
| VCC | 3.3V + C3 100nF directo al pin |
| GND | GND |
| SCL | RC3/SCL1 PIC pin 14 |
| SDA | RC4/SDA1 PIC pin 15 |
| INT | RB0/INT0 PIC pin 21 |
| AD0 | GND → dirección I2C 0x68 |
| XCL | NC |
| XDA | NC |

### MT3608 (Step-Up 3.7V → 5V)

| Pin/Elemento | Conectado a | Valor |
|---|---|---|
| IN+ | BT1+ post SW1 + C5 100nF a GND | — |
| GND | GND | — |
| SW | L1 → OUT | L1 = 22µH 1A |
| OUT | 5V rail + C4 100µF a GND | — |
| EN | IN+ (siempre habilitado) | — |
| FB | Divisor R_FB1/R_FB2 | Vout = 0.6×(1 + R_FB1/R_FB2) = 5V |
| R_FB1 | FB → OUT | 750kΩ |
| R_FB2 | FB → GND | 100kΩ |

> Verificación: 0.6 × (1 + 750/100) = 0.6 × 8.5 = **5.1V** ✅

### MOSFET Q1 — IRLZ44N

| Terminal | Conectado a | Nota |
|---|---|---|
| Gate | RC2 PIC pin 13 + R2 10kΩ a GND | Vgs(th) max = 2V → 3.3V lo satura completamente ✅ |
| Drain | SOL1(-) + Ánodo D1 | — |
| Source | GND | — |

### NPN Q2 — 2N3904 (Driver Buzzer)

| Terminal | Conectado a |
|---|---|
| Base | R7 1kΩ → RC0 PIC pin 11 |
| Emitter | GND |
| Collector | BZ1(-) |

BZ1(+) → 5V / BZ1(-) → Collector Q2

### Diodo D1 — 1N4007 (Flyback)

| Terminal | Conectado a |
|---|---|
| Cátodo | 5V rail |
| Ánodo | Drain Q1 |

---

## Configuración Oscilador PIC

Oscilador interno **HFINTOSC 16 MHz + PLL x4 = 64 MHz**. Sin cristal externo.

```c
// MPLAB XC8 — Configuration bits PIC18LF25K22
#pragma config FOSC    = INTIO67   // Oscilador interno, RA6/RA7 como GPIO
#pragma config PLLCFG  = ON        // PLL x4 → 64 MHz
#pragma config PRICLKEN = ON
#pragma config FCMEN   = OFF
#pragma config IESO    = OFF
#pragma config PWRTEN  = ON        // Power-up timer ON
#pragma config BOREN   = ON        // Brown-out reset ON
#pragma config WDTEN   = OFF       // Watchdog OFF (habilitar en producción)
#pragma config MCLRE   = EXTMCLR  // MCLR pin habilitado (SW2 reset)
#pragma config LVP     = OFF       // Low voltage programming OFF
```

---

## Notas de Diseño

1. **Oscilador interno** — sin cristal externo, RA6/RA7 quedan libres como GPIO
2. **I2C Fast Mode** — pull-ups 4.7kΩ a 3.3V correctos para 400 kHz
3. **Flyback D1** — obligatorio, protege Q1 del pico inductivo del solenoid al apagar
4. **IRLZ44N + 3.3V** — compatible: Vgs(th) máx = 2V, con 3.3V en gate queda en saturación total
5. **MT3608 inductor** — usar 22µH con rating de corriente ≥1A, tipo shielded preferible
6. **C1 y C3** — colocar lo más cerca posible de los pines VDD del PIC y VCC del MPU-6050
7. **ICSP** — RB6/RB7 reservados para PICKit, no usar en aplicación final
8. **SW2** — solo para desarrollo, puede omitirse en versión de producción
9. **Dirección MPU-6050** — AD0 a GND = 0x68, a 3.3V = 0x69 (si se necesitan dos IMUs)

---

## Correcciones v1 → v2

| # | Problema detectado | Corrección aplicada |
|---|---|---|
| 1 | BZ1 sin pin ni driver | Agregado Q2 (2N3904) + R7 (1kΩ) + pin RC0 |
| 2 | MT3608 sin valores de feedback | R_FB1=750kΩ, R_FB2=100kΩ, L1=22µH 1A, C5=100nF entrada |
| 3 | Sin botón de reset | Agregado SW2 (pulsador NO) en MCLR a GND |
| 4 | Nota IRLZ44N faltante | Confirmado: Vgs(th) 2V → 3.3V satura completamente |
