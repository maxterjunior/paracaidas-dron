# Esquemático — Sistema Paracaídas para DJI Matrice 400

Revisión **v4** (todo SMD, componentes en stock en JLCPCB). Archivos KiCad en `hardware/paracaidas-matrice400/`:
`paracaidas-matrice400.kicad_sch` (cableado), `.kicad_pcb` (ruteado), biblioteca propia `paracaidas.kicad_sym` / `paracaidas.pretty`.
Las referencias de este documento coinciden 1:1 con las del esquemático y el PCB. Cada símbolo lleva el campo `LCSC`.

## Componentes

Stock consultado en JLCPCB el 2026-11-09. **B** = básico (sin cargo de carga), **E** = extendido (cargo por referencia en el montaje).

| Ref | Componente | Descripción | Footprint | LCSC | Tipo |
|-----|------------|-------------|-----------|------|------|
| U1 | PIC18F25K22-I/SS | MCU principal | SSOP-28 5.3 mm | C51680 | E |
| U2 | BMI088 | IMU 6 ejes (acelerómetro ±24 g + giróscopo), diseñada para drones | LGA-16 4.5×3 mm | C194919 | E |
| U3 | XC6206P332MR | LDO 3.3V 200 mA | SOT-23-3 | C5446 | B |
| U4 | MT3608 | Boost VBAT → 5V | SOT-23-6 | C84817 | E |
| Q1 | AO3400A | MOSFET N, driver del solenoide (48 mΩ @ Vgs 2.5V) | SOT-23 | C20917 | B |
| Q2 | MMBT3904 | NPN, driver del buzzer | SOT-23 | C20526 | B |
| Q3 | AO3401A | MOSFET P, interruptor de carga de la batería | SOT-23 | C15127 | B |
| D1 | SS34 | Flyback del solenoide | SMA | C8678 | B |
| D4 | SS34 | Schottky de salida del MT3608 | SMA | C8678 | B |
| L1 | FXL0630-220-M | 22µH, Isat 3A, blindado | 7×6.6 mm | C177244 | E |
| BT1 | S2B-PH-SM4-TB | Conector batería LiPo 1S (JST-PH SMD) | JST PH 2P SMD | C295747 | E |
| J4 | S2B-PH-SM4-TB | Conector del solenoide SOL1 (JST-PH SMD) | JST PH 2P SMD | C295747 | E |
| SW1 | MSK12C02 | Interruptor deslizante SPDT (solo maneja la puerta de Q3) | SMD 8×2.8 mm | C431540 | E |
| SW2 | TS-1187A-B-A-B | Pulsador reset MCLR | SMD 5.1×5.1 mm | C318884 | B |
| BZ1 | MLT-9650 | Buzzer activo 5V 30 mA | SMD 9.6×9.6 mm (propio) | C96127 | E |
| LED1 | KT-0603G | LED verde | 0603 | C12624 | E |
| LED2 | KT-0603R | LED rojo | 0603 | C2286 | B |
| J1 | Header 1x03 SMD 2.54 | UART debug (GND, TX, RX) — **montaje manual** | PinHeader 1x03 SMD | — | — |
| J2 | Header 1x06 SMD 2.54 | ICSP PICkit — **montaje manual** | PinHeader 1x06 SMD | — | — |
| R1, R2 | 10kΩ | Pull-up MCLR / pull-down puerta Q1 | 0603 | C25804 | B |
| R3 | 100Ω | LED1 (Vf verde ≈ 2.8–3.1V) | 0603 | C22775 | B |
| R4 | 330Ω | LED2 | 0603 | C23138 | B |
| R5, R6 | 4.7kΩ | Pull-ups I2C | 0603 | C23162 | B |
| R7 | 1kΩ | Base Q2 | 0603 | C21190 | B |
| R8 | 750kΩ 1% | Feedback MT3608, parte alta | 0603 | C23240 | E (preferido) |
| R9 | 100kΩ 1% | Feedback MT3608, parte baja | 0603 | C25803 | B |
| R10 | 100kΩ | Pull-up puerta Q3 (apagado por defecto) | 0603 | C25803 | B |
| R11, R12 | 100kΩ | Divisor de medición de batería (VBAT/2) | 0603 | C25803 | B |
| C1, C3, C8, C9 | 100nF | Desacople PIC / BMI088 VDD / filtro ADC / BMI088 VDDIO | 0603 | C14663 | B |
| C2 | 10µF | Salida LDO | 0805 | C15850 | B |
| C4 | 22µF | Salida MT3608 (cerámico, junto a D4) | 0805 | C45783 | B |
| C5 | 22µF | Entrada MT3608 | 0805 | C45783 | B |
| C6 | 1µF | Entrada LDO | 0603 | C15849 | B |
| C7 | 220µF 10V | Bulk de +5V para el pulso del solenoide | Electrolítico SMD 6.3×5.4 | C72494 | E |

