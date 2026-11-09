/*
 * Lógica de detección de emergencia. C puro (sin registros del PIC), así se
 * puede probar en la PC: ver test/test_detector.c.
 */
#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "imu_types.h"

/* Causas de disparo (máscara de bits) */
#define DET_FREEFALL    0x01u
#define DET_TUMBLE      0x02u
#define DET_TILT        0x04u

typedef struct {
    int32_t  fa[3];         /* aceleración filtrada, mg << 4 */
    int32_t  fg[3];         /* giro filtrado, °/s << 4 */
    int32_t  vib;           /* vibración promedio, mg << 4 */
    int32_t  mag2;          /* |a|² filtrada, mg² */
    int32_t  gyr2;          /* |w|² filtrada, (°/s)² */
    uint16_t ff_ms, tumble_ms, tilt_ms;
    uint16_t vib_hi_ms, vib_lo_ms;
    bool     in_flight;
    bool     primed;
} detector_t;

void     detector_init(detector_t *d);

/* Procesa una muestra; dt_ms = tiempo desde la anterior.
 * Devuelve las causas cuyo criterio se cumplió el tiempo requerido (0 = nada). */
uint8_t  detector_update(detector_t *d, const imu_sample_t *s, uint16_t dt_ms);

/* true si la vibración indica motores girando (dron en vuelo). */
bool     detector_in_flight(const detector_t *d);

/* Valores para el log. */
uint16_t detector_acc_mg(const detector_t *d);
uint16_t detector_gyro_dps(const detector_t *d);
uint16_t detector_vib_mg(const detector_t *d);

#endif
