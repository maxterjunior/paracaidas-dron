#include <xc.h>
#include "tick.h"

static volatile uint32_t ms_count;

void tick_init(void)
{
    /* Fosc/4 = 4 MHz, prescaler 1:16 -> 250 kHz, PR2 = 249 -> 1 kHz */
    T2CON = 0x00;
    T2CONbits.T2CKPS = 0b10;
    PR2 = 249;
    TMR2 = 0;
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    T2CONbits.TMR2ON = 1;
}

void tick_isr(void)
{
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        ms_count++;
    }
}

uint32_t tick_ms(void)
{
    uint32_t t;
    PIE1bits.TMR2IE = 0;    /* lectura atómica de 32 bits en un micro de 8 bits */
    t = ms_count;
    PIE1bits.TMR2IE = 1;
    return t;
}
