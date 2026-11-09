/*
 * Paracaídas de emergencia para DJI Matrice 400 — firmware del PIC18F25K22.
 *
 * Estados:
 *   FALLA       IMU sin respuesta: no puede detectar, reintenta cada segundo. Nunca despliega.
 *   DESARMADO   en tierra (sin vibración de motores) o recién encendido.
 *   ARMADO      en vuelo: cualquier criterio del detector dispara el despliegue.
 *   DESPLEGADO  pulsos al solenoide y luego baliza (LED rojo + buzzer) hasta apagar.
 *
 * Variables persistentes (no se borran en un reset que no sea de encendido):
 *  - reset a mitad del despliegue (la batería cae durante el pulso) -> repite el pulso al arrancar;
 *  - reset en vuelo estando ARMADO (watchdog, brown-out) -> vuelve a ARMADO sin esperar el rearme.
 */
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "app_config.h"
#include "tick.h"
#include "i2c.h"
#include "uart.h"
#include "adc.h"
#include "bmi088.h"
#include "detector.h"

typedef enum {
    ST_FAULT,
    ST_DISARMED,
    ST_ARMED,
    ST_DEPLOYED
} state_t;

static const char *const state_name[] = { "FALLA", "DESARMADO", "ARMADO", "DESPLEGADO" };

/* No se inicializa al arrancar: sobrevive a un reset que no sea de encendido. */
#define DEPLOY_ACTIVE   0xD3A1u
#define DEPLOY_DONE     0x5AFEu
#define ARMED_MAGIC     0xA4A4u
__persistent uint16_t deploy_magic;
__persistent uint16_t armed_magic;      /* reset en vuelo -> volver a ARMADO sin esperar */

static state_t    state = ST_FAULT;
static detector_t det;
static bool       imu_ok;
static bool       low_batt;
static uint16_t   vbat_mv;
static uint8_t    last_cause;

/* Secuencia de pulsos del solenoide */
static uint8_t    pulses_left;
static bool       sol_on;
static uint32_t   sol_t;

#if BENCH_COMMANDS
static bool       bench_armed;
#endif

void __interrupt() isr(void)
{
    tick_isr();
}

static void set_state(state_t s)
{
    if (s != state) {
        state = s;
        if (s == ST_ARMED) {
            armed_magic = ARMED_MAGIC;
        } else if (s != ST_DEPLOYED) {
            armed_magic = 0;
        }
#if UART_LOG_ENABLE
        printf("[%lu] estado: %s\r\n", tick_ms(), state_name[s]);
#endif
    }
}

/* ---------------------------------------------------------------- despliegue */

static void deploy_start(uint32_t now, uint8_t cause)
{
    deploy_magic = DEPLOY_ACTIVE;
    last_cause = cause;
    SOL_LAT = 1;
    sol_on = true;
    sol_t = now;
    pulses_left = SOL_PULSE_COUNT - 1u;
    set_state(ST_DEPLOYED);
#if UART_LOG_ENABLE
    printf("[%lu] DESPLIEGUE causa=0x%02X (1=caida libre 2=giro 4=inclinacion)\r\n", now, cause);
#endif
}

static void deploy_update(uint32_t now)
{
    if (sol_on) {
        if ((uint32_t)(now - sol_t) >= SOL_PULSE_MS) {
            SOL_LAT = 0;
            sol_on = false;
            sol_t = now;
            if (pulses_left == 0u) {
                deploy_magic = DEPLOY_DONE;
            }
        }
    } else if (pulses_left != 0u && (uint32_t)(now - sol_t) >= SOL_PULSE_GAP_MS) {
        SOL_LAT = 1;
        sol_on = true;
        sol_t = now;
        pulses_left--;
    }
}

/* ---------------------------------------------------------------- indicadores */

static void indicators(uint32_t now)
{
    uint16_t p = (uint16_t)(now & 0x7FFu);     /* ciclo de 2.048 s */
    bool g = false, r = false, bz = false;

    switch (state) {
    case ST_DISARMED:                          /* verde: destello corto cada 2 s */
        g = p < 100u;
        r = low_batt && p >= 1000u && p < 1100u;
        break;
    case ST_ARMED:                             /* verde fijo */
        g = true;
        r = low_batt && p < 100u;
        break;
    case ST_DEPLOYED:                          /* rojo fijo + buzzer 1 Hz (baliza para encontrarlo) */
        r = true;
        bz = (now & 0x3FFu) < 0x200u;
        break;
    case ST_FAULT:                             /* rojo parpadeo rápido + bip cada 2 s */
    default:
        r = (now & 0xFFu) < 0x80u;
        bz = p < 60u;
        break;
    }
    LED_G_LAT = g;
    LED_R_LAT = r;
    BUZZER_LAT = bz;
}

/* ---------------------------------------------------------------- comandos de banco */

