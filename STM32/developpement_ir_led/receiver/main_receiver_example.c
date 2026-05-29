/*
 * Exemple d'integration dans Core/Src/main.c du projet STM32CubeIDE recepteur.
 * Garde les fonctions SystemClock_Config(), MX_GPIO_Init(), MX_TIMx_Init()
 * generees par CubeMX, puis ajoute les blocs USER CODE ci-dessous.
 */

#include "main.h"
#include "app_ir_receiver.h"
#include "app_ws2812.h"
#include "ir_nec.h"

extern TIM_HandleTypeDef htim2; /* Timer base de temps 1 MHz */

static IR_Receiver ir_rx =
{
    .ir_port = IR_IN_GPIO_Port,
    .ir_pin = IR_IN_Pin,
    .timebase_timer = &htim2,
};

static WS2812_Strip led_strip =
{
    .data_port = LED_DATA_GPIO_Port,
    .data_pin = LED_DATA_Pin,
};

static uint8_t last_command = 0u;

static void handle_ir_command(uint8_t command)
{
    last_command = command;

    switch (command)
    {
    case IR_CMD_POWER:
        WS2812_Strip_Toggle(&led_strip);
        break;

    case IR_CMD_BRIGHTNESS_UP:
        WS2812_Strip_SetEnabled(&led_strip, WS2812_ENABLED);
        WS2812_Strip_BrightnessUp(&led_strip);
        break;

    case IR_CMD_BRIGHTNESS_DOWN:
        WS2812_Strip_BrightnessDown(&led_strip);
        break;

    case IR_CMD_MODE:
        WS2812_Strip_SetEnabled(&led_strip, WS2812_ENABLED);
        WS2812_Strip_NextMode(&led_strip);
        break;

    default:
        break;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM2_Init();

    IR_Receiver_Init(&ir_rx);
    WS2812_Strip_Init(&led_strip);

    while (1)
    {
        IR_NEC_Frame frame;

        IR_Receiver_Task(&ir_rx);
        WS2812_Strip_Task(&led_strip);

        if (IR_Receiver_ReadFrame(&ir_rx, &frame))
        {
            if (frame.address == IR_NEC_ADDRESS)
            {
                handle_ir_command(frame.command);
            }
        }

        /*
         * Option : maintien d'un bouton.
         * On repete seulement +/- pour eviter de basculer POWER ou MODE en boucle.
         */
        if (IR_Receiver_ReadRepeat(&ir_rx))
        {
            if (last_command == IR_CMD_BRIGHTNESS_UP
                || last_command == IR_CMD_BRIGHTNESS_DOWN)
            {
                handle_ir_command(last_command);
            }
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    IR_Receiver_OnExti(&ir_rx, GPIO_Pin);
}

/*
 * Reglage timer base de temps 1 MHz :
 *
 * Frequence_compteur = Frequence_Timer / (PSC + 1) = 1 MHz
 * ARR peut valoir 0xFFFF ou 0xFFFFFFFF selon le timer.
 *
 * Exemple si timer clock = 80 MHz :
 * PSC = 79
 * ARR = 4294967295 pour TIM2
 *
 * La ligne LED_DATA est une sortie GPIO vers l'entree DIN des WS2812B.
 */
