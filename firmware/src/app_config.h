/*
 * Parámetros de la aplicación: umbrales de detección, armado, despliegue.
 * Los marcados "AJUSTAR" son estimaciones iniciales: hay que calibrarlos con
 * registros reales del M400 (log UART) antes de volar.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define FW_VERSION              "0.1.0"

/* ---- Muestreo ---- */
#define SAMPLE_PERIOD_MS        2u      /* 500 Hz (acel. y giro a ODR 400 Hz) */
#define ACC_IIR_SHIFT           2       /* filtro IIR de 1er orden, alfa = 1/4 (~20 Hz) */
#define GYR_IIR_SHIFT           3       /* alfa = 1/8 */

/* ---- Criterio 1: caída libre ---- */
#define FF_THRESH_MG            300     /* |a| < 0.3 g */
#define FF_TIME_MS              100u    /* sostenido > 100 ms */

/* ---- Criterio 2: giro descontrolado (fallo de motor) ---- AJUSTAR */
#define TUMBLE_THRESH_DPS       250     /* |w| > 250 °/s */
#define TUMBLE_TIME_MS          300u

/* ---- Criterio 3: inclinación excesiva ---- AJUSTAR */
#define TILT_UP_AXIS            2       /* eje del IMU que apunta hacia arriba: 0=X 1=Y 2=Z */
#define TILT_UP_SIGN            (+1)    /* +1: placa con la cara de componentes hacia arriba */
#define TILT_COS2_X1000         117     /* cos²(70°) × 1000 -> dispara con > 70° */
#define TILT_MIN_MG             500     /* solo se evalúa si |a| > 0.5 g */
#define TILT_TIME_MS            500u

/* ---- Armado por vibración de motores ---- AJUSTAR */
#define VIB_IIR_SHIFT           5       /* promedio de la vibración (alfa = 1/32) */
#define VIB_ARM_MG              60      /* vibración que indica motores girando */
#define VIB_ARM_TIME_MS         5000u   /* sostenida 5 s -> en vuelo */
#define VIB_DISARM_MG           30      /* por debajo: motores parados */
#define VIB_DISARM_TIME_MS      10000u  /* sostenida 10 s -> en tierra */
#define ARM_MIN_UPTIME_MS       10000u  /* nunca armar en los primeros 10 s */

/* ---- Despliegue ---- */
#define SOL_PULSE_MS            300u    /* pulso del solenoide */
#define SOL_PULSE_GAP_MS        700u    /* pausa entre pulsos */
#define SOL_PULSE_COUNT         2u      /* 2 pulsos por si el pestillo no suelta al primero */

/* ---- Supervisión ---- */
#define IMU_FAIL_LIMIT          50u     /* lecturas fallidas seguidas (100 ms) -> FALLA */
#define IMU_RETRY_MS            1000u
#define VBAT_PERIOD_MS          1000u
#define VBAT_LOW_MV             3500u
#define LOG_PERIOD_MS           1000u

/* ---- Depuración ---- */
#ifndef UART_LOG_ENABLE
#define UART_LOG_ENABLE         1       /* log de estado cada LOG_PERIOD_MS por J1 */
#endif
#ifndef BENCH_COMMANDS
#define BENCH_COMMANDS          0       /* comandos ARM/FIRE por UART: SOLO banco de pruebas */
#endif

#endif
