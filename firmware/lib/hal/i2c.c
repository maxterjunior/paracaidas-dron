#include <xc.h>
#include "board.h"
#include "i2c.h"

#define I2C_HZ      400000UL
#define TIMEOUT     2000u           /* iteraciones de espera (~5 ms a 16 MHz) */

static bool bus_idle(void)
{
    uint16_t n = TIMEOUT;
    /* SEN/RSEN/PEN/RCEN/ACKEN en SSP1CON2<4:0> y R/W en SSP1STAT<2> */
    while ((SSP1CON2 & 0x1Fu) || (SSP1STAT & 0x04u)) {
        if (--n == 0u) {
            return false;
        }
    }
    return true;
}

static bool start(bool repeated)
{
    uint16_t n = TIMEOUT;
    if (!bus_idle()) {
        return false;
    }
    PIR2bits.BCL1IF = 0;
    if (repeated) {
        SSP1CON2bits.RSEN = 1;
        while (SSP1CON2bits.RSEN) {
            if (--n == 0u) {
                return false;
            }
        }
    } else {
        SSP1CON2bits.SEN = 1;
        while (SSP1CON2bits.SEN) {
            if (--n == 0u) {
                return false;
            }
        }
    }
    return !PIR2bits.BCL1IF;
}

static void stop(void)
{
    uint16_t n = TIMEOUT;
    (void)bus_idle();
    SSP1CON2bits.PEN = 1;
    while (SSP1CON2bits.PEN) {
        if (--n == 0u) {
            return;
        }
    }
}

static bool write_byte(uint8_t b)
{
    uint16_t n = TIMEOUT;
    if (!bus_idle()) {
        return false;
    }
    PIR1bits.SSP1IF = 0;
    SSP1BUF = b;
    if (SSP1CON1bits.WCOL) {
        SSP1CON1bits.WCOL = 0;
        return false;
    }
    while (!PIR1bits.SSP1IF) {
        if (--n == 0u) {
            return false;
        }
    }
    return SSP1CON2bits.ACKSTAT == 0;   /* 0 = el esclavo respondió ACK */
}

static bool read_byte(uint8_t *b, bool ack)
{
    uint16_t n = TIMEOUT;
    if (!bus_idle()) {
        return false;
    }
    SSP1CON2bits.RCEN = 1;
    while (!SSP1STATbits.BF) {
        if (--n == 0u) {
            return false;
        }
    }
    *b = SSP1BUF;
    if (!bus_idle()) {
        return false;
    }
    SSP1CON2bits.ACKDT = ack ? 0 : 1;   /* NACK en el último byte */
    SSP1CON2bits.ACKEN = 1;
    n = TIMEOUT;
    while (SSP1CON2bits.ACKEN) {
        if (--n == 0u) {
            return false;
        }
    }
    return true;
}

void i2c_init(void)
{
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;
    SSP1CON1 = 0x00;
    SSP1ADD = (uint8_t)(_XTAL_FREQ / (4UL * I2C_HZ) - 1UL);    /* 9 -> 400 kHz */
    SSP1STAT = 0x00;            /* SMP = 0: control de slew rate para 400 kHz */
    SSP1CON2 = 0x00;
    PIR1bits.SSP1IF = 0;
    PIR2bits.BCL1IF = 0;
    SSP1CON1 = 0x28;            /* SSPEN = 1, modo maestro I2C */
}

void i2c_recover(void)
{
    uint8_t i;
    SSP1CON1bits.SSPEN = 0;
    LATCbits.LATC3 = 0;
    TRISCbits.TRISC4 = 1;
    for (i = 0; i < 9u; i++) {          /* SCL como colector abierto: TRIS = 0 baja, TRIS = 1 suelta */
        TRISCbits.TRISC3 = 0;
        __delay_us(5);
        TRISCbits.TRISC3 = 1;
        __delay_us(5);
        if (PORTCbits.RC4) {
            break;                      /* el esclavo soltó SDA */
        }
    }
    i2c_init();
    stop();
}

bool i2c_write_reg(uint8_t addr7, uint8_t reg, uint8_t val)
{
    bool ok = start(false)
              && write_byte((uint8_t)(addr7 << 1))
              && write_byte(reg)
              && write_byte(val);
    stop();
    return ok;
}

bool i2c_read_regs(uint8_t addr7, uint8_t reg, uint8_t *buf, uint8_t n)
{
    uint8_t i;
    bool ok = start(false)
              && write_byte((uint8_t)(addr7 << 1))
              && write_byte(reg)
              && start(true)
              && write_byte((uint8_t)((addr7 << 1) | 1u));
    for (i = 0; ok && i < n; i++) {
        ok = read_byte(&buf[i], (uint8_t)(i + 1u) < n);
    }
    stop();
    return ok;
}
