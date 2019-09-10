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

#ifndef __max31760_h__
#define __max31760_h__
    
#include "project.h"
    
#define MAX31760_I2C_ADDR 0x50
    
#define MAX31760_CR1    0x00
#define MAX31760_CR2    0x01
#define MAX31760_CR3    0x03
#define MAX31760_FFDC   0x04
#define MAX31760_MASK   0x05
#define MAX31760_RHSH   0x06
#define MAX31760_RHSL   0x07
#define MAX31760_LOTSH  0x08
#define MAX31760_LOTSL  0x09
#define MAX31760_ROTSH  0x0A
#define MAX31760_ROTSL  0x0B
#define MAX31760_LHSH   0x0C
#define MAX31760_LHSL   0x0D
#define MAX31760_TCTH   0x0E
#define MAX31760_TCTL   0x0F
#define MAX31760_USER   0x10
#define MAX31760_USER_COUNT 0x08
#define MAX31760_LUT    0x20
#define MAX31760_LUT_COUNT 0x30
#define MAX31760_PWMR   0x50
#define MAX31760_PWMV   0x51
#define MAX31760_TC1H   0x52
#define MAX31760_TC1L   0x53
#define MAX31760_TC2H   0x54
#define MAX31760_TC2L   0x55
#define MAX31760_RTH    0x56
#define MAX31760_RTL    0x57
#define MAX31760_LTH    0x58
#define MAX31760_LTL    0x59
#define MAX31760_SR     0x5A
#define MAX31760_EEX    0x5B
    
void MAX31760_initialize(void);
uint16 MAX31760_read_temperature(int index);
uint16 MAX31760_read_fan_speed(int index);
void MAX31760_set_fan_pwm(uint8 value);
    
#endif // __max31760_h__

/* [] END OF FILE */
