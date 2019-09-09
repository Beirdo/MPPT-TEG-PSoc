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

#include "FreeRTOS.h"
#include "task.h"
#include "project.h"
#include "semphr.h"

#include "systemTasks.h"
#include "eepromContents.h"
#include "ltr303.h"
#include "utils.h"

SemaphoreHandle_t needToReadALS;

void LIGHT_SENSOR_IRQ_Interrupt_InterruptCallback(void)
{
    static BaseType_t preempted = pdFALSE;
    xSemaphoreGiveFromISR(needToReadALS, &preempted);
    portYIELD_FROM_ISR(preempted); 
}

void setupBacklightAdjust(void)
{
    needToReadALS = xSemaphoreCreateBinary();
    ltr303_initialize();
}

void doTaskBacklightAdjust(void *args)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(500); 
    int16 backlight_m = 0;
    int16 backlight_b = 0;
    int16 old_m;
    int16 old_b;
    uint16 als_reading[2] = {0, 0};
    uint16 temp;
    int32 accumulator;
    int needCalc = 0;
    uint8 pwmReading;
    (void)args;
    
    // Enable the ALS interrupts (must happen after scheduler starts, or the semaphore will go wonky)
    ltr303_enable_interrupt();
    
    xLastWakeTime = xTaskGetTickCount(); 
    
    // This task needs to read the ambient light sensor (ALS) and set the backlight PWM to the value 
    // dictated by the light level  via linear regression (y = PWM value, x = sensor value... y = mx + b
    // The values for m and b will be stored in the EEPROM, and can be tweaked via the UI eventually
    for(;;) {
        /* This task should execute every 500 milliseconds.  Time is measured
           in ticks.  The pdMS_TO_TICKS macro is used to convert milliseconds
           into ticks.  xLastWakeTime is automatically updated within vTaskDelayUntil()
           so is not explicitly updated by the task. */
        vTaskDelayUntil(&xLastWakeTime, xPeriod); 
        
        needCalc = 0;
        old_m = backlight_m;
        old_b = backlight_b;
        backlight_m = eeprom_contents->backlight_m;
        backlight_b = eeprom_contents->backlight_b;
        if (old_m != backlight_m || old_b != backlight_b) {
            needCalc++;
        }

        if (xSemaphoreTake(needToReadALS, 0) == pdPASS) {
            // need to read the ALS.
            temp = TO_BYTE_D(ltr303_read_register(ALS_DATA_CH1_0));
            temp |= TO_BYTE_D(ltr303_read_register(ALS_DATA_CH1_1));
            als_reading[1] = temp;
            
            temp = TO_BYTE_D(ltr303_read_register(ALS_DATA_CH0_0));
            temp |= TO_BYTE_D(ltr303_read_register(ALS_DATA_CH0_1));
            als_reading[0] = temp;
            needCalc++;
        }
        
        if (!needCalc) {
            continue;
        }
        
        // We want PWM value from 0-255
        // ALS values are 0-65535 (6100 at 200 Lux on Ch 1)
        // m is scaled to be int3.13 (ie. 16bit total with 13 fractional bits - m * 8192)
        // Assuming we want full backlight at low light and no backlight at high light,
        // m will be -255/6100 => 0xFEAA, and b will be 255 (i.e. max value at 0 light)
        // We only care about sensor 1, and will ignore sensor 0 data
        accumulator = backlight_m * als_reading[1];
        accumulator >>= 13;
        accumulator += backlight_b;
        pwmReading = (uint8)clamp(accumulator, 0, 255);
        
        BACKLIGHT_PWMGEN_WriteCompare(pwmReading);
    }
}

/* [] END OF FILE */
