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

#ifndef __systemTasks_h__
#define __systemTasks_h__

#include "FreeRTOS.h"
#include "task.h"
    
    
void setupBacklightAdjust(void);
void doTaskBacklightAdjust(void *args);

void setupThermalMonitor(void);
void doTaskThermalMonitor(void *args);
    
#endif // __systemTasks_h__

/* [] END OF FILE */
