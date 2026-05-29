#include "app_ir_remote.h"
#include "ir_nec.h"

#ifndef IR_BUTTON_DEBOUNCE_MS
#define IR_BUTTON_DEBOUNCE_MS 180u
#endif

#define IR_US_PER_SECOND       1000000u
#define IR_US_PER_MS              1000u
#define IR_IDLE_COMMAND           0u

static void ir_dwt_init(void)
{
#if defined(DWT) && defined(CoreDebug_DEMCR_TRCENA_Msk)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
}

static void ir_delay_us(uint32_t us)
{
#if defined(DWT) && defined(DWT_CTRL_CYCCNTENA_Msk)
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (HAL_RCC_GetHCLKFreq() / IR_US_PER_SECOND) * us;
    while ((uint32_t)(DWT->CYCCNT - start) < ticks)
    {
    }
#else
    uint32_t start = HAL_GetTick();
    uint32_t ms = (us + (IR_US_PER_MS - 1u)) / IR_US_PER_MS;
    while ((uint32_t)(HAL_GetTick() - start) < ms)
    {
    }
#endif
}

static void ir_carrier_on(IR_Remote *remote)
{
    HAL_TIM_PWM_Start(remote->carrier_timer, remote->carrier_channel);
}

static void ir_carrier_off(IR_Remote *remote)
{
    HAL_TIM_PWM_Stop(remote->carrier_timer, remote->carrier_channel);
}

static void ir_mark(IR_Remote *remote, uint32_t duration_us)
{
    ir_carrier_on(remote);
    ir_delay_us(duration_us);
}

static void ir_space(IR_Remote *remote, uint32_t duration_us)
{
    ir_carrier_off(remote);
    ir_delay_us(duration_us);
}

static void ir_send_raw_nec(IR_Remote *remote, uint32_t raw)
{
    ir_mark(remote, IR_NEC_HEADER_MARK_US);
    ir_space(remote, IR_NEC_HEADER_SPACE_US);

    for (uint8_t i = 0u; i < IR_NEC_BITS_PER_FRAME; i++)
    {
        ir_mark(remote, IR_NEC_BIT_MARK_US);
        if (((raw >> i) & IR_NEC_BIT_MASK) != 0u)
        {
            ir_space(remote, IR_NEC_ONE_SPACE_US);
        }
        else
        {
            ir_space(remote, IR_NEC_ZERO_SPACE_US);
        }
    }

    ir_mark(remote, IR_NEC_STOP_MARK_US);
    ir_carrier_off(remote);
}

void IR_Remote_Init(IR_Remote *remote)
{
    ir_dwt_init();
    ir_carrier_off(remote);
    remote->last_send_ms = 0u;
    remote->last_command = IR_IDLE_COMMAND;
}

void IR_Remote_SendCommand(IR_Remote *remote, uint8_t command)
{
    uint32_t raw = IR_NEC_Pack(remote->address, command);
    ir_send_raw_nec(remote, raw);
    remote->last_send_ms = HAL_GetTick();
    remote->last_command = command;
}

void IR_Remote_Task(IR_Remote *remote)
{
    uint32_t now = HAL_GetTick();

    for (uint8_t i = 0u; i < remote->button_count; i++)
    {
        const IR_ButtonMap *button = &remote->buttons[i];

        if (HAL_GPIO_ReadPin(button->port, button->pin) == GPIO_PIN_RESET)
        {
            if ((uint32_t)(now - remote->last_send_ms) >= IR_BUTTON_DEBOUNCE_MS
                || remote->last_command != button->command)
            {
                IR_Remote_SendCommand(remote, button->command);
            }
            return;
        }
    }

    remote->last_command = IR_IDLE_COMMAND;
}
