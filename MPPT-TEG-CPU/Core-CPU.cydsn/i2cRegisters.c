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
#include "i2cRegisters.h"
#include "utils.h"

uint8 i2c_register_read(uint8 addr, uint8 regnum)
{
    uint8 value;
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterSendRestart(addr, 1);
    value = I2C_MasterReadByte(1);
    I2C_MasterSendStop();
    return value;
}

void i2c_register_write(uint8 addr, uint8 regnum, uint8 value)
{
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterWriteByte(value);
    I2C_MasterSendStop();
}

uint16 i2c_register_read_msb16(uint8 addr, uint8 basereg)
{
    uint16 value = TO_BYTE_C(i2c_register_read(addr, basereg));
    value |= TO_BYTE_D(i2c_register_read(addr, basereg + 1));
    return value;
}

uint16 i2c_register_read_lsb16(uint8 addr, uint8 basereg)
{
    uint16 value = TO_BYTE_D(i2c_register_read(addr, basereg));
    value |= TO_BYTE_C(i2c_register_read(addr, basereg + 1));
    return value;
}

uint16 i2c_register_read16be(uint8 addr, uint8 regnum)
{
    uint16 value;
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterSendRestart(addr, 1);
    value = TO_BYTE_D(I2C_MasterReadByte(0));
    value |= TO_BYTE_C(I2C_MasterReadByte(1));
    I2C_MasterSendStop();
    return value;
}

void i2c_register_write16be(uint8 addr, uint8 regnum, uint16 value)
{
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterWriteByte(BYTE_D(value));
    I2C_MasterWriteByte(BYTE_C(value));
    I2C_MasterSendStop();
}


/* [] END OF FILE */
