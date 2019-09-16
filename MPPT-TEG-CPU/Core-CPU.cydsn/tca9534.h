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

#ifndef __tca9534_h__
#define __tca9534_h__
        
#include "project.h"
    
#define TCA9534_BASE_ADDR 0x38
    
typedef enum {
    TCA9534_LEAD_STATUS,
    TCA9534_LEAD_CONTROL,
    TCA9534_LIION_CONTROL,
    MAX_TCA9534_COUNT,
} tca9534Type_t;

typedef struct _tca9534Control {
    uint8 addr;
    uint8 readMask;
    uint8 writeMask;
    uint8 value;
} tca9534Control_t;

extern tca9534Control_t tca9534Control[MAX_TCA9534_COUNT];
    
void TCA9534_initialize(uint8 index, uint8 addr, uint8 readMask, uint8 writeMask);
uint8 TCA9534_read_inputs(uint8 index);
uint8 TCA9534_read_bit(uint8 index, uint8 bitnum);
void TCA9534_write_bit(uint8 index, uint8 bitnum, uint8 value);
void TCA9534_write_outputs(uint8 index, uint8 value);
void TCA9534_set_bit(uint8 index, uint8 bitnum);
void TCA9534_clear_bit(uint8 index, uint8 bitnum);
    
#endif // __tca9534_h__

/* [] END OF FILE */
