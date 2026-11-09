/*
 * Placa paracaídas M400 rev. v4 — mapa de pines del PIC18F25K22 (ver docs/esquematicos/esquematico.md).
 *
 *   RA0/AN0  BAT_SENSE (VBAT/2)        RC0  BZ_CTRL  (buzzer activo, alto = suena)
 *   RA1      LED_G (verde)             RC2  SOL_GATE (solenoide, alto = libera)
 *   RA2      LED_R (rojo)              RC3  SCL      (MSSP1, I2C 400 kHz)
 *   RB0      GYR_INT (BMI088 INT3)     RC4  SDA
 *   RB1      ACC_INT (BMI088 INT1)     RC6  UART_TX  (J1, 115200 8N1)
 *   RB2      libre (disparo manual)    RC7  UART_RX
 *   RB6/RB7  ICSP (PGC/PGD)            RE3  MCLR (SW2)
 */
#ifndef BOARD_H
#define BOARD_H

#include <xc.h>

#define _XTAL_FREQ      16000000UL      /* HFINTOSC 16 MHz, sin PLL */

#define LED_G_LAT       LATAbits.LATA1
#define LED_R_LAT       LATAbits.LATA2
#define BUZZER_LAT      LATCbits.LATC0
#define SOL_LAT         LATCbits.LATC2

void board_init(void);

#endif
