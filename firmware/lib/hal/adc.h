/* Medición de la batería: RA0/AN0 = VBAT/2 (divisor R11/R12), referencia FVR 2.048 V.
 * La referencia interna no depende de VDD, que cae cuando la batería está baja.
 * Rango útil: hasta 4.09 V (por encima satura; batería cargada). */
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void     adc_init(void);
uint16_t adc_vbat_mv(void);

#endif
