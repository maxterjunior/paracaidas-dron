#include <xc.h>
#include "board.h"
#include "i2c.h"
#include "bmi088.h"

#define ACC_ADDR            0x18u
#define GYR_ADDR            0x68u

/* Acelerómetro */
#define ACC_CHIP_ID         0x00u   /* = 0x1E */
#define ACC_X_LSB           0x12u
#define ACC_CONF            0x40u
#define ACC_RANGE           0x41u
#define ACC_PWR_CONF        0x7Cu
#define ACC_PWR_CTRL        0x7Du
#define ACC_SOFTRESET       0x7Eu
#define ACC_ID_VAL          0x1Eu
#define ACC_CONF_400HZ      0xAAu   /* acc_bwp = normal (0xA), acc_odr = 400 Hz (0x0A) */
#define ACC_RANGE_12G       0x02u

/* Giróscopo */
#define GYR_CHIP_ID         0x00u   /* = 0x0F */
#define GYR_RATE_X_LSB      0x02u
#define GYR_RANGE           0x0Fu
#define GYR_BANDWIDTH       0x10u
#define GYR_LPM1            0x11u
#define GYR_SOFTRESET       0x14u
#define GYR_ID_VAL          0x0Fu
#define GYR_RANGE_1000      0x01u
#define GYR_BW_400HZ_47HZ   0x03u

#define SOFTRESET_CMD       0xB6u

static int16_t le16(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[1] << 8) | p[0]);
}

bool bmi088_init(void)
{
    uint8_t v;

    /* Giróscopo */
    if (!i2c_write_reg(GYR_ADDR, GYR_SOFTRESET, SOFTRESET_CMD)) {
        return false;
    }
    __delay_ms(30);
    if (!i2c_read_regs(GYR_ADDR, GYR_CHIP_ID, &v, 1) || v != GYR_ID_VAL) {
        return false;
    }
    if (!i2c_write_reg(GYR_ADDR, GYR_RANGE, GYR_RANGE_1000)
        || !i2c_write_reg(GYR_ADDR, GYR_BANDWIDTH, GYR_BW_400HZ_47HZ)
        || !i2c_write_reg(GYR_ADDR, GYR_LPM1, 0x00u)) {          /* modo normal */
        return false;
    }

    /* Acelerómetro: arranca suspendido, hay que activarlo */
    if (!i2c_write_reg(ACC_ADDR, ACC_SOFTRESET, SOFTRESET_CMD)) {
        return false;
    }
    __delay_ms(5);
    if (!i2c_read_regs(ACC_ADDR, ACC_CHIP_ID, &v, 1) || v != ACC_ID_VAL) {
        return false;
    }
    if (!i2c_write_reg(ACC_ADDR, ACC_PWR_CONF, 0x00u)) {         /* modo activo */
        return false;
    }
    __delay_ms(5);
    if (!i2c_write_reg(ACC_ADDR, ACC_PWR_CTRL, 0x04u)) {         /* acelerómetro encendido */
        return false;
    }
    __delay_ms(50);
    if (!i2c_write_reg(ACC_ADDR, ACC_CONF, ACC_CONF_400HZ)
        || !i2c_write_reg(ACC_ADDR, ACC_RANGE, ACC_RANGE_12G)) {
        return false;
    }
    /* Verificación de que la configuración quedó escrita */
    if (!i2c_read_regs(ACC_ADDR, ACC_RANGE, &v, 1) || v != ACC_RANGE_12G) {
        return false;
    }
    return true;
}

bool bmi088_read(imu_sample_t *s)
{
    uint8_t b[6];
    uint8_t i;

    if (!i2c_read_regs(ACC_ADDR, ACC_X_LSB, b, 6)) {
        return false;
    }
    for (i = 0; i < 3u; i++) {
        /* ±12 g: mg = raw × 12000 / 32768 = raw × 375 / 1024 */
        s->a[i] = (int16_t)(((int32_t)le16(&b[2u * i]) * 375) >> 10);
    }

    if (!i2c_read_regs(GYR_ADDR, GYR_RATE_X_LSB, b, 6)) {
        return false;
    }
    for (i = 0; i < 3u; i++) {
        /* ±1000 °/s: dps = raw × 1000 / 32768 = raw × 125 / 4096 */
        s->g[i] = (int16_t)(((int32_t)le16(&b[2u * i]) * 125) >> 12);
    }
    return true;
}