#if BENCH_COMMANDS
static void bench_poll(uint32_t now)
{
    static char line[12];
    static uint8_t len;
    char c;

    while (uart_getc(&c)) {
        if (c == '\r' || c == '\n') {
            line[len] = '\0';
            if (strcmp(line, "ARM") == 0 && state == ST_DISARMED) {
                bench_armed = true;
                set_state(ST_ARMED);
            } else if (strcmp(line, "DISARM") == 0 && state == ST_ARMED) {
                bench_armed = false;
                set_state(ST_DISARMED);
            } else if (strcmp(line, "FIRE") == 0 && state != ST_DEPLOYED) {
                deploy_start(now, 0);
            }
            len = 0;
        } else if (len < sizeof(line) - 1u) {
            line[len++] = c;
        } else {
            len = 0;
        }
    }
}
#endif

/* ---------------------------------------------------------------- main */

void main(void)
{
    bool power_on = (RCON & 0x02u) == 0u;       /* POR = 0 -> reset por encendido */
    uint32_t now, t_sample, t_log, t_bat, t_retry;
    uint16_t fails = 0;
    bool resume_armed;

    RCON |= 0x03u;                              /* rearma los flags POR/BOR */
    board_init();
    tick_init();
    uart_init();
    adc_init();
    i2c_init();
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    WDTCONbits.SWDTEN = 1;                      /* watchdog ~1 s */

#if UART_LOG_ENABLE
    printf("\r\nParacaidas M400 fw %s (reset: %s)\r\n", FW_VERSION, power_on ? "encendido" : "otro");
#endif

    now = tick_ms();
    resume_armed = !power_on && armed_magic == ARMED_MAGIC;
    if (!power_on && deploy_magic == DEPLOY_ACTIVE) {
        /* Reset a mitad del pulso: repetir el despliegue sin esperar nada. */
        deploy_start(now, last_cause);
    } else if (!power_on && deploy_magic == DEPLOY_DONE) {
        set_state(ST_DEPLOYED);                 /* ya desplegado: mantener la baliza */
    } else if (resume_armed) {
        deploy_magic = 0;                       /* sin prueba de LEDs: no perder tiempo */
    } else {
        deploy_magic = 0;
        armed_magic = 0;
        LED_G_LAT = 1;                          /* prueba de LEDs y buzzer */
        LED_R_LAT = 1;
        BUZZER_LAT = 1;
        __delay_ms(200);
        LED_G_LAT = 0;
        LED_R_LAT = 0;
        BUZZER_LAT = 0;
    }

    detector_init(&det);
    imu_ok = bmi088_init();
    if (imu_ok && state == ST_FAULT) {
        if (resume_armed) {
            det.in_flight = true;               /* reset en vuelo: seguir armado */
            set_state(ST_ARMED);
        } else {
            set_state(ST_DISARMED);
        }
    }
#if UART_LOG_ENABLE
    printf("IMU BMI088: %s\r\n", imu_ok ? "OK" : "SIN RESPUESTA");
#endif

    now = tick_ms();
    t_sample = t_log = t_bat = t_retry = now;
    vbat_mv = adc_vbat_mv();

    for (;;) {
        CLRWDT();
        now = tick_ms();
        deploy_update(now);

        /* ---- IMU + detección ---- */
        if ((uint32_t)(now - t_sample) >= SAMPLE_PERIOD_MS) {
            uint16_t dt = (uint16_t)(now - t_sample);
            t_sample = now;

            if (imu_ok) {
                imu_sample_t s;
                if (bmi088_read(&s)) {
                    uint8_t cause = detector_update(&det, &s, dt);
                    bool flying = detector_in_flight(&det);
                    fails = 0;
#if BENCH_COMMANDS
                    flying = flying || bench_armed;
#endif
                    if (state == ST_DISARMED && flying && now >= ARM_MIN_UPTIME_MS) {
                        set_state(ST_ARMED);
                    } else if (state == ST_ARMED && !flying) {
                        set_state(ST_DISARMED);
                    }
                    if (state == ST_ARMED && cause != 0u) {
                        deploy_start(now, cause);
                    }
                } else if (++fails >= IMU_FAIL_LIMIT) {
                    imu_ok = false;
                    t_retry = now;
                    i2c_recover();
                    if (state != ST_DEPLOYED) {
                        set_state(ST_FAULT);
                    }
                }
            } else if (state != ST_DEPLOYED && (uint32_t)(now - t_retry) >= IMU_RETRY_MS) {
                t_retry = now;
                i2c_recover();
                if (bmi088_init()) {
                    imu_ok = true;
                    fails = 0;
                    detector_init(&det);
                    set_state(ST_DISARMED);
                }
            }
        }

        /* ---- batería ---- */
        if ((uint32_t)(now - t_bat) >= VBAT_PERIOD_MS) {
            t_bat = now;
            vbat_mv = adc_vbat_mv();
            low_batt = vbat_mv < VBAT_LOW_MV;
        }

#if UART_LOG_ENABLE
        /* ---- log de estado: t, estado, |a| mg, |w| dps, vibración mg, en vuelo, VBAT mV ---- */
        if ((uint32_t)(now - t_log) >= LOG_PERIOD_MS) {
            t_log = now;
            printf("[%lu] %s a=%u g=%u vib=%u vuelo=%u vbat=%u\r\n", now, state_name[state],
                   detector_acc_mg(&det), detector_gyro_dps(&det), detector_vib_mg(&det),
                   (unsigned)detector_in_flight(&det), vbat_mv);
        }
#endif

#if BENCH_COMMANDS
        bench_poll(now);
#endif
        indicators(now);
    }
}
