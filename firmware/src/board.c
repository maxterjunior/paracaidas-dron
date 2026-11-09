#include <xc.h>
#include "board.h"

void board_init(void)
{
    /* Salidas en bajo antes de habilitarlas: el solenoide nunca debe activarse al arrancar
     * (además R2 mantiene la puerta de Q1 en bajo mientras el pin es entrada). */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /* Oscilador interno 16 MHz sin PLL (FOSC = INTIO67, PLLCFG = OFF). */
    OSCCONbits.IRCF = 0b111;
    OSCTUNEbits.PLLEN = 0;
    while (!OSCCONbits.HFIOFS) {
    }

    /* Todo digital salvo RA0/AN0 (medición de batería). RC3/RC4 deben ser digitales para el I2C. */
    ANSELA = 0x01;
    ANSELB = 0x00;
    ANSELC = 0x00;

    /* Direcciones. Los pines sin uso quedan como salida en bajo (no flotan). */
    TRISA = 0b00000001;     /* RA0 entrada analógica; RA1, RA2 LEDs; RA3-RA7 libres */
    TRISB = 0b11000111;     /* RB0/RB1 INT del IMU, RB2 libre (entrada), RB6/RB7 ICSP; RB3-RB5 libres */
    TRISC = 0b11011000;     /* RC3/RC4 I2C, RC6/RC7 UART como entradas; RC0, RC2 salidas; RC1, RC5 libres */
    WPUB = 0x00;
    INTCON2bits.nRBPU = 1;  /* sin pull-ups débiles */
}
