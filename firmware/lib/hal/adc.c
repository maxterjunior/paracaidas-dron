#include <xc.h>
#include "adc.h"

void adc_init(void)
{
    VREFCON0bits.FVRS = 0b10;       /* FVR = 2.048 V */
    VREFCON0bits.FVREN = 1;
    while (!VREFCON0bits.FVRST) {
    }
    ADCON2 = 0b10111101;            /* justificado a derecha, Tacq = 20 TAD, TAD = Fosc/16 = 1 us */
    ADCON1 = 0b00001000;            /* Vref+ = FVR BUF2, Vref- = AVss */
    ADCON0 = 0b00000001;            /* canal AN0, ADC encendido */
}

uint16_t adc_vbat_mv(void)
{
    uint16_t raw;
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO) {
    }
    raw = ((uint16_t)ADRESH << 8) | ADRESL;
    /* VBAT = raw / 1023 × 2048 mV × 2 */
    return (uint16_t)(((uint32_t)raw * 4096UL + 511UL) / 1023UL);
}
