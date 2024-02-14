#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "timer.h"

#include "infrared.h"
#include "buzzer.h"

#include "stm32f1xx_hal.h"

/** Definitions --------------------------------------------------- */
#define GPIO_MOTOR_1_CLOCK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define PWM_MOTOR_1_PORT            GPIOA
#define PWM_I1_PIN                  GPIO_PIN_6
#define PWM_I2_PIN                  GPIO_PIN_7

#define GPIO_MOTOR_2_CLOCK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define PWM_MOTOR_2_PORT            GPIOB
#define PWM_I3_PIN                  GPIO_PIN_0
#define PWM_I4_PIN                  GPIO_PIN_1

/** Types --------------------------------------------------------- */

/** Variables ----------------------------------------------------- */

/** Prototypes ---------------------------------------------------- */
static void clock_config(void);

/** Internal functions -------------------------------------------- */
/**
 * @brief MCU clock configuration.
 */
static void clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/** Public functions ---------------------------------------------- */
int main(void) {
    uint32_t timeshot = 0;

    HAL_Init();
    clock_config();

    infrared_setup();
    buzzer_setup();

    GPIO_MOTOR_1_CLOCK_ENABLE();
    GPIO_MOTOR_2_CLOCK_ENABLE();

    GPIO_InitTypeDef gpio_init;
    gpio_init.Pin = PWM_I1_PIN | PWM_I2_PIN;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(PWM_MOTOR_1_PORT, &gpio_init);

    gpio_init.Pin = PWM_I3_PIN | PWM_I4_PIN;
    HAL_GPIO_Init(PWM_MOTOR_2_PORT, &gpio_init);

    timer_setup(TIMER_3, 71, 999);
    timer_pwm_setup(TIMER_3, TIMER_CH_1);
    timer_pwm_setup(TIMER_3, TIMER_CH_2);
    timer_pwm_setup(TIMER_3, TIMER_CH_3);
    timer_pwm_setup(TIMER_3, TIMER_CH_4);

    timer_attach_callback(TIMER_3, NULL);

    timer_pwm_set_duty(TIMER_3, TIMER_CH_1, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_2, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_3, 0);
    timer_pwm_set_duty(TIMER_3, TIMER_CH_4, 0);

    while (true) {
        if (HAL_GetTick() - timeshot > 200) {
            timeshot = HAL_GetTick();

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
