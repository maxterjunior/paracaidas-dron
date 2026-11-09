/* Tipos compartidos entre el driver del IMU y la lógica de detección (sin dependencias del PIC). */
#ifndef IMU_TYPES_H
#define IMU_TYPES_H

#include <stdint.h>

typedef struct {
    int16_t a[3];   /* aceleración en mg (X, Y, Z) */
    int16_t g[3];   /* velocidad angular en °/s (X, Y, Z) */
} imu_sample_t;

#endif
