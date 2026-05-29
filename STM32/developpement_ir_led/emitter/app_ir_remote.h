#ifndef APP_IR_REMOTE_H
#define APP_IR_REMOTE_H

#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t command;
} IR_ButtonMap;

typedef struct
{
    TIM_HandleTypeDef *carrier_timer;
    uint32_t carrier_channel;
    const IR_ButtonMap *buttons;
    uint8_t button_count;
    uint16_t address;
    uint32_t last_send_ms;
    uint8_t last_command;
} IR_Remote;

void IR_Remote_Init(IR_Remote *remote);
void IR_Remote_Task(IR_Remote *remote);
void IR_Remote_SendCommand(IR_Remote *remote, uint8_t command);

#ifdef __cplusplus
}
#endif

#endif
