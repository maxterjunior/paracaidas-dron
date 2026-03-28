# Investigación de Módulos — Paracaídas para DJI Matrice 400

## Especificaciones del Sistema

- **Dron:** DJI Matrice 400 (~6.7 kg sin payload, MTOW ~11 kg)
- **Detección:** Caída libre via IMU/acelerómetro
- **Despliegue:** Mecanismo de resorte (spring-loaded)
- **MCU:** PIC18LF25K22 (disponible)
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
| Level shifter IMU | ❌ No necesario  | ⚠️ Necesario     | ❌ No necesario  |
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
| MOSFET N IRLZ44N | Switching solenoid desde pin PIC 3.3V | ~$0.5 |
| Diodo 1N4007  | Flyback — protección contra pico solenoid | ~$0.1 |
| Resistor 10kΩ | Pull-down gate MOSFET                    | ~$0.01|

---

## Batería

| Opción            | Capacidad | Voltaje | Peso   | Duración estimada (standby) |
|-------------------|-----------|---------|--------|-----------------------------|
| LiPo 1S 500mAh    | 500 mAh   | 3.7V    | ~12g   | ~4–6 h                      |
| **LiPo 1S 1000mAh** | 1000 mAh | 3.7V  | ~23g   | ~8–12 h                     |
| LiPo 2S 500mAh    | 500 mAh   | 7.4V    | ~25g   | ~4–6 h                      |

Con regulador 3.3V (LDO tipo MCP1700) para el PIC y la IMU.

---

## Tabla Final — Módulos Seleccionados

| Módulo            | Componente          | Motivo de selección                              |
|-------------------|---------------------|--------------------------------------------------|
| **MCU**           | PIC18LF25K22        | Disponible, 3.3V nativo, 2x MSSP, 16 MIPS       |
| **IMU**           | MPU-6050 (GY-521)   | Económico, suficiente, fácil de conseguir        |
| **Actuador**      | Solenoid 5V         | Rápido (<50ms), simple, reversible               |
| **Driver**        | MOSFET IRLZ44N      | Lógica 3.3V compatible, bajo Vgs threshold       |
| **Regulador**     | MCP1700-3302E       | LDO 3.3V, hasta 250mA, muy bajo quiescent current|
| **Batería**       | LiPo 1S 1000mAh     | Ligera, suficiente autonomía para vuelo          |
| **Protección**    | Diodo 1N4007        | Flyback del solenoid                             |

---

## Notas de Diseño

- El PIC opera a 3.3V → mismo nivel que la IMU → sin conversores de nivel
- Punto fijo (integer math) en firmware para compensar la falta de FPU
- Filtro IIR de primer orden en firmware para eliminar vibración de motores
- Umbral de caída libre: `|√(ax²+ay²+az²)| < 0.3g` sostenido por >100 ms
- El solenoid de 5V requiere boost desde LiPo 3.7V → regulador step-up (MT3608 o similar)
