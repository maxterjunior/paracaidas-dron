/*
 * Pruebas en la PC de la lógica de detección (src/detector.c) con señales sintéticas.
 *   make test        (requiere gcc o clang)
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "detector.h"
#include "app_config.h"

#define DT  SAMPLE_PERIOD_MS
#define PI  3.14159265358979

static int failures;
static uint32_t t_ms;           /* tiempo simulado */

#define CHECK(cond, msg) do { \
        if (cond) { printf("  ok    %s\n", msg); } \
        else { printf("  FALLA %s (linea %d)\n", msg, __LINE__); failures++; } \
    } while (0)

/* Muestra con gravedad en la dirección dada (g en mg), vibración senoidal y giro. */
static imu_sample_t sample(double ax, double ay, double az, double vib_mg, double gz_dps)
{
    imu_sample_t s;
    double v = vib_mg * sin(2.0 * PI * 80.0 * t_ms / 1000.0);          /* motores ~80 Hz */
    double v2 = vib_mg * sin(2.0 * PI * 113.0 * t_ms / 1000.0 + 1.0);
    s.a[0] = (int16_t)lround(ax + v);
    s.a[1] = (int16_t)lround(ay + v2);
    s.a[2] = (int16_t)lround(az + 0.5 * v);
    s.g[0] = 0;
    s.g[1] = 0;
    s.g[2] = (int16_t)lround(gz_dps);
    return s;
}

/* Corre `ms` milisegundos; devuelve el tiempo (relativo) del primer disparo, o -1. */
static long run(detector_t *d, uint32_t ms, double ax, double ay, double az, double vib, double gz,
                uint8_t *cause_out)
{
    uint32_t start = t_ms;
    long first = -1;
    while (t_ms - start < ms) {
        imu_sample_t s;
        uint8_t c;
        t_ms += DT;
        s = sample(ax, ay, az, vib, gz);
        c = detector_update(d, &s, DT);
        if (c && first < 0) {
            first = (long)(t_ms - start);
            if (cause_out) {
                *cause_out = c;
            }
        }
    }
    return first;
}

/* Lleva el detector a "en vuelo": hover con vibración. */
static void hover(detector_t *d)
{
    detector_init(d);
    t_ms = 0;
    run(d, VIB_ARM_TIME_MS + 2000u, 0, 0, 1000, 150, 0, NULL);
}

int main(void)
{
    detector_t d;
    uint8_t c = 0;
    long t;

    printf("Quieto sobre la mesa\n");
    detector_init(&d);
    t_ms = 0;
    t = run(&d, 20000, 0, 0, 1000, 2, 0, NULL);
    CHECK(t < 0, "no dispara");
    CHECK(!detector_in_flight(&d), "no se considera en vuelo (sin vibracion)");
    CHECK(abs(detector_acc_mg(&d) - 1000) < 20, "|a| ~ 1000 mg");

    printf("Hover con vibracion de motores\n");
    hover(&d);
    CHECK(detector_in_flight(&d), "en vuelo tras 5 s de vibracion");
    t = run(&d, 10000, 0, 0, 1000, 150, 0, NULL);
    CHECK(t < 0, "hover estable: no dispara");

    printf("Caida libre (corte total de motores)\n");
    hover(&d);
    c = 0;
    t = run(&d, 500, 0, 0, 0, 0, 0, &c);
    CHECK(t >= (long)FF_TIME_MS && t <= (long)FF_TIME_MS + 30, "dispara 100-130 ms despues de empezar la caida");
    CHECK(c & DET_FREEFALL, "causa = caida libre");
    CHECK(detector_in_flight(&d), "sigue en vuelo durante la caida (retencion de 10 s)");

    printf("Caida libre breve (60 ms, p. ej. turbulencia)\n");
    hover(&d);
    t = run(&d, 60, 0, 0, 0, 0, 0, NULL);
    CHECK(t < 0, "60 ms: no dispara");
    t = run(&d, 2000, 0, 0, 1000, 150, 0, NULL);
    CHECK(t < 0, "vuelve al hover sin disparar");

    printf("Aceleracion vertical fuerte hacia abajo (0.5 g)\n");
    hover(&d);
    t = run(&d, 2000, 0, 0, 500, 150, 0, NULL);
    CHECK(t < 0, "descenso agresivo a 0.5 g: no dispara");

    printf("Giro descontrolado (fallo de un motor)\n");
    hover(&d);
    c = 0;
    t = run(&d, 1000, 0, 0, 1000, 150, 400, &c);
    CHECK(t > 0 && (c & DET_TUMBLE), "400 deg/s dispara por giro");
    CHECK(t >= (long)TUMBLE_TIME_MS && t <= (long)TUMBLE_TIME_MS + 60, "dispara en 300-360 ms");

    printf("Giro normal de maniobra (120 deg/s)\n");
    hover(&d);
    t = run(&d, 3000, 0, 0, 1000, 150, 120, NULL);
    CHECK(t < 0, "no dispara");

    printf("Inclinacion\n");
    hover(&d);
    t = run(&d, 3000, 500, 0, 866, 150, 0, NULL);          /* 30 grados */
    CHECK(t < 0, "30 grados: no dispara");
    hover(&d);
    c = 0;
    t = run(&d, 2000, 1000, 0, 0, 150, 0, &c);             /* 90 grados */
    CHECK(t > 0 && (c & DET_TILT), "90 grados dispara por inclinacion");
    CHECK(t >= (long)TILT_TIME_MS && t <= (long)TILT_TIME_MS + 60, "dispara en 500-560 ms");
    hover(&d);
    c = 0;
    t = run(&d, 2000, 0, 0, -1000, 150, 0, &c);            /* invertido */
    CHECK(t > 0 && (c & DET_TILT), "invertido dispara por inclinacion");

    printf("Aterrizaje: motores apagados\n");
    hover(&d);
    run(&d, VIB_DISARM_TIME_MS - 1000u, 0, 0, 1000, 2, 0, NULL);
    CHECK(detector_in_flight(&d), "9 s sin vibracion: todavia en vuelo");
    run(&d, 2000, 0, 0, 1000, 2, 0, NULL);
    CHECK(!detector_in_flight(&d), "11 s sin vibracion: en tierra (desarma)");

    printf("Transporte a mano: inclinado y sacudido sin motores\n");
    detector_init(&d);
    t_ms = 0;
    run(&d, 15000, 1000, 0, 0, 20, 0, NULL);
    CHECK(!detector_in_flight(&d), "no se considera en vuelo (no arma)");

    printf("\n%s: %d fallas\n", failures ? "FALLO" : "OK", failures);
    return failures ? 1 : 0;
}
