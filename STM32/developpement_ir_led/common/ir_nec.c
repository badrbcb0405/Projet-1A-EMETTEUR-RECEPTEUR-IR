#include "ir_nec.h"

uint32_t IR_NEC_Pack(uint16_t address, uint8_t command)
{
    uint8_t addr_low = (uint8_t)(address & IR_NEC_BYTE_MASK);
    uint8_t addr_high = (uint8_t)((address >> IR_NEC_BYTE_WIDTH_BITS) & IR_NEC_BYTE_MASK);
    uint8_t command_inv = (uint8_t)~command;

    return ((uint32_t)addr_low)
         | ((uint32_t)addr_high << IR_NEC_BYTE_WIDTH_BITS)
         | ((uint32_t)command << IR_NEC_COMMAND_SHIFT_BITS)
         | ((uint32_t)command_inv << IR_NEC_COMMAND_INV_SHIFT_BITS);
}

uint8_t IR_NEC_Unpack(uint32_t raw, IR_NEC_Frame *frame)
{
    uint8_t addr_low = (uint8_t)(raw & IR_NEC_BYTE_MASK);
    uint8_t addr_high = (uint8_t)((raw >> IR_NEC_BYTE_WIDTH_BITS) & IR_NEC_BYTE_MASK);
    uint8_t command = (uint8_t)((raw >> IR_NEC_COMMAND_SHIFT_BITS) & IR_NEC_BYTE_MASK);
    uint8_t command_inv = (uint8_t)((raw >> IR_NEC_COMMAND_INV_SHIFT_BITS) & IR_NEC_BYTE_MASK);

    if ((uint8_t)(command ^ command_inv) != IR_NEC_BYTE_MASK)
    {
        return 0u;
    }

    if (frame != 0)
    {
        frame->address = (uint16_t)addr_low | ((uint16_t)addr_high << IR_NEC_BYTE_WIDTH_BITS);
        frame->command = command;
    }

    return 1u;
}
