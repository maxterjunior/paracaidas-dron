/* I2C maestro por MSSP1 (RC3 = SCL, RC4 = SDA), 400 kHz. Todas las esperas tienen timeout:
 * un bus colgado nunca bloquea el lazo principal. */
#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

void i2c_init(void);
void i2c_recover(void);     /* libera un esclavo que retiene SDA (9 pulsos de SCL) y reinicia el MSSP */
bool i2c_write_reg(uint8_t addr7, uint8_t reg, uint8_t val);
bool i2c_read_regs(uint8_t addr7, uint8_t reg, uint8_t *buf, uint8_t n);

#endif
