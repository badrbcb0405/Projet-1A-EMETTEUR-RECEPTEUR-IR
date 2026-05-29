#include "app_ir_receiver.h"

#define NEC_TOLERANCE_PERCENT 28u
#define NEC_FRAME_TIMEOUT_US  14000u
#define PERCENT_SCALE           100u
#define TIMER_WRAP_INCREMENT      1u
#define IR_RX_RAW_ONE_BIT       1UL

static uint32_t ir_now_us(IR_Receiver *rx)
{
    return __HAL_TIM_GET_COUNTER(rx->timebase_timer);
}

static uint32_t elapsed_us(IR_Receiver *rx, uint32_t now, uint32_t before)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(rx->timebase_timer);

    if (now >= before)
    {
        return now - before;
    }

    return (arr - before) + now + TIMER_WRAP_INCREMENT;
}

static uint8_t in_range(uint32_t value, uint32_t target)
{
    uint32_t tol = (target * NEC_TOLERANCE_PERCENT) / PERCENT_SCALE;
    return (value >= (target - tol)) && (value <= (target + tol));
}

static void reset_decoder(IR_Receiver *rx)
{
    rx->state = IR_RX_IDLE;
    rx->raw = 0u;
    rx->bit_index = 0u;
}

void IR_Receiver_Init(IR_Receiver *rx)
{
    HAL_TIM_Base_Start(rx->timebase_timer);
    rx->last_edge_us = ir_now_us(rx);
    rx->last_level = HAL_GPIO_ReadPin(rx->ir_port, rx->ir_pin);
    rx->frame_ready = 0u;
    rx->repeat_ready = 0u;
    rx->last_frame_ms = 0u;
    reset_decoder(rx);
}

void IR_Receiver_OnExti(IR_Receiver *rx, uint16_t gpio_pin)
{
    if (gpio_pin != rx->ir_pin)
    {
        return;
    }

    uint32_t now = ir_now_us(rx);
    uint32_t duration = elapsed_us(rx, now, rx->last_edge_us);
    GPIO_PinState current_level = HAL_GPIO_ReadPin(rx->ir_port, rx->ir_pin);
    GPIO_PinState previous_level = rx->last_level;

    rx->last_edge_us = now;
    rx->last_level = current_level;

    /*
     * Les modules IR demodules sont en general actifs bas :
     * - niveau bas = burst IR recu ;
     * - niveau haut = silence entre deux bursts.
     * A chaque front, on analyse donc la duree du niveau precedent.
     */
    switch (rx->state)
    {
    case IR_RX_IDLE:
        if (previous_level == GPIO_PIN_RESET && in_range(duration, IR_NEC_HEADER_MARK_US))
        {
            rx->state = IR_RX_HEADER_SPACE;
        }
        break;

    case IR_RX_HEADER_SPACE:
        if (previous_level != GPIO_PIN_SET)
        {
            reset_decoder(rx);
            break;
        }

        if (in_range(duration, IR_NEC_HEADER_SPACE_US))
        {
            rx->state = IR_RX_BIT_MARK;
            rx->raw = 0u;
            rx->bit_index = 0u;
        }
        else if (in_range(duration, IR_NEC_REPEAT_SPACE_US))
        {
            rx->repeat_ready = 1u;
            reset_decoder(rx);
        }
        else
        {
            reset_decoder(rx);
        }
        break;

    case IR_RX_BIT_MARK:
        if (previous_level == GPIO_PIN_RESET && in_range(duration, IR_NEC_BIT_MARK_US))
        {
            rx->state = IR_RX_BIT_SPACE;
        }
        else
        {
            reset_decoder(rx);
        }
        break;

    case IR_RX_BIT_SPACE:
        if (previous_level != GPIO_PIN_SET)
        {
            reset_decoder(rx);
            break;
        }

        if (in_range(duration, IR_NEC_ZERO_SPACE_US))
        {
            rx->state = IR_RX_BIT_MARK;
            rx->bit_index++;
        }
        else if (in_range(duration, IR_NEC_ONE_SPACE_US))
        {
            rx->raw |= (IR_RX_RAW_ONE_BIT << rx->bit_index);
            rx->state = IR_RX_BIT_MARK;
            rx->bit_index++;
        }
        else
        {
            reset_decoder(rx);
            break;
        }

        if (rx->bit_index >= IR_NEC_BITS_PER_FRAME)
        {
            IR_NEC_Frame frame;
            if (IR_NEC_Unpack(rx->raw, &frame))
            {
                rx->frame = frame;
                rx->frame_ready = 1u;
                rx->last_frame_ms = HAL_GetTick();
            }
            reset_decoder(rx);
        }
        break;

    default:
        reset_decoder(rx);
        break;
    }
}

uint8_t IR_Receiver_ReadFrame(IR_Receiver *rx, IR_NEC_Frame *frame)
{
    if (rx->frame_ready == 0u)
    {
        return 0u;
    }

    __disable_irq();
    if (frame != 0)
    {
        *frame = rx->frame;
    }
    rx->frame_ready = 0u;
    __enable_irq();

    return 1u;
}

uint8_t IR_Receiver_ReadRepeat(IR_Receiver *rx)
{
    if (rx->repeat_ready == 0u)
    {
        return 0u;
    }

    __disable_irq();
    rx->repeat_ready = 0u;
    __enable_irq();

    return 1u;
}

void IR_Receiver_Task(IR_Receiver *rx)
{
    uint32_t now = ir_now_us(rx);
    uint32_t elapsed = elapsed_us(rx, now, rx->last_edge_us);

    if (rx->state != IR_RX_IDLE && elapsed > NEC_FRAME_TIMEOUT_US)
    {
        reset_decoder(rx);
    }
}
