/*
 * Driver mínimo del BMI088 por I2C.
 *   Acelerómetro: 0x18 (SDO1 = GND), ±12 g, ODR 400 Hz
 *   Giróscopo:    0x68 (SDO2 = GND), ±1000 °/s, ODR 400 Hz, filtro 47 Hz
 * Registros según el datasheet Bosch BST-BMI088-DS001.
 */
#ifndef BMI088_H
#define BMI088_H

#include <stdbool.h>
#include "imu_types.h"

bool bmi088_init(void);                 /* reset, verificación de chip ID y configuración (~90 ms) */
bool bmi088_read(imu_sample_t *s);      /* aceleración en mg, giro en °/s */

#endif
