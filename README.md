# Paracaídas de Emergencia para DJI Matrice 400

Sistema autónomo que detecta la caída libre del dron con una IMU y despliega un paracaídas mediante un tubo de muelle. Funciona **independiente del flight controller**: tiene su propia batería, su MCU y su sensor.

| | |
|---|---|
| **Dron objetivo** | DJI Matrice 400: 9.74 kg con baterías, MTOW 15.8 kg |
| **MCU** | PIC18F25K22 (SSOP-28) con oscilador interno + PLL, 3.3 V |
| **IMU** | BMI088 (acelerómetro ±24 g + giróscopo) soldado en la placa, I²C: acel. 0x18 / giro 0x68 |
| **Actuador** | Solenoide 5 V que libera el pestillo de un tubo de muelle |
| **Paracaídas** | Circular, ripstop nylon 30D, **Ø3.5 m**, 12 gajos, descenso ≤ 6 m/s a 16 kg |
| **Alimentación** | LiPo 1S 1000 mAh → interruptor MOSFET → LDO 3.3 V (XC6206) + boost 5.1 V (MT3608) |

## Estado

| Área | Estado |
|---|---|
| Investigación de módulos y paracaídas | ✅ Completa |
| Esquemático (KiCad 10, rev. v4, todo SMD) | ✅ Cableado, ERC limpio, campo LCSC en cada componente |
| PCB 2 capas 60 × 40 mm | ✅ Ruteado, planos de GND, DRC limpio, archivos JLCPCB listos |
| Firmware (MPLAB XC8) | ⏳ Pendiente |
| Mecánica (tubo de muelle, canopia) | ⏳ Pendiente |

## Cómo funciona

```
LiPo 1S ─Q3/SW1─┬─► XC6206 3.3V ─► PIC18F25K22 ◄─I²C─► BMI088
                │                       │
                └─► MT3608 5.1V ────────┼─► AO3400A ─► Solenoide ─► pestillo ─► muelle ─► paracaídas
                                        └─► LEDs / buzzer
```

1. El PIC lee el acelerómetro y filtra la vibración de los motores con un IIR de primer orden (solo aritmética entera, el PIC18 no tiene FPU).
2. Si `|a| = √(ax² + ay² + az²) < 0.3 g` durante más de 100 ms, considera que el dron está en caída libre.
3. Da un pulso de ~300 ms al solenoide, que retira el pin del pestillo. El muelle expulsa el paracaídas en ~50–100 ms.
4. El pestillo retiene **sin energía**. Un reset o un brown-out nunca despliega el paracaídas (R2 mantiene Q1 apagado).

### Limitaciones conocidas

- **Altura mínima de despliegue: ~20–40 m.** Entre detección, expulsión e inflado se pierden 1.5–2.5 s.
- **La caída libre sola no basta.** Si falla un motor, el dron gira con los demás empujando y el acelerómetro no ve 0 g. El firmware debe añadir un criterio con el giróscopo del BMI088 (inclinación o velocidad angular anormal) y un disparo manual por radio (RB2 libre).
- **No corta los motores.** El tubo debe expulsar hacia arriba, fuera del plano de las hélices.

## Estructura del repositorio

```
paracaidas-dron/
├── docs/
│   ├── esquematicos/esquematico.md   # Fuente de verdad: componentes, netlist pin a pin, notas
│   └── investigacion/modulos.md      # Selección de módulos, cálculo y construcción del paracaídas
├── hardware/
│   └── paracaidas-matrice400/        # Proyecto KiCad 10 (.kicad_pro / .kicad_sch / .kicad_pcb)
│       ├── paracaidas.kicad_sym      # Símbolo propio (BMI088 en modo I2C)
│       ├── paracaidas.pretty/        # Footprint propio (buzzer MLT-9650)
│       └── jlcpcb/                   # Gerbers, BOM y CPL para pedir la placa armada
├── CLAUDE.md                         # Guía para Claude Code
└── README.md
```

El firmware irá en `firmware/src/` (código) y `firmware/lib/` (librerías) cuando se implemente.

## Primeros pasos

### Abrir el hardware

1. Instala [KiCad 10](https://www.kicad.org/download/).
2. Abre `hardware/paracaidas-matrice400/paracaidas-matrice400.kicad_pro`.

Para verificar el diseño desde la línea de comandos:

```sh
KICAD="C:/Program Files/KiCad/10.0/bin/kicad-cli.exe"
cd hardware/paracaidas-matrice400

"$KICAD" sch erc paracaidas-matrice400.kicad_sch
"$KICAD" pcb drc --schematic-parity paracaidas-matrice400.kicad_pcb
```

### Firmware (cuando exista)

- Toolchain: MPLAB X + XC8, programado con PICkit por el header ICSP **J2** (RB6/RB7).
- Depuración serie por el header UART **J1** (RC6 TX / RC7 RX).
- Bits de configuración base: `FOSC = INTIO67`, `PLLCFG = ON`, `MCLRE = EXTMCLR`, `LVP = OFF`, `WDTEN = OFF` (activar el WDT en producción).

## Cambiar el diseño

[`docs/esquematicos/esquematico.md`](docs/esquematicos/esquematico.md) es la **fuente de verdad** de la conectividad. Las referencias coinciden 1:1 con el `.kicad_sch` y el `.kicad_pcb`.

Para cualquier cambio de conexiones:

1. Actualiza el documento **y** el esquemático, y corre el ERC.
2. Actualiza el PCB desde el esquemático (F8) y vuelve a rutear las redes afectadas.
3. Rellena las zonas (B) y corre el DRC con `--schematic-parity`.

Netclasses del PCB: `Default` 0.2 mm con 0.15 mm de separación (paso fino del SSOP-28 y del LGA-16) y `Power` 0.6 mm (+5V, VBAT, SW_NODE, SOL_DRV, BAT+). GND va por plano de cobre en ambas caras.

### Pedir la placa a JLCPCB

1. Comprime `hardware/paracaidas-matrice400/jlcpcb/gerbers/` en un .zip y súbelo.
2. En PCB Assembly sube `bom_jlcpcb.csv` y `cpl_jlcpcb.csv` (cara superior).
3. Revisa la rotación de los componentes en el visor antes de confirmar. J1 (UART) y J2 (ICSP) se sueldan a mano.

Si cambias el PCB, vuelve a exportar gerbers, taladros y posiciones con `kicad-cli pcb export gerbers|drill|pos`.

## Paracaídas

El dimensionamiento completo, el patrón de gajos, la costura y los proveedores (Fruity Chutes, Mars Parachutes, modelos para 15–16 kg) están en [`docs/investigacion/modulos.md`](docs/investigacion/modulos.md#paracaídas--canopia-y-mecanismo-de-despliegue).

```
Área = 2·m·g / (Cd·ρ·v²) = (2 × 16 × 9.81) / (0.75 × 1.225 × 6²) ≈ 9.5 m²  →  Ø3.5 m
```

> Los paracaídas de Ø1.2 m que venden en AliExpress son para drones de 2–4 kg. Con el M400 darían 14–17 m/s de descenso, el equivalente a soltarlo desde 10–15 m.

## Commits

Este repo usa [Conventional Commits](https://www.conventionalcommits.org/es/v1.0.0/) con emoji y la descripción en español:

```
feat(hardware): ✨ esquemático cableado sin r5/r6 y pcb ruteado con plano de gnd
```
