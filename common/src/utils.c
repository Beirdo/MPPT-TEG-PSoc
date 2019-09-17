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

int16 convert_temperature(uint16 raw_value, int shifts, uint32 mask_off_bits) {
    int16 value = *(int16 *)&raw_value;
    if (shifts < 0) {
        value <<= -shifts;
    } else {
        value >>= shifts;
    }
    return value & ~mask_off_bits;
}


/* [] END OF FILE */
