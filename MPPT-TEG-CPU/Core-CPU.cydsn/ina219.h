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
    
#define INA219_BASE_ADDR 0x40
#define MAX_INA219_COUNT 4
    
void INA219_initialize(void);
ina219_reading_t INA219_read(int index);
    
#endif // __ina219_h__
/* [] END OF FILE */
