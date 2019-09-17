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
    
typedef enum {
    INA219_INPUT,
    INA219_MID,
    INA219_OUTPUT,
    INA219_PER_INTERFACE,
} ina219Type_t;
    
#define INA219_BASE_ADDR 0x40
    
void INA219_initialize(int interface);
ina219_reading_t INA219_read(int interface, int index);
    
#endif // __ina219_h__
/* [] END OF FILE */
