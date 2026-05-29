/*
 * Exemple d'integration dans Core/Src/main.c du projet STM32CubeIDE emetteur.
 * Garde les fonctions SystemClock_Config(), MX_GPIO_Init(), MX_TIMx_Init()
 * generees par CubeMX, puis ajoute les blocs USER CODE ci-dessous.
 */

#include "main.h"
#include "app_ir_remote.h"
#include "ir_nec.h"

extern TIM_HandleTypeDef htim1; /* Timer PWM 38 kHz pour LED IR */

#define IR_REMOTE_TASK_PERIOD_MS 5u

/*
 * Exemple de mapping GPIO.
 * Les noms BUTTON1 a BUTTON4 correspondent aux labels de la carte KiCad.
 */
static const IR_ButtonMap remote_buttons[] =
{
    {BUTTON1_GPIO_Port, BUTTON1_Pin, IR_CMD_POWER},
    {BUTTON2_GPIO_Port, BUTTON2_Pin, IR_CMD_BRIGHTNESS_UP},
    {BUTTON3_GPIO_Port, BUTTON3_Pin, IR_CMD_BRIGHTNESS_DOWN},
    {BUTTON4_GPIO_Port, BUTTON4_Pin, IR_CMD_MODE},
};

static IR_Remote remote =
{
    .carrier_timer = &htim1,
    .carrier_channel = TIM_CHANNEL_1,
    .buttons = remote_buttons,
    .button_count = sizeof(remote_buttons) / sizeof(remote_buttons[0]),
    .address = IR_NEC_ADDRESS,
};

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM1_Init();

    IR_Remote_Init(&remote);

    while (1)
    {
        IR_Remote_Task(&remote);
        HAL_Delay(IR_REMOTE_TASK_PERIOD_MS);
    }
}

/*
 * Reglage timer PWM 38 kHz :
 *
 * Frequence_PWM = Frequence_Timer / ((PSC + 1) * (ARR + 1))
 *
 * Exemple si timer clock = 80 MHz :
 * PSC = 0
 * ARR = 2104     -> 80 MHz / 2105 = 38005 Hz
 * CCR1 = ARR / 3 -> duty cycle environ 33 %
 */
