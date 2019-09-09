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
#include "ltr303.h"
#include "i2cRegisters.h"

void ltr303_initialize(void)
{
    ltr303_write_register(ALS_CONTROL, 0x00);  // Drop it into Standby mode so we can reconfig
    ltr303_write_register(ALS_CONTROL, 0x1C);  // Set the gain to 96x (0.01 - 600 Lux), stay in standby
    ltr303_write_register(ALS_MEAS_RATE, 0x03); // integration time of 100ms, measurement time of 500ms
    ltr303_write_register(ALS_INTERRUPT, 0x00); // active low interrupts, but disabled for now
    ltr303_write_register(ALS_THRES_UP_0, 0xFF);  // set thresholds to 0xFFFF, 0x0000
    ltr303_write_register(ALS_THRES_UP_1, 0xFF);
    ltr303_write_register(ALS_THRES_LOW_0, 0x00);
    ltr303_write_register(ALS_THRES_LOW_1, 0x00);
    ltr303_write_register(ALS_INTERRUPT_PERSIST, 0x00); // get interrupt for every out of bounds value (none)
    ltr303_write_register(ALS_CONTROL, 0x1D);  // Set the gain to 96x (0.01 - 600 Lux), go active
}

void ltr303_enable_interrupt(void)
{
    ltr303_write_register(ALS_INTERRUPT, 0x01); // active low interrupts, and enabled
}

uint8 ltr303_read_register(uint8 regnum)
{
    return i2c_register_read(LTR303_I2C_ADDR, regnum);
}

void ltr303_write_register(uint8 regnum, uint8 value)
{
    return i2c_register_write(LTR303_I2C_ADDR, regnum, value);
}

/* [] END OF FILE */
