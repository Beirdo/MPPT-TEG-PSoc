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

#ifndef __i2cRegister_h__
#define __i2cRegister_h__
    
#include "project.h"
    
uint8 i2c_register_read(uint8 addr, uint8 regnum);
void i2c_register_write(uint8 addr, uint8 regnum, uint8 value);
    
#endif // __i2cRegister_h__

/* [] END OF FILE */
