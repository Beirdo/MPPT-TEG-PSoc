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

#ifndef __ina219_h__
#define __ina219_h__
    
#include "project.h"
#include "interprocessor.h"
    
#define TCA9548_ADDR 0x70
#define INA219_INPUT_ADDR 0x40
#define INA219_MID_ADDR 0x41
#define INA219_OUTPUT_ADDR 0x42
    
void INA219_initialize(int index);
void INA219_connect(int index);
uint16 INA219_read_register(uint8 addr, uint8 reg);
void INA219_write_register(uint8 addr, uint8 reg, uint16 value);
ina219_reading_t INA219_read(uint8 addr, int index);
void INA219_disconnect(void);
    
#endif // __ina219_h__
/* [] END OF FILE */
