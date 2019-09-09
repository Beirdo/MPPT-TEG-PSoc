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

#include "tca9534.h"

uint8 TCA9534_read(uint8 addr) {
    uint8 data;
    I2C_I2CMasterSendStart(addr, I2C_I2C_WRITE_XFER_MODE, 0);
    I2C_I2CMasterWriteByte(0x00, 0);  // Write the data register address
    I2C_I2CMasterSendRestart(addr, I2C_I2C_READ_XFER_MODE, 0);
    I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA, &data, 0);
    I2C_I2CMasterSendStop(0);
    return data;
}

void TCA9534_write(uint8 addr, uint8 value) {
    I2C_I2CMasterSendStart(addr, I2C_I2C_WRITE_XFER_MODE, 0);  
    I2C_I2CMasterWriteByte(0x00, 0);  // Write the data register address
    I2C_I2CMasterWriteByte(value, 0);  // Write the data value
    I2C_I2CMasterSendStop(0);
}

/* [] END OF FILE */