> **PIC18F25K22 en lugar de PIC18LF25K22:** el LF en encapsulado SMD no tiene stock en JLCPCB (1–2 unidades). El F es el mismo chip (mismo pinout, periféricos y firmware) con rango 2.3–5.5V; a 3.3V funciona igual. Diferencias: VDD mínimo 2.3V (vs 1.8V) y consumo en reposo algo mayor. **Verificar en el datasheet** la frecuencia máxima a 3.3V antes de fijar el PLL a 64 MHz; si hace falta, usar 16–32 MHz (sobra para leer el IMU a 1 kHz).

---

## Diagrama de Bloques

```
                 ┌────────────────────────────────────────────────────────────┐
  ┌─────────┐    │  BAT+   ┌──────────────────┐ VBAT ┌───────────────────────┐ │  5V RAIL
  │ LiPo 1S │────┼────────►│ Q3 AO3401A (P)   │──┬──►│ MT3608 + L1 + D4      │─┼─► J4 SOL1 / BZ1 / C7 220µF
  │ 1000mAh │    │         │ puerta ← SW1     │  │   │ R8 750k / R9 100k     │ │
  └─────────┘    │         └──────────────────┘  │   └───────────────────────┘ │  3.3V RAIL
                 │                               ├──►│ XC6206 LDO 3.3V       │─┼─► U1 PIC18F25K22, U2 BMI088
                 │                               └──►  R11/R12 → RA0 (AN0)     │
                 └────────────────────────────────────────────────────────────┘
```

---

## Diagrama Esquemático ASCII

