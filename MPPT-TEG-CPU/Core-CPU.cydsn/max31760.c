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
#include "max31760.h"
#include "i2cRegisters.h"
#include "utils.h"
#include "eepromContents.h"

uint8 fan_poles[2];

void MAX31760_initialize(void)
{
    memcpy(&eeprom_contents->fan_poles[0], &fan_poles[0], sizeof(fan_poles));
    // TODO: setup a whack of registers on the chip!
}

uint16 MAX31760_read_temperature(int index)
{
    uint8 regbase = MAX31760_RTH + 2 * index;
    return i2c_register_read_msb16(MAX31760_I2C_ADDR, regbase);
}

uint16 MAX31760_read_fan_speed(int index)
{
    uint32 rpm;
    uint8 regbase = MAX31760_TC1H + 2 * index;
    uint16 value = i2c_register_read_msb16(MAX31760_I2C_ADDR, regbase);
    rpm = 60 * 100000 / value / fan_poles[index];
    return (uint16)(rpm);
}

void MAX31760_set_fan_pwm(uint8 value)
{
    i2c_register_write(MAX31760_I2C_ADDR, MAX31760_PWMR, value);
}

/* [] END OF FILE */
