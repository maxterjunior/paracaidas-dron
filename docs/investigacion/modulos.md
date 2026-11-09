# Investigación de Módulos — Paracaídas para DJI Matrice 400

## Especificaciones del Sistema

- **Dron:** DJI Matrice 400 — 9.74 kg con baterías (5.02 kg sin baterías), MTOW 15.8 kg, carga útil máx. 6 kg ([specs DJI](https://enterprise.dji.com/matrice-400/specs))
- **Masa de diseño del paracaídas:** 16 kg (MTOW + ~0.3 kg del propio sistema)
- **Detección:** Caída libre via IMU/acelerómetro (+ giroscopio, pendiente; ver Notas de Diseño)
- **Despliegue:** Mecanismo de resorte (spring-loaded)
- **MCU:** PIC18F25K22 SSOP-28 (v4: el PIC18LF25K22 SMD no tiene stock en JLCPCB; mismo chip con rango 2.3–5.5V)
- **Sistema:** Independiente del Flight Controller

---

## MCU — Selección Final

| Característica    | PIC18LF25K22     | PIC18F4550       | STM32F411        |
|-------------------|------------------|------------------|------------------|
| Core              | 8-bit            | 8-bit            | 32-bit ARM        |
| MIPS              | 16               | 12               | 100              |
| RAM               | 3.8 KB           | 2 KB             | 128 KB           |
| Flash             | 32 KB            | 32 KB            | 512 KB           |
| Voltaje           | 1.8–3.6V         | 2.0–5.5V         | 1.7–3.6V         |
| SPI/I2C           | 2x MSSP          | 1x MSSP          | 3x SPI / 3x I2C  |
| UART              | 2x EUSART        | 1x USART         | 3x USART         |
| FPU               | ❌ No            | ❌ No            | ✅ Sí            |
| USB               | ❌ No            | ✅ Sí (inútil)   | ✅ Sí            |
| Level shifter IMU | ❌ No necesario  | ❌ No necesario (2.0–5.5V) | ❌ No necesario  |
| Disponible        | ✅ En mano       | —                | —                |
| **Seleccionado**  | ✅ **SÍ**        | —                | —                |

---

## IMU — Candidatos

| Módulo        | Chip          | Interface | Rango Accel     | ODR max  | Vibration tolerance | Costo  | Compatibilidad 3.3V |
|---------------|---------------|-----------|-----------------|----------|---------------------|--------|----------------------|
| GY-521        | MPU-6050      | I2C       | ±2g–±16g        | 1 kHz    | Media               | ~$1    | ✅ (con pull-ups)    |
| GY-91         | MPU-9250      | SPI/I2C   | ±2g–±16g        | 4 kHz    | Media               | ~$3    | ✅                   |
| **ICM-42688** | ICM-42688-P   | SPI/I2C   | ±2g–±16g        | 32 kHz   | Alta                | ~$5    | ✅                   |
| BMI088        | BMI088        | SPI/I2C   | ±3g–±24g        | 1.6 kHz  | **Muy alta**        | ~$4    | ✅                   |

### Recomendación IMU
- **Opción económica:** MPU-6050 (GY-521) — suficiente para detección de caída libre, I2C, librerías simples
- **Opción robusta:** BMI088 — diseñado específicamente para drones, mejor tolerancia a vibraciones de motores

---

## Actuador de Despliegue — Candidatos

| Tipo              | Tiempo respuesta | Corriente  | Voltaje | Reversible | Costo  |
|-------------------|------------------|------------|---------|------------|--------|
| Servo SG90        | ~200 ms          | 100–250 mA | 4.8–6V  | ✅ Sí      | ~$2    |
| Servo MG996R      | ~150 ms          | 500–900 mA | 4.8–6V  | ✅ Sí      | ~$4    |
| **Solenoid 5V**   | ~30–50 ms        | 500 mA     | 5V      | ✅ Sí      | ~$3    |
| Solenoid 12V      | ~20–30 ms        | 300 mA     | 12V     | ✅ Sí      | ~$4    |
| Burn-wire         | ~100 ms          | 1–2 A      | 3–5V    | ❌ No      | ~$0.5  |

### Recomendación Actuador
- **Solenoid 5V push-pull** — mejor balance entre velocidad, corriente y voltaje disponible en sistema de 5V

---

## Driver del Actuador

| Componente    | Uso                                      | Costo |
|---------------|------------------------------------------|-------|
| MOSFET N AO3400A | Switching solenoid desde pin PIC 3.3V (Rds(on) especificada a Vgs 2.5V) | ~$0.1 |
| ~~IRLZ44N~~ | Descartado: Rds(on) especificada a Vgs 4–5V, no garantiza conducción plena a 3.3V | — |
| Diodo SS34    | Flyback — protección contra pico solenoid | ~$0.04 |
| Diodo SS34    | Schottky de salida del boost MT3608 (obligatorio) | ~$0.1 |
| Resistor 10kΩ | Pull-down gate MOSFET                    | ~$0.01|

---

## Batería

| Opción            | Capacidad | Voltaje | Peso   | Duración estimada (standby) |
|-------------------|-----------|---------|--------|-----------------------------|
| LiPo 1S 500mAh    | 500 mAh   | 3.7V    | ~12g   | ~4–6 h                      |
| **LiPo 1S 1000mAh** | 1000 mAh | 3.7V  | ~23g   | ~8–12 h                     |
| LiPo 2S 500mAh    | 500 mAh   | 7.4V    | ~25g   | ~4–6 h                      |

Con regulador 3.3V (LDO XC6206) para el PIC y la IMU. Un MOSFET P (AO3401A) hace de interruptor de encendido: el deslizante SMD solo soporta ~50 mA y maneja su puerta.

> La autonomía en standby supone el solenoide **apagado** durante el vuelo (lógica de liberación por pulso, ver abajo). Si el solenoide tuviera que estar energizado para retener (~3 W desde la LiPo vía boost), la batería de 1000 mAh no llegaría a 1 h.

---

## Tabla Final — Módulos Seleccionados

| Módulo            | Componente          | Motivo de selección                              |
|-------------------|---------------------|--------------------------------------------------|
| **MCU**           | PIC18F25K22-I/SS    | Mismo chip que el LF en mano, SMD con stock JLCPCB, 2x MSSP, 16 MIPS |
| **IMU**           | BMI088              | Diseñado para drones, ±24 g, tolera vibración; MPU-6050 discontinuado (~US$21 en JLCPCB) |
| **Actuador**      | Solenoid 5V         | Rápido (<50ms), simple, reversible               |
| **Driver**        | MOSFET AO3400A      | Logic-level especificado a Vgs 2.5V              |
| **Regulador**     | XC6206P332MR        | LDO 3.3V 200 mA, componente básico de JLCPCB     |
| **Batería**       | LiPo 1S 1000mAh     | Ligera, suficiente autonomía para vuelo          |
| **Protección**    | Diodo SS34          | Flyback del solenoid (mismo componente que D4)   |
| **Boost 5V**      | MT3608 + SS34 + 22µH | VBAT → 5.1V para solenoide y buzzer             |

---

## Paracaídas — Canopia y Mecanismo de Despliegue

### Dimensionamiento

Para el Matrice 400 (peor caso 16 kg = MTOW 15.8 kg + sistema; velocidad de descenso objetivo ≤6 m/s):

```
Área = (2 × m × g) / (Cd × ρ × v²)
     = (2 × 16 × 9.81) / (0.75 × 1.225 × 36)
     = 313.9 / 33.08
     ≈ 9.5 m²   →  diámetro = √(4 × 9.5 / π) ≈ 3.5 m  (circular, Cd≈0.75)
```

> **Corrección (v3):** la versión anterior daba 0.82 m² / Ø1.2 m por un error aritmético (con 11 kg el resultado real es 6.5 m², no 0.82 m²) y además usaba un peso desactualizado (6.7 / 11 kg). Un paracaídas de Ø1.2 m deja caer al M400 a 14–17 m/s, equivalente a soltarlo desde 10–15 m: no lo protege.

Velocidad de descenso `v = √(2·m·g / (Cd·ρ·A))` según diámetro (Cd 0.75, nivel del mar):

| Diámetro | Área | 10.1 kg (sin carga útil) | 16 kg (MTOW) |
|---|---|---|---|
| 1.2 m | 1.13 m² | 13.8 m/s ❌ | 17.4 m/s ❌ |
| 2.5 m | 4.91 m² | 6.6 m/s | 8.3 m/s ❌ |
| 3.0 m | 7.07 m² | 5.5 m/s | 7.0 m/s ⚠️ |
| **3.5 m** | **9.62 m²** | **4.7 m/s** | **6.0 m/s** ✅ |

**Recomendación:** diámetro **3.5 m** si se vuela con carga útil cerca del MTOW; **3.0 m** es aceptable si el dron vuela siempre por debajo de ~12 kg. En altura (ρ menor) la velocidad sube: a 2000 m s.n.m. (ρ ≈ 1.0 kg/m³) aumenta ~10%.

Peso del conjunto (Ø3.5 m): tela ~340 g + cuerdas, refuerzos y bolsa ≈ **0.5–0.7 kg**. Volumen plegado ≈ 1.5–2 L, por lo que el tubo de muelle debe dimensionarse para ese volumen (Ø ~100 mm × ~250 mm) y el resorte para expulsar ~0.6 kg.

### Tipo de Canopia

| Tipo | Estabilidad | Complejidad | Recomendado |
|------|------------|-------------|-------------|
| Circular clásico | Media | Baja | ✅ Opción principal |
| Cruciforme (cross) | Alta | Media | ✅ Alternativa más estable |
| Ballistic/rocket-deployed | Alta | Alta | Para futuras versiones |

**Material seleccionado: Ripstop Nylon 30D con recubrimiento sil-nylon**

| Propiedad | Valor |
|-----------|-------|
| Peso | ~35 g/m² → ~340 g para 9.6 m² (Ø3.5 m) |
| Recubrimiento | Silicona (sil-nylon) — reduce porosidad, hidrófugo |
| Resistencia | Trama ripstop cada ~5 mm evita propagación de desgarros |
| Plegabilidad | Excelente — comprime bien dentro del tubo de muelle |

- **Fuentes:** "sil-nylon 30D ripstop" en AliExpress/Amazon (~$3–6/m²), ancho típico 150 cm

### Construcción — Patrón y Costura

**Patrón:** 12 gajos trapezoidales iguales (360°/12 = 30° por gajo). Con Ø3.5 m, 8 gajos tendrían 1.37 m de base y no caben en una tela de 150 cm con márgenes.

| Dimensión | Valor |
|-----------|-------|
| Cantidad de gajos | 12 |
| Alto del gajo | ~1.75 m (radio del paracaídas) |
| Ancho en la base | (π × 3.5) / 12 ≈ 0.92 m |
| Margen de costura | 1.5 cm en cada borde longitudinal |
| Tela necesaria | ~12 m lineales de tela de 150 cm (dos gajos invertidos por largo) |

**Orden de costura:**

1. Cortar 12 gajos idénticos con margen de 1.5 cm
2. Unir gajos de a pares → 6 piezas (6 costuras)
3. Unir los 6 pares en tríos → 2 mitades (4 costuras)
4. Unir las 2 mitades por ambos bordes → canopia completa (2 costuras; total 12 costuras = 12 gajos)
5. Reforzar el ápex
6. Dobladillo en el borde inferior
7. Fijar las 12 cuerdas de suspensión

**Tipos de costura por zona:**

| Costura | Dónde | Por qué |
|---------|-------|---------|
| Plana francesa (flat-felled) | Unión de gajos | Encapsula las hebras, distribuye carga |
| Doble pespunte (2 líneas a 3 mm) | Todo el paracaídas | Mínimo resistente |
| Zigzag | Dobladillo del borde inferior | Permite elasticidad leve |

La costura plana francesa es la crítica: se cose derecho con derecho a 1 cm, se abre, se dobla un margen sobre el otro (0.5 cm) y se cose encima — todas las hebras quedan encapsuladas.

**Refuerzo del ápex:**

- Parche de tafetán 10×10 cm pegado con adhesivo de contacto + cosido en X
- Ojal reforzado o anillo de acero inoxidable Ø15 mm para atar las cuerdas

**Suspensión:**

- 12 cuerdas Dyneema/Spectra 1 mm (~100 kg de rotura c/u) o nylon paracord 1.5 mm, longitud ~3.5 m c/u (≈ 1 × diámetro)
- El choque de apertura puede ser varias veces el peso del dron: la carga total de rotura de las cuerdas y del punto de anclaje al dron debe superar ~10× el peso (≥160 kgf)
- Cada cuerda pasa por dentro del dobladillo del borde y sale por un refuerzo de tafetán (3×3 cm) cosido en el punto de anclaje
- Nudo de alondra o costura en barra sobre cada refuerzo

**Herramientas mínimas:**

- Máquina de coser con aguja 70/10 (telas livianas)
- Hilo nylon 210D o poliéster N°50
- Papel kraft para el patrón
- Marcador de tela + regla curva

### Mecanismo de Despliegue

Compatible con el solenoid 5V (SOL1) del circuito actual:

| Mecanismo | Integración con SOL1 | Velocidad | Seleccionado |
|-----------|---------------------|-----------|-------------|
| Caja de resorte comprimido | ✅ Solenoid libera el seguro | ~100–200 ms | — |
| **Tubo de muelle (spring tube)** | ✅ Solenoid empuja el tapón | ~50–100 ms | ✅ |
| Velcro + resorte | ✅ Solenoid corta tensión | ~150 ms | — |
| Pirotécnico (e-match) | ⚠️ Requiere rediseño circuito | <50 ms | — |

**Selección:** tubo de muelle + solenoid como seguro mecánico.
Lógica: el pestillo mecánico retiene **sin energía**; un pulso de ~300 ms del solenoide retira el pin y el muelle expulsa el paracaídas. Así no hay consumo en vuelo y un reset o corte de energía no despliega el paracaídas.

### Dónde Conseguir

- **Listo para usar:** Fruity Chutes, Mars Parachutes — buscar modelos certificados/especificados para 15–16 kg (los de AliExpress de Ø1.2 m son para drones de 2–4 kg)
- **Hecho a medida:** ripstop nylon + costuras radiales
- **Referencia:** ParaZero SafeAir (whitepapers públicos)

---

## Notas de Diseño

- El PIC opera a 3.3V → mismo nivel que la IMU → sin conversores de nivel
- Punto fijo (integer math) en firmware para compensar la falta de FPU
- Filtro IIR de primer orden en firmware para eliminar vibración de motores
- Umbral de caída libre: `|√(ax²+ay²+az²)| < 0.3g` sostenido por >100 ms
- **La caída libre sola no basta:** con un motor fallado el dron gira o se voltea con los demás motores empujando y el acelerómetro no ve 0 g. Agregar criterio con giroscopio (inclinación > ~60–70° o velocidad angular anormal sostenida) y disparo manual por radio
- El solenoid de 5V requiere boost desde LiPo 3.7V → regulador step-up (MT3608 o similar)
- Paracaídas circular **Ø3.5 m** de ripstop (ver Dimensionamiento); expulsión ~50–100 ms con resorte + solenoid, inflado completo ~1–2 s
- **Altura mínima de despliegue:** detección + pulso + expulsión + inflado ≈ 1.5–2.5 s → se pierden ~20–40 m. Por debajo de esa altura el sistema no alcanza a frenar el dron
- **Hélices:** el sistema no corta los motores; el tubo debe expulsar hacia arriba, fuera del plano de las hélices, con velocidad suficiente para evitar que la canopia o las cuerdas se enreden (los sistemas comerciales cortan los motores por esta razón)
