# Esquemático — Sistema Paracaídas para DJI Matrice 400

Revisión **v3**. Archivos KiCad: `hardware/paracaidas-matrice400/paracaidas-matrice400.kicad_sch` (cableado) y `.kicad_pcb`
(las referencias de este documento coinciden 1:1 con las del esquemático y el PCB).

## Componentes

| Ref | Componente | Descripción | Footprint |
|-----|------------|-------------|-----------|
| U1 | PIC18LF25K22-I/SP | MCU principal (28-pin SPDIP) | DIP-28 W7.62mm |
| J3 | Módulo GY-521 (MPU-6050) | IMU — detección caída libre, header 8 pines | PinHeader 1x08 2.54mm |
| U3 | MCP1700T-3302E/TT | LDO 3.3V, 250 mA | SOT-23 |
| U4 | MT3608 | Step-up boost VBAT → 5V | SOT-23-6 |
| Q1 | AO3400A | MOSFET N logic-level, driver solenoide (Rds(on) ≤ 48 mΩ @ Vgs 2.5V) | SOT-23 |
| Q2 | 2N3904 | NPN driver buzzer | TO-92 Inline Wide (paso 2.54) |
| D1 | 1N4007 | Flyback solenoide | DO-41 |
| D4 | SS34 | Schottky de salida del MT3608 (40V 3A) | SMA |
| BT1 | LiPo 1S 1000mAh | Batería 3.7V (conector JST-PH 2 pines) | JST PH 2P |
| J4 | Solenoide 5V (SOL1) | Conector del actuador de liberación | Bornera 2P 5.08mm |
| LED1 | LED verde 3mm | Indicador sistema activo / armado | LED THT 3mm |
| LED2 | LED rojo 3mm | Indicador despliegue / fallo | LED THT 3mm |
| BZ1 | Buzzer activo 5V | Alarma sonora | Buzzer 12x9.5mm |
| SW1 | Interruptor deslizante | Encendido sistema (VBAT) | — |
| SW2 | Pulsador NO | Reset MCLR (desarrollo) | Pulsador 6mm THT |
| J1 | Header 1x03 | UART debug (GND, TX, RX) | PinHeader 1x03 |
| J2 | Header 1x06 | ICSP PICkit (MCLR, VDD, GND, PGD, PGC, NC) | PinHeader 1x06 |
| L1 | 22µH ≥1.5A (shielded) | Inductor MT3608 | — |
| R1 | 10kΩ | Pull-up MCLR | 0805 / THT |
| R2 | 10kΩ | Pull-down gate Q1 | |
| R3 | 330Ω | LED1 | |
| R4 | 330Ω | LED2 | |
| R7 | 1kΩ | Base Q2 | |
| R8 | 750kΩ 1% | Feedback MT3608, parte alta (antes R_FB1) | |
| R9 | 100kΩ 1% | Feedback MT3608, parte baja (antes R_FB2) | |
| C1 | 100nF | Desacople VDD PIC (junto al pin 20) | |
| C2 | 10µF | Salida LDO 3.3V | |
| C3 | 100nF | Desacople VCC GY-521 (junto al header) | |
| C4 | 100µF 10V | Salida MT3608 5V (bulk para el pico del solenoide) | |
| C5 | 22µF 10V | Entrada MT3608 (cerámico X5R/X7R) | |
| C6 | 1µF | Entrada MCP1700 (requerido por datasheet) | |

---

## Diagrama de Bloques

```
                ┌──────────────────────────────────────────────────────────┐
  ┌─────────┐   │  VBAT   ┌───────────────────────────┐    5V RAIL         │
  │ LiPo 1S │─SW1─────┬──►│ MT3608 + L1 + D4 (SS34)   │──► J4 SOL1 / BZ1   │
  │ 1000mAh │   │     │   │ R8 750k / R9 100k → 5.1V  │                    │
  └─────────┘   │     │   └───────────────────────────┘                    │
                │     │   ┌───────────────────────────┐    3.3V RAIL       │
                │     └──►│ MCP1700 LDO 3.3V          │──► U1 PIC18LF25K22 │
                │         └───────────────────────────┘    J3 GY-521       │
                └──────────────────────────────────────────────────────────┘
```

