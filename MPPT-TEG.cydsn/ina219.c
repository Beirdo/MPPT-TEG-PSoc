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
#include "ina219.h"

#define INA219_COUNT 3

uint8 INA219_address[] = {INA219_INPUT_ADDR, INA219_MID_ADDR, INA219_OUTPUT_ADDR};
uint16 INA219_calibration[] = {4194, 4194, 0x0000};
int connected = 0;

void INA219_initialize(int index) {
    int i;
    INA219_connect(index);
    
    for (i = 0; i < INA219_COUNT; i++) {
        uint8 addr = INA219_address[i];
        uint16 calibration = INA219_calibration[i];
        
        // Send 0x8000 to the config register (0x00) - hard reset
        INA219_write_register(addr, 0x00, 0x8000);

        // Send the calibration value for this chip to calibration register (0x05)
        INA219_write_register(addr, 0x05, calibration);
    }
}

void INA219_connect(int index) {
    uint8 value = 1 << index;
    
    if (connected == value) {
        return;
    }
    
    // Ensure the reset isn't on for some dumbass reason
    I2C_RESET_Write(1);
    
    // First use the TCA9548 to switch the I2C bus over to the channel
    I2C_1_I2CMasterSendStart(TCA9548_ADDR, I2C_1_I2C_WRITE_XFER_MODE, 0);  
    I2C_1_I2CMasterWriteByte(value, 0);  // Write the data value
    I2C_1_I2CMasterSendStop(0);
    
    connected = value;
}

uint16 INA219_read_register(uint8 addr, uint8 reg) {
    uint16 value;
    uint8 temp;
    
    // Write out the register number
    I2C_1_I2CMasterSendStart(addr, I2C_1_I2C_WRITE_XFER_MODE, 0);  
    I2C_1_I2CMasterWriteByte(reg, 0);  // Write the register address
    // Restart reading
    I2C_1_I2CMasterSendRestart(addr, I2C_1_I2C_READ_XFER_MODE, 0);  
    I2C_1_I2CMasterReadByte(I2C_1_I2C_ACK_DATA, &temp, 0);  // Read the MSB of value
    value = temp << 8;
    I2C_1_I2CMasterReadByte(I2C_1_I2C_NAK_DATA, &temp, 0);  // Read the LSB of value
    value |= temp;
    I2C_1_I2CMasterSendStop(0);
    return value;
}

void INA219_write_register(uint8 addr, uint8 reg, uint16 value) {
    I2C_1_I2CMasterSendStart(addr, I2C_1_I2C_WRITE_XFER_MODE, 0);  
    I2C_1_I2CMasterWriteByte(reg, 0);  // Write the register address
    I2C_1_I2CMasterWriteByte((value >> 8) & 0xFF, 0);  // Write the MSB of value
    I2C_1_I2CMasterWriteByte(value & 0xFF, 0);  // Write the LSB of value
    I2C_1_I2CMasterSendStop(0);
}

void INA219_read(uint8 addr, int index, uint16 *current, uint16 *voltage, uint16 *power) {
    if (!current || !voltage || !power) {
        return;
    }

    INA219_connect(index);

    // Now we have the bus connected to the correct INA219 chips.  Read 3 registers
    // (0x02 = bus voltage, 0x03 = power, 0x04 = current)
    *voltage = INA219_read_register(addr, 0x02);
    *power = INA219_read_register(addr, 0x03);
    *current = INA219_read_register(addr, 0x04);
}

void INA219_disconnect(void) {
    I2C_RESET_Write(1);
    I2C_RESET_Write(0);
    I2C_RESET_Write(1);
    connected = 0;
}

/* [] END OF FILE */