```
ALIMENTACIÓN

BT1+ ── BAT+ ──┬── Q3.S   AO3401A   Q3.D ──┬── VBAT
               │         Q3.G              │
              R10 100k    │                ├── C6 1µF ── GND
               └──────────┴── SW1.B        ├── U3 XC6206 VI    VO ──┬── +3V3
                     SW1.A ── GND  (ON)    │       GND              C2 10µF ── GND
                     SW1.C ── NC   (OFF)   ├── R11 100k ──┬── BAT_SENSE → RA0
                                           │              R12 100k + C8 100nF ── GND
                                           ├── C5 22µF ── GND
                                           ├── U4.IN, U4.EN
                                           └── L1 22µH ── SW_NODE ── D4 SS34 (A→K) ──┬── +5V
                                                          │                         ├── C4 22µF ── GND
                                                        U4.SW                       ├── C7 220µF ── GND
                                                                                    └── R8 750k ──┬── FB (U4.FB)
                                                                                                  R9 100k ── GND
BT1- ─────────────────────────────────────────────────────────────────────────── GND


MCU / IMU

           +3V3                         U1 PIC18F25K22
            │
           R1 10k
            │
  J2.1 ─────┼──── MCLR (1)
  SW2 ──────┘ (a GND)
                         VDD (20) ── +3V3 + C1 100nF
                         VSS (8, 19) ── GND
   BAT:  RA0 (2)  ── BAT_SENSE (VBAT/2)
   LED1: RA1 (3)  ── R3 100Ω ── LED1 verde ── GND
   LED2: RA2 (4)  ── R4 330Ω ── LED2 rojo  ── GND
   BZ:   RC0 (11) ── R7 1kΩ ── Base Q2
   SOL:  RC2 (13) ── Gate Q1 (+ R2 10k a GND)
   SCL:  RC3 (14) ── U2.SCL (pull-up R5 4.7k)
   SDA:  RC4 (15) ── U2.SDA (pull-up R6 4.7k)
   TX:   RC6 (17) ── J1.2
   RX:   RC7 (18) ── J1.3
   GYR:  RB0 (21) ── U2.INT3 (giróscopo)
   ACC:  RB1 (22) ── U2.INT1 (acelerómetro)
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
| BAT+ | BT1(+), Q3.S, R10 |
| (puerta Q3) | Q3.G, R10, SW1.B (común) |
| VBAT | Q3.D, U3.VI(3), C6, U4.IN(5), U4.EN(4), C5, L1.1, R11 |
| SW_NODE | U4.SW(1), L1.2, D4.A |
| +5V | D4.K, C4, C7(+), R8, D1.K, J4.1, BZ1(+) |
| FB | U4.FB(3), R8, R9 |
| +3V3 | U3.VO(2), C2, U1.VDD(20), C1, R1, J2.2, U2.VDD(3), U2.VDDIO(11), U2.CSB1(14), U2.CSB2(5), U2.PS(7), C3, C9, R5, R6 |
| GND | BT1(−), SW1.A, U3.GND, U4.GND, C1–C9, R2, R9, R12, U1.VSS(8,19), SW2.2, LED1.K, LED2.K, Q1.S, Q2.E, J1.1, J2.3, U2.GNDA(2,4), U2.GNDIO(6), U2.SDO1(15), U2.SDO2(10) |
| BAT_SENSE | U1.RA0(2), R11, R12, C8 |
| MCLR | U1.1, R1, SW2.1, J2.1 |
| LED_G | U1.RA1(3) – R3 – LED1.A |
| LED_R | U1.RA2(4) – R4 – LED2.A |
| BZ_CTRL | U1.RC0(11) – R7 – Q2.B |
| (BZ1−) | Q2.C, BZ1(−) |
| SOL_GATE | U1.RC2(13), Q1.G, R2 |
| SOL_DRV | Q1.D, D1.A, J4.2 |
| SCL | U1.RC3(14), U2.SCK/SCL(8), R5 |
| SDA | U1.RC4(15), U2.SDI/SDA(9), R6 |
| GYR_INT | U1.RB0(21), U2.INT3(12) |
| ACC_INT | U1.RB1(22), U2.INT1(16) |
| UART_TX | U1.RC6(17), J1.2 (→ RX del adaptador USB-UART) |
| UART_RX | U1.RC7(18), J1.3 (← TX del adaptador USB-UART) |
| PGC | U1.RB6(27), J2.5 |
| PGD | U1.RB7(28), J2.4 |

Sin conectar (no-connect): U1 pines 5–7, 9, 10, 12, 16, 23–26 · U4.NC(6) · J2.6 · U2.INT2(1) · U2.INT4(13) · SW1.C.
Pines libres útiles: **RB2/INT2** (p. ej. disparo manual por radio), RB3–RB5, RA3–RA5, RC1, RC5.

### U2 — BMI088 en modo I2C

| Pin | Señal | Conexión |
|-----|-------|----------|
| 7 | PS | +3V3 → modo I2C |
| 14, 5 | CSB1, CSB2 | +3V3 |
| 15 | SDO1 | GND → acelerómetro en **0x18** |
| 10 | SDO2 | GND → giróscopo en **0x68** |
| 8, 9 | SCL, SDA | RC3, RC4 (pull-ups R5/R6 4.7k) |
| 16 | INT1 (acel.) | RB1 |
| 12 | INT3 (giro) | RB0/INT0 |
| 3, 11 | VDD, VDDIO | +3V3, con C3 y C9 100nF |
| 2, 4, 6 | GNDA, GNDIO | GND |

> El símbolo es `paracaidas:BMI088_I2C`: copia del símbolo de KiCad con SDO1/SDO2 como entradas (en I2C seleccionan la dirección), para que el ERC acepte llevarlos a GND.
> El BMI088 reemplaza al módulo GY-521 (MPU-6050, discontinuado y a ~US$21 en JLCPCB). Rango ±24 g y filtros pensados para la vibración de motores. El firmware (aún no escrito) debe usar sus registros, no los del MPU-6050.

### J2 — ICSP (pinout PICkit)

| Pin | Señal |
|-----|-------|
| 1 | MCLR/VPP |
| 2 | VDD (+3V3) |
| 3 | GND |
| 4 | PGD (RB7) |
| 5 | PGC (RB6) |
| 6 | NC (PGM/LVP, no usado) |

> J1 y J2 son headers SMD de 2.54 mm de montaje manual (no tienen LCSC y no van en el BOM de JLCPCB). También se puede programar apoyando un adaptador de pogo pins sobre los pads.

### Q3 — Interruptor de carga (AO3401A)

SW1 (MSK12C02, 50 mA) no soporta la corriente del solenoide, así que solo maneja la puerta de un MOSFET P:
- **SW1 en ON (A):** puerta a GND → Q3 conduce (85 mΩ @ 2.5V, 4A) → VBAT activo.
- **SW1 en OFF (C):** R10 lleva la puerta a BAT+ → Q3 abierto. El diodo interno de Q3 queda en inversa y no hay fuga hacia VBAT.

### MT3608 — Boost VBAT → 5V

Topología boost estándar: **L1 entre VBAT y SW; Schottky D4 de SW (ánodo) a +5V (cátodo)**. El MT3608 no es síncrono: sin D4 no hay salida.

> Verificación: Vout = 0.6 × (1 + R8/R9) = 0.6 × (1 + 750/100) = **5.1V** ✅
> El MT3608 limita la corriente del switch a ~2A: con la batería a 3.5V entrega como máximo ≈ 2 A × 3.5 V × 0.85 / 5 V ≈ **1.2 A** a 5V. Elegir un solenoide de ≤ 1 A, o tener en cuenta que C7 (220µF) solo cubre el arranque del pulso.

### Q1 — AO3400A (driver solenoide)

| Terminal | Conectado a |
|---|---|
| Gate | RC2 + R2 10kΩ a GND (apagado durante el reset/arranque) |
| Drain | SOL_DRV (J4.2 + ánodo D1) |
| Source | GND |

---

## Lógica de Despliegue

**Solenoide normalmente desenergizado; un pulso libera el paracaídas.**

- En vuelo el solenoide **no consume** (el pestillo mecánico retiene el tapón del tubo por sí solo, p. ej. pin de un solenoide push-pull con muelle de retorno que traba el tapón).
- Para desplegar, el PIC pone RC2 en alto durante **~300 ms**: el solenoide retira el pin y el muelle expulsa el paracaídas.
- Arranque, reset del PIC o corte de energía → R2 mantiene Q1 apagado → **no hay despliegue accidental**.

---

## Configuración Oscilador PIC

Oscilador interno **HFINTOSC 16 MHz + PLL x4 = 64 MHz**. Sin cristal externo.

```c
// MPLAB XC8 — Configuration bits PIC18F25K22
#pragma config FOSC    = INTIO67   // Oscilador interno, RA6/RA7 como GPIO
#pragma config PLLCFG  = ON        // PLL x4 → 64 MHz (verificar Fmax a 3.3V en el datasheet)
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
2. **I2C Fast Mode** — pull-ups R5/R6 4.7kΩ a 3.3V, correctas para 400 kHz
3. **Flyback D1** — obligatorio, protege Q1 del pico inductivo del solenoide al apagar (SS34, mismo componente que D4)
4. **Q1 AO3400A** — logic-level especificado a 2.5V; el pin del PIC a 3.3V lo enciende completamente
5. **MT3608** — L1 FXL0630 22µH con Isat 3A; D4 SS34 3A; C5 22µF a la entrada; C4 22µF junto a D4 y C7 220µF de reserva para el pulso
6. **XC6206** — dropout ~250 mV a 100 mA: con VBAT < 3.55V la salida baja de 3.3V (el PIC18F funciona hasta 2.3V y el BMI088 hasta 2.4V)
7. **Medición de batería** — RA0/AN0 lee VBAT/2 (2.1V a 4.2V). Como VDD también cae con la batería baja, medir el FVR interno (1.024V) para calcular VDD real y corregir la lectura
8. **ICSP** — RB6/RB7 reservados para PICkit, no usar en la aplicación final
9. **SW2** — reset para desarrollo
10. **BZ1** — buzzer activo: suena con RC0 en alto, no necesita PWM

