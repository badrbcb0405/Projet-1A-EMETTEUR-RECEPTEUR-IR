#ifndef APP_IR_RECEIVER_H
#define APP_IR_RECEIVER_H

#include "main.h"
#include "ir_nec.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IR_RX_IDLE = 0,
    IR_RX_HEADER_SPACE,
    IR_RX_BIT_MARK,
    IR_RX_BIT_SPACE,
} IR_RxState;

typedef struct
{
    GPIO_TypeDef *ir_port;
    uint16_t ir_pin;
    TIM_HandleTypeDef *timebase_timer;
    IR_RxState state;
    uint32_t last_edge_us;
    GPIO_PinState last_level;
    uint32_t raw;
    uint8_t bit_index;
    volatile uint8_t frame_ready;
    volatile uint8_t repeat_ready;
    IR_NEC_Frame frame;
    uint32_t last_frame_ms;
} IR_Receiver;

void IR_Receiver_Init(IR_Receiver *rx);
void IR_Receiver_OnExti(IR_Receiver *rx, uint16_t gpio_pin);
uint8_t IR_Receiver_ReadFrame(IR_Receiver *rx, IR_NEC_Frame *frame);
uint8_t IR_Receiver_ReadRepeat(IR_Receiver *rx);
void IR_Receiver_Task(IR_Receiver *rx);

#ifdef __cplusplus
}
#endif

#endif
