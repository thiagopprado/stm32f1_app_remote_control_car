#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "rcc.h"
#include "timer.h"

#include "infrared.h"
#include "buzzer.h"

/** Definitions --------------------------------------------------- */

/** Types --------------------------------------------------------- */

/** Variables ----------------------------------------------------- */
static volatile uint32_t timer_counter = 0;

/** Prototypes ---------------------------------------------------- */
static void timer_callback(void);

/** Internal functions -------------------------------------------- */
/**
 * @brief Timer callback.
 * 
 * Executed every 1ms.
 */
static void timer_callback(void) {
    timer_counter++;
}

/** Public functions ---------------------------------------------- */
int main(void) {
    uint32_t timeshot = 0;

    rcc_clock_init();

    infrared_setup();
    buzzer_setup();

    gpio_setup(GPIO_PORTA, 6, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);
    gpio_setup(GPIO_PORTA, 7, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);
    gpio_setup(GPIO_PORTB, 0, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);
    gpio_setup(GPIO_PORTB, 1, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);

    timer_setup(TIMER_3, 71, 999);
    timer_pwm_setup(TIMER_3, TIMER_CH_1);
    timer_pwm_setup(TIMER_3, TIMER_CH_2);
    timer_pwm_setup(TIMER_3, TIMER_CH_3);
    timer_pwm_setup(TIMER_3, TIMER_CH_4);

    timer_attach_callback(TIMER_3, timer_callback);

    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);

    while (true) {
        uint32_t time_diff = timer_counter - timeshot;

        if (time_diff > 200) {
            timeshot = timer_counter;

            ir_key_id_t key_pressed = infrared_decode();

            switch (key_pressed) {
                case INFRARED_KEY_UP: {
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 999);

                    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 999);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);
                    break;
                }
                case INFRARED_KEY_DOWN: {
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 999);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 0);

                    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 999);
                    break;
                }
                case INFRARED_KEY_LEFT: {
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 0);

                    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 999);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);
                    break;
                }
                case INFRARED_KEY_RIGHT: {
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 999);

                    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);
                    break;
                }
                default: {
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 0);
                    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);
                    break;
                }
            }

            if (key_pressed == INFRARED_KEY_ENTER) {
                buzzer_play_note(BUZZER_NOTE_A4);
            } else {
                buzzer_play_note(BUZZER_NOTE_ST);
            }
        }
    }
}
