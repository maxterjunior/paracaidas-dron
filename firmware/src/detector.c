/*
 * Detección de emergencia con aritmética entera (el PIC18 no tiene FPU).
 *
 *  1. Caída libre:     |a| < FF_THRESH_MG          durante FF_TIME_MS
 *  2. Giro anormal:    |w| > TUMBLE_THRESH_DPS     durante TUMBLE_TIME_MS
 *  3. Inclinación:     ángulo con la vertical > 70° durante TILT_TIME_MS
 *
 * "En vuelo" se infiere de la vibración de los motores (componente de alta
 * frecuencia de la aceleración), para no armar el sistema en tierra.
 */
#include <string.h>
#include "detector.h"
#include "app_config.h"

#define FP 4   /* bits fraccionarios de los filtros */

static uint16_t sat_add(uint16_t c, uint16_t dt)
{
    uint32_t s = (uint32_t)c + dt;
    return (s > 0xFFFFu) ? 0xFFFFu : (uint16_t)s;
}

static int32_t iabs32(int32_t v)
{
    return (v < 0) ? -v : v;
}

static uint16_t isqrt32(uint32_t v)
{
    uint32_t r = 0, bit = 1ul << 30;
    while (bit > v) {
        bit >>= 2;
    }
    while (bit != 0u) {
        if (v >= r + bit) {
            v -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (uint16_t)r;
}

void detector_init(detector_t *d)
{
    memset(d, 0, sizeof(*d));
}

uint8_t detector_update(detector_t *d, const imu_sample_t *s, uint16_t dt_ms)
{
    int32_t m[3];
    int32_t hf = 0, mag2 = 0, gyr2 = 0;
    uint8_t i, cause = 0;

    if (!d->primed) {
        for (i = 0; i < 3u; i++) {
            d->fa[i] = (int32_t)s->a[i] << FP;
            d->fg[i] = (int32_t)s->g[i] << FP;
        }
        d->primed = true;
    }

    for (i = 0; i < 3u; i++) {
        int32_t x = (int32_t)s->a[i] << FP;
        int32_t w = (int32_t)s->g[i] << FP;
        int32_t gv;

        hf += iabs32(x - d->fa[i]);                     /* alta frecuencia = vibración */
        d->fa[i] += (x - d->fa[i]) >> ACC_IIR_SHIFT;
        d->fg[i] += (w - d->fg[i]) >> GYR_IIR_SHIFT;

        m[i] = d->fa[i] >> FP;
        gv = d->fg[i] >> FP;
        mag2 += m[i] * m[i];
        gyr2 += gv * gv;
    }
    d->mag2 = mag2;
    d->gyr2 = gyr2;

    /* ---- vibración -> en vuelo / en tierra ---- */
    d->vib += (hf - d->vib) >> VIB_IIR_SHIFT;
    {
        int32_t v = d->vib >> FP;
        if (v >= VIB_ARM_MG) {
            d->vib_hi_ms = sat_add(d->vib_hi_ms, dt_ms);
            d->vib_lo_ms = 0;
        } else if (v < VIB_DISARM_MG) {
            d->vib_lo_ms = sat_add(d->vib_lo_ms, dt_ms);
            d->vib_hi_ms = 0;
        }
        if (!d->in_flight && d->vib_hi_ms >= VIB_ARM_TIME_MS) {
            d->in_flight = true;
        } else if (d->in_flight && d->vib_lo_ms >= VIB_DISARM_TIME_MS) {
            d->in_flight = false;
        }
    }

    /* ---- 1. caída libre ---- */
    if (mag2 < (int32_t)FF_THRESH_MG * FF_THRESH_MG) {
        d->ff_ms = sat_add(d->ff_ms, dt_ms);
    } else {
        d->ff_ms = 0;
    }
    if (d->ff_ms >= FF_TIME_MS) {
        cause |= DET_FREEFALL;
    }

    /* ---- 2. giro descontrolado ---- */
    if (gyr2 > (int32_t)TUMBLE_THRESH_DPS * TUMBLE_THRESH_DPS) {
        d->tumble_ms = sat_add(d->tumble_ms, dt_ms);
    } else {
        d->tumble_ms = 0;
    }
    if (d->tumble_ms >= TUMBLE_TIME_MS) {
        cause |= DET_TUMBLE;
    }

    /* ---- 3. inclinación: cos(θ) = a_up / |a|;  θ > 70°  <=>  a_up < 0  o  a_up² < cos²·|a|² ---- */
    if (mag2 > (int32_t)TILT_MIN_MG * TILT_MIN_MG) {
        int32_t up = (int32_t)TILT_UP_SIGN * m[TILT_UP_AXIS];
        bool tilted = (up < 0) || (up * up < (mag2 / 1000) * TILT_COS2_X1000);
        d->tilt_ms = tilted ? sat_add(d->tilt_ms, dt_ms) : 0u;
    } else {
        d->tilt_ms = 0;
    }
    if (d->tilt_ms >= TILT_TIME_MS) {
        cause |= DET_TILT;
    }

    return cause;
}

bool detector_in_flight(const detector_t *d)
{
    return d->in_flight;
}

uint16_t detector_acc_mg(const detector_t *d)
{
    return isqrt32((uint32_t)d->mag2);
}

uint16_t detector_gyro_dps(const detector_t *d)
{
    return isqrt32((uint32_t)d->gyr2);
}

uint16_t detector_vib_mg(const detector_t *d)
{
    return (uint16_t)(d->vib >> FP);
}
