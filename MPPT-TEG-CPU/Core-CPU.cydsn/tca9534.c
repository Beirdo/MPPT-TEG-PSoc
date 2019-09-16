/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "project.h"
#include "tca9534.h"
#include "i2cRegisters.h"

tca9534Control_t tca9534Control[MAX_TCA9534_COUNT];
    
void TCA9534_initialize(uint8 index, uint8 addr, uint8 readMask, uint8 writeMask) {
    if (index >= MAX_TCA9534_COUNT) {
        return;
    }
    
    tca9534Control_t *control = &tca9534Control[index];
    control->addr = addr;
    control->readMask = readMask;
    control->writeMask = writeMask;
    
    i2c_register_write(addr, 0x03, ~writeMask);  // outputs to 0, inputs 1, unused as inputs
    TCA9534_write_outputs(index, 0x00); // drive all outputs low.
}

uint8 TCA9534_read_inputs(uint8 index) {
    if (index >= MAX_TCA9534_COUNT) {
        return 0;
    }

    tca9534Control_t *control = &tca9534Control[index];
    return i2c_register_read(control->addr, 0x00) & control->readMask;
}
   
void TCA9534_write_outputs(uint8 index, uint8 value) {
    if (index >= MAX_TCA9534_COUNT) {
        return;
    }

    tca9534Control_t *control = &tca9534Control[index];
    control->value = value & control->writeMask;
    i2c_register_write(control->addr, 0x01, control->value);
}

void TCA9534_set_bit(uint8 index, uint8 bitnum) {
    if (index >= MAX_TCA9534_COUNT || bitnum > 7) {
        return;
    }

    tca9534Control_t *control = &tca9534Control[index];
    uint8 bitvalue = 1 << bitnum;
    control->value |= bitvalue;
    control->value &= control->writeMask;
    i2c_register_write(control->addr, 0x01, control->value);
}

void TCA9534_clear_bit(uint8 index, uint8 bitnum) {
    if (index >= MAX_TCA9534_COUNT || bitnum > 7) {
        return;
    }

    tca9534Control_t *control = &tca9534Control[index];
    uint8 bitvalue = 1 << bitnum;
    control->value &= ~bitvalue;
    control->value &= control->writeMask;
    i2c_register_write(control->addr, 0x01, control->value);
}

void TCA9534_write_bit(uint8 index, uint8 bitnum, uint8 value) {
    if (value) {
        TCA9534_set_bit(index, bitnum);
    } else {
        TCA9534_clear_bit(index, bitnum);
    }
}

// Don't mask off the outputs, let us read them back too
uint8 TCA9534_read_bit(uint8 index, uint8 bitnum) {
    if (index >= MAX_TCA9534_COUNT || bitnum > 7) {
        return 0;
    }

    tca9534Control_t *control = &tca9534Control[index];
    uint8 bitvalue = 1 << bitnum;
    bitvalue &= i2c_register_read(control->addr, 0x00);
    return !(!bitvalue);
}


/* [] END OF FILE */