### PCB

- 2 capas, **60 × 40 mm** (antes 90 × 62 mm: −57% de área), 4 agujeros M3 en las esquinas, todos los componentes en la cara superior.
- Clases de red: **Default** pista 0.2 mm / separación 0.15 mm / vía 0.6-0.3 mm (necesario para el SSOP-28 de 0.65 mm y el LGA de 0.5 mm); **Power** (VBAT, +5V, SW_NODE, SOL_DRV, BAT+) pista 0.6 mm / separación 0.2 mm / vía 0.8-0.4 mm. GND va por plano de cobre en ambas caras.
- **BMI088:** el footprint de Bosch prohíbe cobre en F.Cu bajo el sensor. Los pines +3V3 y GND (intercalados a 0.5 mm) salen a vías escalonadas de 0.5/0.25 mm con pistas de 0.15 mm, y el +3V3 se une por B.Cu. Estos tramos están bloqueados.
- Conectores BT1 (izquierda) y J4 (derecha) con la entrada del cable hacia el borde; SW1 en el borde izquierdo; J2 ICSP en el borde superior; J1 UART en el borde derecho.
- Estado: **ruteado** con Freerouting 2.1 + fanout manual del BMI088 (27 vías en total, 10 del fanout), plano de GND en F.Cu y B.Cu. DRC: 0 violaciones, 0 conexiones pendientes, 0 problemas de paridad con el esquemático. Si se mueven componentes, rellenar zonas (B) y repetir el DRC.

### Fabricación en JLCPCB

