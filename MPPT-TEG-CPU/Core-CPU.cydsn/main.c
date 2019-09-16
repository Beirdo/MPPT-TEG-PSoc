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

#include <FreeRTOS.h>
#include <task.h>
#include "project.h"
#include "systemTasks.h"


void prvSetupHardware(void)
{
    EEPROM_Start();
    setupBacklightAdjust();
    setupThermalMonitor();
    setupMcuSpiHandler();
    setupGUITask();
    setupIotTask();
    setupChargingMonitor();
    
    CyGlobalIntEnable;
}

int main(void)
{
    /* Perform any hardware setup necessary. */
    prvSetupHardware();

    /* --- APPLICATION TASKS CAN BE CREATED HERE --- */
    xTaskCreate(doTaskBacklightAdjust, "backlight-adjust", 100, NULL, 8, NULL);
    xTaskCreate(doTaskThermalMonitor, "thermal-monitor", 100, NULL, 8, NULL);
    xTaskCreate(doMcuSpiHandler, "MCU-SPI", 100, NULL, 8, NULL);
    xTaskCreate(doGUITask, "gui", 100, NULL, 8, NULL);
    xTaskCreate(doWiFiTask, "WiFi", 100, NULL, 8, NULL);
    xTaskCreate(doIotTask, "IOT", 100, NULL, 8, NULL);
    xTaskCreate(doTaskChargingMonitor, "charging-monitor", 100, NULL, 8, NULL);

    
    /* Start the created tasks running. */
    vTaskStartScheduler();

    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for( ;; );
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
