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

#ifndef __tmp100_h__
#define __tmp100_h__

#include "project.h"
    
#define MAX_TMP100_COUNT 8
#define TMP100_BASE_ADDR 0x48
    
#define TMP100_TEMPERATURE_REG 0  // WORD Reads
#define TMP100_CONFIG_REG 1
#define TMP100_TLOW_REG 2
#define TMP100_THIGH_REG 3
    
void TMP100_initialize(void);
int16 TMP100_get_temperature(int index);
    
#endif // __tmp100_h__

/* [] END OF FILE */