Archivos en `hardware/paracaidas-matrice400/jlcpcb/`:
- `gerbers/` — Gerber + taladros Excellon (comprimir la carpeta en un .zip para subirla).
- `bom_jlcpcb.csv` — BOM con columna `LCSC Part #` (29 líneas, 38 componentes).
- `cpl_jlcpcb.csv` — posiciones para el montaje (cara superior).

> En el visor de JLCPCB **revisar la rotación** de los componentes polarizados o asimétricos (U1, U2, U3, U4, Q1–Q3, D1, D4, C7, LEDs, BZ1, BT1, J4): KiCad y JLC no siempre usan el mismo cero de rotación. J1/J2 se sueldan a mano.
> Hay 9 referencias extendidas con cargo de carga en el montaje (U1, U2, U4, L1, BT1/J4, SW1, BZ1, LED1, C7) más R8, extendida "preferida" (sin cargo); el resto son básicas.

### Pendientes / mejoras futuras

**Críticas (antes de volar):**

- **Paracaídas Ø3.5 m** (no Ø1.2 m): ver el dimensionamiento corregido en [modulos.md](../investigacion/modulos.md#dimensionamiento). Rediseñar el tubo de muelle para ~1.5–2 L de volumen plegado y ~0.6 kg de expulsión.
- **Criterio de disparo con giroscopio:** con un motor fallado el dron gira o se voltea y el acelerómetro no ve 0 g. Disparar también por inclinación > ~60–70° o velocidad angular anormal sostenida (el BMI088 ya da el giróscopo).
- **Disparo manual por radio:** entrada adicional (p. ej. salida PWM de un receptor RC a RB2/INT2, libre) como respaldo.
- **Lógica de armado:** no disparar en tierra ni durante el despegue (armar tras superar una altura o tiempo de vuelo; requiere barómetro o señal externa).
- **Caída de tensión durante el pulso del solenoide:** el MT3608 entrega ~1.2 A máx. a 5V. Si el solenoide pide más, la tensión cae y el PIC podría reiniciarse a mitad del pulso (R2 apaga Q1 y el seguro **no se libera**). C7 220µF ayuda en el arranque; probar con el solenoide real y batería a 3.5V. Si falla: solenoide de menor corriente, más capacidad en +5V, o batería 2S con buck.
- **Hélices:** el sistema no corta los motores; ubicar el tubo para expulsar hacia arriba, fuera del plano de las hélices. Evaluar una salida para cortar motores si el integrador lo permite.
- **Pruebas de caída** con masa equivalente (16 kg) antes de montar en el M400; medir altura mínima de despliegue (estimada 20–40 m).

**PCB:**

- Barniz conformal y fijación de la placa con los 4 M3 (con separadores antivibración si es posible).
- Revisar la rotación de componentes en el visor de JLCPCB antes de pedir.

**Otras:**

- Cargador LiPo (TP4056) o carga externa
- Firmware (aún no implementado) — usar los registros del BMI088

---

## Historial de Correcciones

### v3 → v4

| # | Cambio | Motivo |
|---|---|---|
| 1 | Todo SMD con stock JLCPCB y campo LCSC en cada símbolo | Montaje automático en JLCPCB |
| 2 | PCB 90×62 → **60×40 mm**, 4 agujeros M3 | Placa más compacta |
| 3 | GY-521 (J3) → **BMI088 (U2)** soldado en la placa, con pull-ups R5/R6 y desacoples C3/C9 | MPU-6050 discontinuado; módulo con header no es SMD y se afloja con vibración |
| 4 | PIC18LF25K22-I/SP → **PIC18F25K22-I/SS** (SSOP-28) | El LF SMD no tiene stock |
| 5 | MCP1700 → **XC6206P332MR** | Componente básico de JLCPCB, mismo pinout SOT-23 |
| 6 | SW1 directo en la línea de batería → **Q3 AO3401A + R10** con SW1 en la puerta | Los interruptores deslizantes SMD soportan ~50 mA |
| 7 | **Medición de batería** R11/R12 + C8 a RA0/AN0 | Alarma de batería baja |
| 8 | C4 100µF THT → C4 22µF cerámico junto a D4 + **C7 220µF** electrolítico SMD | Lazo del boost corto y reserva para el pulso del solenoide |
| 9 | D1 1N4007 → SS34; Q2 2N3904 → MMBT3904; LEDs 3 mm → 0603; buzzer THT → MLT-9650 SMD | SMD |
| 10 | LEDs pasan de RA0/RA1 a RA1/RA2; INT del IMU: acelerómetro → RB1, giróscopo → RB0 | RA0 queda para la medición de batería |
| 11 | Clase Default 0.3 → 0.2 mm; GND fuera de la clase Power (va por plano) | Paso fino del SSOP-28 y del LGA-16 |

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
