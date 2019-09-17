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

// Silly API changes between builtin I2C on PSOC5 and the SCB-based one on PSOC4
#if CY_PSOC4
#define I2C_MasterSendStart(x, y)   I2C_I2CMasterSendStart(x, y, 0)
#define I2C_MasterWriteByte(x)      I2C_I2CMasterWriteByte(x, 0)
#define I2C_MasterSendRestart(x, y) I2C_I2CMasterSendRestart(x, y, 0)
#define I2C_MasterReadByteY(x, y)   I2C_I2CMasterReadByte(x, &y, 0)
#define I2C_MasterSendStop()        I2C_I2CMasterSendStop(0)
#elif CY_PSOC5
#define I2C_MasterReadByteY(x, y)   y = I2C_MasterReadByte(x)
#endif

uint8 i2c_register_read(uint8 addr, uint8 regnum)
{
    uint8 value;
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterSendRestart(addr, 1);
    I2C_MasterReadByteY(1, value);
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

uint8 i2c_register_read_noreg(uint8 addr)
{
    uint8 value;
    I2C_MasterSendStart(addr, 1);
    I2C_MasterReadByteY(1, value);
    I2C_MasterSendStop();
    return value;
}

void i2c_register_write_noreg(uint8 addr, uint8 value)
{
    I2C_MasterSendStart(addr, 0);
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
    uint8 temp;
    I2C_MasterSendStart(addr, 0);
    I2C_MasterWriteByte(regnum);
    I2C_MasterSendRestart(addr, 1);
    I2C_MasterReadByteY(0, temp);
    value = TO_BYTE_D(temp);
    I2C_MasterReadByteY(1, temp);
    value |= TO_BYTE_C(temp);
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
