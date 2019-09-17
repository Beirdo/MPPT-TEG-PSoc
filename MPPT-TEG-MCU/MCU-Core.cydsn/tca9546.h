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

#ifndef __tca9546_h__
#define __tca9546_h__

#define TCA9546_BASE_ADDR 0x70
    
void TCA9546_select_bus(int board, int interface);
void TCA9546_reset(void);
    
#endif // __tca9546_h__

/* [] END OF FILE */
