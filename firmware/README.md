# Firmware — Paracaídas M400

Firmware del PIC18F25K22 de la placa v4 (MPLAB XC8, C99). Versión **0.1.0**.

> ⚠️ **Todavía no se compiló ni se probó en hardware.** La lógica de detección (`src/detector.c`) se verificó con una simulación de los mismos casos de `test/test_detector.c`; los drivers (I2C, BMI088, ADC, UART) están escritos contra el datasheet y deben probarse en la placa. Los umbrales marcados "AJUSTAR" en `src/app_config.h` son estimaciones: calibrarlos con registros reales antes de volar.

## Estructura

```
firmware/
├── src/
│   ├── main.c          # máquina de estados, despliegue, indicadores, log
│   ├── detector.c/.h   # criterios de emergencia y "en vuelo" (C puro, probado en PC)
│   ├── app_config.h    # umbrales y tiempos
│   ├── board.c/.h      # reloj 16 MHz y pines de la placa v4
│   ├── config_bits.c   # bits de configuración
│   └── imu_types.h
├── lib/
│   ├── hal/            # tick 1 ms (Timer2), I2C (MSSP1), UART (EUSART1), ADC (FVR 2.048 V)
│   └── bmi088/         # driver del IMU (acel. 0x18 ±12 g, giro 0x68 ±1000 °/s, 400 Hz)
├── test/test_detector.c
└── Makefile
```

## Compilar y programar

1. Instala [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) y el compilador [MPLAB XC8](https://www.microchip.com/xc8) (la licencia gratuita alcanza).
2. Desde `firmware/` (con `xc8-cc` en el PATH y `make`, p. ej. Git Bash):
   ```sh
   make              # -> build/paracaidas.hex
   make test         # pruebas de la lógica en la PC (necesita gcc)
   ```
   O en MPLAB X: *New Project → Standalone → PIC18F25K22 → XC8*, agrega los `.c` de `src/` y `lib/` y las carpetas `src`, `lib/hal`, `lib/bmi088` a *Include directories*.
3. Programa `build/paracaidas.hex` con un PICkit por **J2** (pin 1 = MCLR). Alimenta la placa con la batería, o configura el PICkit para dar 3.3 V.

## Funcionamiento

Reloj interno de **16 MHz sin PLL** (4 MIPS): alcanza para leer el IMU a 500 Hz y funciona en todo el rango de VDD, sin depender de la frecuencia máxima del PIC18F a 3.3 V.

### Estados e indicadores

| Estado | Cuándo | LED verde | LED rojo | Buzzer |
|---|---|---|---|---|
| Arranque | encendido | prueba de 200 ms (LEDs y buzzer) | | |
| **DESARMADO** | en tierra, o primeros 10 s | destello cada 2 s | destello si batería < 3.5 V | — |
| **ARMADO** | en vuelo (vibración de motores ≥ 5 s) | fijo | destello si batería baja | — |
| **DESPLEGADO** | disparó | — | fijo | 1 Hz (baliza para encontrarlo) |
| **FALLA** | el IMU no responde | — | parpadeo rápido | bip cada 2 s |

- **Armado:** el sistema solo dispara en ARMADO. Pasa a ARMADO cuando la vibración de los motores (componente de alta frecuencia de la aceleración) supera `VIB_ARM_MG` durante 5 s, y vuelve a DESARMADO tras 10 s sin vibración. Así no se despliega al cargar o inclinar el dron en tierra. Si todos los motores se paran en vuelo, sigue armado esos 10 s, de sobra para detectar la caída.
- **FALLA nunca despliega:** sin IMU no hay forma de saber si el dron cae. Reintenta el IMU cada segundo.

### Criterios de disparo (estando ARMADO)

| Criterio | Condición | Tiempo | Caso |
|---|---|---|---|
| Caída libre | \|a\| < 300 mg | 100 ms | corte total de motores |
| Giro descontrolado | \|ω\| > 250 °/s | 300 ms | fallo de un motor (el dron gira) |
| Inclinación | > 70° con la vertical (o invertido) | 500 ms | vuelco |

Aceleración y giro pasan por filtros IIR de primer orden (aritmética entera). La inclinación usa el eje Z del BMI088 hacia arriba (placa con los componentes hacia arriba). Si la placa se monta de otra forma, cambia `TILT_UP_AXIS` / `TILT_UP_SIGN`.

### Despliegue

- Dos pulsos de 300 ms al solenoide (RC2), separados 700 ms, por si el pestillo no suelta al primero.
- **Reset durante el pulso:** si la batería cae y el PIC se reinicia por brown-out o watchdog a mitad del despliegue, una variable `__persistent` lo detecta y repite el pulso apenas arranca.
- **Reset en vuelo:** si el PIC se reinicia estando ARMADO, vuelve directo a ARMADO sin esperar los 15 s de rearme.
- Watchdog de ~1 s y brown-out a 2.5 V.

### Log por UART (J1, 115200 8N1)

Cada segundo:
```
[12345] ARMADO a=1003 g=2 vib=180 vuelo=1 vbat=3912
```
`a` = |aceleración| en mg, `g` = |giro| en °/s, `vib` = vibración en mg, `vbat` en mV. También se registran los cambios de estado y la causa del despliegue (1 = caída libre, 2 = giro, 4 = inclinación).

**Para calibrar:** registra el log con el dron en tierra con motores apagados, con motores en ralentí, en hover y en maniobras. Con eso ajusta `VIB_ARM_MG`/`VIB_DISARM_MG` (entre el ralentí y la mesa) y verifica que `a`, `g` e inclinación en maniobras normales queden lejos de los umbrales.

### Banco de pruebas

`make BENCH=1` habilita comandos por UART (terminar con Enter): `ARM` (arma sin vibración), `DISARM`, `FIRE` (despliega). **No volar con esta compilación**: ruido en RX podría disparar.

## Pendientes

- Compilar con XC8 y probar en la placa (drivers sin verificar en hardware).
- Calibrar los umbrales con registros de vuelo reales.
- Disparo manual por radio en RB2/INT2 (pin libre en la placa).
- Usar las interrupciones de dato listo del BMI088 (INT1 → RB1, INT3 → RB0) en lugar de muestrear por tiempo.
