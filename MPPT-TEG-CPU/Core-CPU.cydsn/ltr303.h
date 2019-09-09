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

#ifndef __ltr303_h__
#define __ltr303_h__
    
#include "project.h"
    
#define LTR303_I2C_ADDR 0x29
    
#define ALS_CONTROL 0x80
#define ALS_MEAS_RATE 0x85
#define ALS_PART_ID 0x86
#define ALS_MANUFAC_ID 0x87
#define ALS_DATA_CH1_0 0x88
#define ALS_DATA_CH1_1 0x89
#define ALS_DATA_CH0_0 0x8A
#define ALS_DATA_CH0_1 0x8B
#define ALS_STATUS 0x8C
#define ALS_INTERRUPT 0x8F
#define ALS_THRES_UP_0 0x97 
#define ALS_THRES_UP_1 0x98 
#define ALS_THRES_LOW_0 0x99 
#define ALS_THRES_LOW_1 0x9A
#define ALS_INTERRUPT_PERSIST 0x9E

void ltr303_initialize(void);
void ltr303_enable_interrupt(void);
uint8 ltr303_read_register(uint8 regnum);
void ltr303_write_register(uint8 regnum, uint8 value);
    
#endif // __ltr303_h__

/* [] END OF FILE */