---

## Diagrama Esquemático ASCII

```
ALIMENTACIÓN

BT1+ ──SW1──┬── VBAT ──────────────────────────────────────────────┐
            │                                                      │
           C6 1µF      U3 MCP1700                                  │
            │        VI ──┤      ├── VO ──┬── +3V3                 │
           GND           GND           C2 10µF                     │
                                         GND                       │
                                                                   │
   VBAT ──┬── C5 22µF ── GND                                       │
          ├── U4.IN                                                │
          ├── U4.EN                                                │
          └── L1 22µH ── SW_NODE ── D4 SS34 (A→K) ──┬── +5V        │
                         │                          ├── C4 100µF ── GND
                       U4.SW                        ├── R8 750k ──┬── FB (U4.FB)
                                                    │             R9 100k
                                                    │             GND
BT1- ───────────────────────────────────────────────────────── GND


MCU / IMU

           +3V3                         U1 PIC18LF25K22
            │
           R1 10k
            │
  J2.1 ─────┼──── MCLR (1)
  SW2 ──────┘ (a GND)
                         VDD (20) ── +3V3 + C1 100nF
                         VSS (8, 19) ── GND
   LED1: RA0 (2)  ── R3 330Ω ── LED1 verde ── GND
   LED2: RA1 (3)  ── R4 330Ω ── LED2 rojo  ── GND
   BZ:   RC0 (11) ── R7 1kΩ ── Base Q2
   SOL:  RC2 (13) ── Gate Q1 (+ R2 10k a GND)
   SCL:  RC3 (14) ── J3.3   (pull-up 4.7k en el GY-521)
   SDA:  RC4 (15) ── J3.4   (pull-up 4.7k en el GY-521)
   TX:   RC6 (17) ── J1.2
   RX:   RC7 (18) ── J1.3
   INT:  RB0 (21) ── J3.8
   PGC:  RB6 (27) ── J2.5
   PGD:  RB7 (28) ── J2.4


ACTUADORES

  +5V ── J4.1 (SOL1+)            +5V ── BZ1(+)
         J4.2 (SOL1−) ── SOL_DRV        BZ1(−) ── Colector Q2
  D1: Cátodo +5V / Ánodo SOL_DRV        Emisor Q2 ── GND
  Q1 AO3400A: D = SOL_DRV, S = GND, G = SOL_GATE
```

---

## Tabla de Conexiones (Netlist)

| Net | Conexiones |
|-----|------------|
| BAT+ | BT1(+), SW1.A |
| VBAT | SW1.B, U3.VI(3), C6, U4.IN(5), U4.EN(4), C5, L1.1 |
| SW_NODE | U4.SW(1), L1.2, D4.A |
| +5V | D4.K, C4, R8, D1.K, J4.1, BZ1(+) |
| FB | U4.FB(3), R8, R9 |
| +3V3 | U3.VO(2), C2, U1.VDD(20), C1, R1, J3.1 (VCC), C3, J2.2 |
| GND | BT1(−), U3.GND, U4.GND, C1–C6, R2, R9, U1.VSS(8,19), SW2.2, LED1.K, LED2.K, Q1.S, Q2.E, J1.1, J2.3, J3.2, J3.7 (AD0) |
| MCLR | U1.1, R1, SW2.1, J2.1 |
| LED_G / LED_G_A | U1.RA0(2) – R3 – LED1.A |
| LED_R / LED_R_A | U1.RA1(3) – R4 – LED2.A |
| BZ_CTRL / BZ_BASE | U1.RC0(11) – R7 – Q2.B |
| BZ_DRV | Q2.C, BZ1(−) |
| SOL_GATE | U1.RC2(13), Q1.G, R2 |
| SOL_DRV | Q1.D, D1.A, J4.2 |
| SCL | U1.RC3(14), J3.3 |
| SDA | U1.RC4(15), J3.4 |
| IMU_INT | U1.RB0(21), J3.8 |
| UART_TX | U1.RC6(17), J1.2 (→ RX del adaptador USB-UART) |
| UART_RX | U1.RC7(18), J1.3 (← TX del adaptador USB-UART) |
| PGC | U1.RB6(27), J2.5 |
| PGD | U1.RB7(28), J2.4 |

