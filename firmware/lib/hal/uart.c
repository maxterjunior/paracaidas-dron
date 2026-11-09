#include <xc.h>
#include <stdint.h>
#include "board.h"
#include "uart.h"

#define BAUD    115200UL

void uart_init(void)
{
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;
    BAUDCON1bits.BRG16 = 1;
    TXSTA1bits.BRGH = 1;
    TXSTA1bits.SYNC = 0;
    /* BRG16 = 1, BRGH = 1: baud = Fosc / (4 (n + 1)) -> n = 34 -> 114286 bps (-0.8 %) */
    SPBRGH1 = 0;
    SPBRG1 = (uint8_t)((_XTAL_FREQ + 2UL * BAUD) / (4UL * BAUD) - 1UL);
    RCSTA1bits.SPEN = 1;
    TXSTA1bits.TXEN = 1;
    RCSTA1bits.CREN = 1;
}

void putch(char c)
{
    while (!PIR1bits.TX1IF) {
    }
    TXREG1 = (uint8_t)c;
}

bool uart_getc(char *c)
{
    if (RCSTA1bits.OERR) {          /* desborde: reiniciar el receptor */
        RCSTA1bits.CREN = 0;
        RCSTA1bits.CREN = 1;
    }
    if (!PIR1bits.RC1IF) {
        return false;
    }
    *c = (char)RCREG1;
    return true;
}
