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
#include "systemTasks.h"
#include "FreeRTOS.h"
#include "task.h"

void prvSetupHardware(void) {
    setupTegControl();
    CyGlobalIntEnable; /* Enable global interrupts. */
}

int main(void) {
    /* Perform any hardware setup necessary */
    prvSetupHardware();
    
    xTaskCreate(doTaskTegControl, "TEG-control", 100, NULL, 8, NULL);
    
    vTaskStartScheduler();
    
    while(1) {
    }
    return 0;
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    (void)pxTask;
    (void)pcTaskName;
    
	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationMallocFailedHook( void )
{
	/* The heap space has been execeeded. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}



/* [] END OF FILE */