Sin conectar (no-connect): U1 pines 4–7, 9, 10, 12, 16, 22–26 · U4.NC(6) · J2.6 · J3.5 (XDA) · J3.6 (XCL).

### J3 — Módulo GY-521 (orden del header)

| Pin | Señal | Conexión |
|-----|-------|----------|
| 1 | VCC | +3V3 + C3 100nF |
| 2 | GND | GND |
| 3 | SCL | RC3 |
| 4 | SDA | RC4 |
| 5 | XDA | NC |
| 6 | XCL | NC |
| 7 | AD0 | GND → dirección 0x68 |
| 8 | INT | RB0/INT0 |

> El GY-521 trae su propio LDO (662K) y pull-ups de 4.7kΩ en SCL/SDA, por eso la placa no lleva pull-ups propias (R5/R6 eliminadas). Alimentado con 3.3V el MPU-6050 queda en ~3.1V (rango válido 2.375–3.46V). Si se usa un módulo sin pull-ups, agregar 4.7kΩ a +3V3 en SCL y SDA.

### J2 — ICSP (pinout PICkit)

| Pin | Señal |
|-----|-------|
| 1 | MCLR/VPP |
| 2 | VDD (+3V3) |
| 3 | GND |
| 4 | PGD (RB7) |
| 5 | PGC (RB6) |
| 6 | NC (PGM/LVP, no usado) |

> Al programar con el PICkit, configurar VDD a 3.3V (el PIC18**LF** admite máx. 3.6V) o alimentar desde la batería con el PICkit sin suministrar tensión.

### MT3608 — Boost VBAT → 5V

Topología boost estándar: **L1 entre VBAT y SW; Schottky D4 de SW (ánodo) a +5V (cátodo)**. El MT3608 no es síncrono: sin D4 no hay salida.

> Verificación: Vout = 0.6 × (1 + R8/R9) = 0.6 × (1 + 750/100) = **5.1V** ✅

### Q1 — AO3400A (driver solenoide)

| Terminal | Conectado a |
|---|---|
| Gate | RC2 + R2 10kΩ a GND (apagado durante el reset/arranque) |
| Drain | SOL_DRV (J4.2 + ánodo D1) |
| Source | GND |

> Se reemplaza el IRLZ44N: su Rds(on) está especificada a Vgs = 4–5V, y con 3.3V en el gate no está garantizada la conducción plena. El AO3400A está especificado a Vgs = 2.5V (≤ 48 mΩ) y soporta 5.7A.

---

## Lógica de Despliegue (v3)

**Solenoide normalmente desenergizado; un pulso libera el paracaídas.**

- En vuelo el solenoide **no consume** (el pestillo mecánico retiene el tapón del tubo por sí solo, p. ej. pin de un solenoide push-pull con muelle de retorno que traba el tapón).
- Para desplegar, el PIC pone RC2 en alto durante **~300 ms**: el solenoide retira el pin y el muelle expulsa el paracaídas.
- Arranque, reset del PIC o corte de energía → R2 mantiene Q1 apagado → **no hay despliegue accidental**.
- Antes (v2) el solenoide debía estar energizado todo el vuelo (~3 W desde la LiPo de 1000 mAh, < 1 h de autonomía) y cualquier reset o caída de tensión desplegaba el paracaídas.

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
2. **I2C Fast Mode** — pull-ups 4.7kΩ del GY-521 a 3.3V, correctas para 400 kHz
3. **Flyback D1** — obligatorio, protege Q1 del pico inductivo del solenoide al apagar
4. **Q1 AO3400A** — logic-level especificado a 2.5V; el pin del PIC a 3.3V lo enciende completamente
5. **MT3608** — L1 22µH shielded con corriente de saturación ≥ 1.5A; D4 Schottky ≥ 2A; C5 22µF cerámico a la entrada (picos de corriente del solenoide)
6. **MCP1700** — requiere ≥ 1µF en la entrada (C6) y ≥ 1µF en la salida (C2 10µF). Dropout ~180 mV: con VBAT < 3.5V la salida baja de 3.3V (el PIC18LF funciona hasta 1.8V y el MPU-6050 hasta 2.375V)
7. **C1 y C3** — lo más cerca posible del pin VDD del PIC y del header del GY-521
8. **ICSP** — RB6/RB7 reservados para PICkit, no usar en la aplicación final
9. **SW2** — solo para desarrollo, puede omitirse en producción
10. **Dirección MPU-6050** — AD0 a GND = 0x68, a 3.3V = 0x69

