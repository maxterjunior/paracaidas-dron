/* Bits de configuración del PIC18F25K22 (MPLAB XC8). */
#include <xc.h>

/* CONFIG1H: oscilador interno 16 MHz, RA6/RA7 como GPIO, sin PLL */
#pragma config FOSC = INTIO67, PLLCFG = OFF, PRICLKEN = ON, FCMEN = OFF, IESO = OFF
/* CONFIG2L/H: power-up timer, brown-out 2.5 V siempre activo, watchdog controlado por software (~1 s) */
#pragma config PWRTEN = ON, BOREN = SBORDIS, BORV = 250
#pragma config WDTEN = SWON, WDTPS = 256
/* CONFIG3H: PORTB digital al reset, MCLR habilitado (SW2) */
#pragma config CCP2MX = PORTC1, PBADEN = OFF, CCP3MX = PORTB5, HFOFST = ON
#pragma config T3CMX = PORTC0, P2BMX = PORTB5, MCLRE = EXTMCLR
/* CONFIG4L */
#pragma config STVREN = ON, LVP = OFF, XINST = OFF
/* Protecciones desactivadas */
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF, CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF, WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF, EBTRB = OFF
