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
    
#define TCA9534_ENABLES_INPUT   0x38
#define TCA9534_ENABLES_OUTPUT  0x39
    
uint8 TCA9534_read(uint8 addr);
void TCA9534_write(uint8 addr, uint8 value);
    
#endif // __tca9534_h__

/* [] END OF FILE */
