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
#include "tca9546.h"
#include "interprocessor.h"
#include "i2cRegisters.h"

static uint8 connected = 0;

void TCA9546_select_bus(int board, int interface) {
    if (board >= INTERFACE_BOARD_COUNT || interface >= INTERFACE_COUNT_PER_BOARD) {
        return;
    }

    interface = board * INTERFACE_COUNT_PER_BOARD + interface;
    uint8 value = 1 << interface;
    uint8 addr = TCA9546_BASE_ADDR + board;
    
    if (connected == value) {
        return;
    }
    
    // Reset as we have two of these beasts, and only one will be used at a time
    TCA9546_reset();
    
    // First use the TCA9546 to switch the I2C bus over to the channel
    i2c_register_write_noreg(addr, value & INTERFACE_MASK); 
    
    connected = value;
}

void TCA9546_reset(void) {
    I2C_RESET_Write(1);
    I2C_RESET_Write(0);
    I2C_RESET_Write(1);
    connected = 0;
}

/* [] END OF FILE */