### PCB

- 2 capas, 90 × 62 mm, 3 agujeros M3 (esquina sup. izq. ocupada por BT1).
- Clases de red: **Default** pista 0.3 mm / vía 0.8-0.4 mm; **Power** (GND, VBAT, +5V, SW_NODE, SOL_DRV, BAT+) pista 0.6 mm (el paso de 0.95 mm del SOT-23-6 del MT3608 no admite 0.8 mm) / vía 1.0-0.5 mm.
- El GY-521 se monta sobre J3 con header hembra; el área a la izquierda de J3 queda bajo el módulo (solo componentes bajos).
- Estado: **ruteado** con Freerouting 2.1 (2 vías, sin violaciones) y plano de GND en F.Cu y B.Cu con alivios térmicos. DRC: 0 violaciones, 0 conexiones pendientes, 0 problemas de paridad con el esquemático. Si se mueven componentes, rellenar zonas (B) y repetir el DRC.

### Pendientes / mejoras futuras

- Medición de tensión de batería (divisor a RA2/AN2) para alarma de batería baja
- Cargador LiPo (TP4056) o carga externa
- Criterio de disparo adicional con giroscopio (giro descontrolado tras fallo de motor) y lógica de armado (no disparar en tierra)

---

## Historial de Correcciones

### v2 → v3

| # | Problema | Corrección |
|---|---|---|
| 1 | MT3608 sin diodo de salida; L1 mal ubicado (`SW──L1──OUT`) | Topología boost correcta: L1 VBAT→SW, D4 SS34 SW→+5V |
| 2 | IRLZ44N no garantiza conducción plena con Vgs = 3.3V | Q1 → AO3400A (especificado a 2.5V) |
| 3 | Sin capacitor de entrada en el MCP1700 | C6 1µF |
| 4 | C5 100nF insuficiente en la entrada del MT3608 | C5 → 22µF |
| 5 | Solenoide energizado = retenido (consume todo el vuelo, despliega en reset/corte) | Solenoide normalmente apagado; pulso de ~300 ms libera |
| 6 | J1, J2, BT1, SOL1 sin definir en el esquemático | J1 UART 1x03, J2 ICSP 1x06, BT1 JST-PH, J4 SOL1 |
| 7 | GY-521 representado como chip QFN | J3 header 1x08 con el pinout del módulo |
| 8 | Esquemático KiCad con LEDs infrarrojos, PIC SOIC y SW2 DIP | LEDs visibles, PIC SPDIP, SW2 pulsador |
| 9 | R_FB1/R_FB2 con nombres distintos a KiCad | Renombrados R8/R9 |
| 10 | R5/R6 duplicaban las pull-ups del GY-521 | Eliminadas |
| 11 | Esquemático solo con etiquetas, sin cables | Esquemático cableado con símbolos de alimentación |

### v1 → v2

| # | Problema detectado | Corrección aplicada |
|---|---|---|
| 1 | BZ1 sin pin ni driver | Agregado Q2 (2N3904) + R7 (1kΩ) + pin RC0 |
| 2 | MT3608 sin valores de feedback | R_FB1=750kΩ, R_FB2=100kΩ, L1=22µH 1A, C5=100nF entrada |
| 3 | Sin botón de reset | Agregado SW2 (pulsador NO) en MCLR a GND |
| 4 | Nota IRLZ44N faltante | (Revisado en v3: la nota era incorrecta) |
