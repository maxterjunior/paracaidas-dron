/* Base de tiempo de 1 ms con Timer2. */
#ifndef TICK_H
#define TICK_H

#include <stdint.h>

void     tick_init(void);
void     tick_isr(void);        /* llamar desde la rutina de interrupción */
uint32_t tick_ms(void);         /* milisegundos desde el arranque */

#endif
