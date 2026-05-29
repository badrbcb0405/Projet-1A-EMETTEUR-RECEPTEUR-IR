#ifndef IR_NEC_H
#define IR_NEC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IR_NEC_ADDRESS          0x10EFu

#define IR_CMD_POWER            0x45u
#define IR_CMD_BRIGHTNESS_UP    0x46u
#define IR_CMD_BRIGHTNESS_DOWN  0x47u
#define IR_CMD_MODE             0x44u

#define IR_NEC_HEADER_MARK_US   9000u
#define IR_NEC_HEADER_SPACE_US  4500u
#define IR_NEC_BIT_MARK_US       560u
#define IR_NEC_ZERO_SPACE_US     560u
#define IR_NEC_ONE_SPACE_US     1690u
#define IR_NEC_STOP_MARK_US      560u
#define IR_NEC_REPEAT_SPACE_US  2250u

#define IR_NEC_BITS_PER_FRAME     32u
#define IR_NEC_BYTE_WIDTH_BITS     8u
#define IR_NEC_BYTE_MASK        0xFFu
#define IR_NEC_BIT_MASK         0x01u
#define IR_NEC_COMMAND_SHIFT_BITS 16u
#define IR_NEC_COMMAND_INV_SHIFT_BITS 24u

typedef struct
{
    uint16_t address;
    uint8_t command;
} IR_NEC_Frame;

uint32_t IR_NEC_Pack(uint16_t address, uint8_t command);
uint8_t IR_NEC_Unpack(uint32_t raw, IR_NEC_Frame *frame);

#ifdef __cplusplus
}
#endif

#endif
