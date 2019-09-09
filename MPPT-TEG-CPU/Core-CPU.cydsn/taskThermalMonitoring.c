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
#include "semphr.h"

#include "project.h"
#include "systemTasks.h"
#include "utils.h"

#define THERMOCOUPLE_START 0
#define THERMOCOUPLE_COUNT 3
#define THERMISTOR_START (THERMOCOUPLE_START + THERMOCOUPLE_COUNT)
#define THERMISTOR_COUNT 2
#define TMP05_START (THERMISTOR_START + THERMISTOR_COUNT)
#define TMP05_COUNT 4
#define DIE_START (TMP05_START + TMP05_COUNT)
#define DIE_COUNT 1

#define SPI_SENSOR_COUNT (THERMISTOR_START + THERMISTOR_COUNT)
#define TEMPERATURE_COUNT (DIE_START + DIE_COUNT)

int16 temperatures[TEMPERATURE_COUNT];
SemaphoreHandle_t needToReadTMP05;

void TMP05_EOC_ISR_Interrupt_InterruptCallback(void)
{
    // EOC for the TMP05 chain conversions
    static BaseType_t preempted = pdFALSE;
    xSemaphoreGiveFromISR(needToReadTMP05, &preempted);
    portYIELD_FROM_ISR(preempted); 
}

void setupThermalMonitor(void)
{
    needToReadTMP05 = xSemaphoreCreateBinary();
    // TBD
}

void doTaskThermalMonitor(void *args)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i;
    uint16 spi_sensor_value[SPI_SENSOR_COUNT];
    int16 tmp05_value[TMP05_COUNT];
    int16 die_temp;
    
    (void)args;
    
    xLastWakeTime = xTaskGetTickCount(); 
    
    for(;;) {
        /* This task should execute every 100 milliseconds.  Time is measured
           in ticks.  The pdMS_TO_TICKS macro is used to convert milliseconds
           into ticks.  xLastWakeTime is automatically updated within vTaskDelayUntil()
           so is not explicitly updated by the task. */
        vTaskDelayUntil(&xLastWakeTime, xPeriod); 
    
        // Let's do the SPI sensors first
        for(i = 0; i < SPI_SENSOR_COUNT; i++) {
            SENSOR_SELECT_Write(i);
            SPIM_SENSORS_WriteTxData(0x0000);  // all of these sensors are send-only, but we need to TX to RX
            spi_sensor_value[i] = SPIM_SENSORS_ReadRxData();
        }
        
        // Now pull the temperatures from the TMP05 chain if they are ready (already in 1/100C format!)
        if (xSemaphoreTake(needToReadTMP05, 0) == pdPASS) {
            for(i = 0; i < TMP05_COUNT; i++) {
                tmp05_value[i] = TMP05_GetTemperature(i);
            }
        }
        
        // Also let's get the die temperature
        DieTemp_GetTemp(&die_temp);

                
        // Convert all our temperatures into a common format (int16, 1/1000 degree C), 
        // updating the externally visible table
        for(i = THERMOCOUPLE_START; i < THERMOCOUPLE_START + THERMOCOUPLE_COUNT; i++) {
            // Thermocouple range is 0 - 1023.75C, int11.2 (12bits, with a dummy sign bit), left justified
            // to convert to 1/1000C...  (value * 1000) / 4 (going to round it too)
            temperatures[i] = ((((spi_sensor_value[i] >> 3) & 0x0FFF) * 1000) + 2) >> 2;            
        }
        
        for(i = THERMISTOR_START; i < THERMISTOR_START + THERMISTOR_COUNT; i++) {
            // Termistor readings have an LSB of 0.125C and are signed.  int8.3 format (11 bits total), left justified
            // to convert to 1/1000C...  (*(int16 *)&value * 1000) / 8
            temperatures[i] = (((((*(int16 *)&spi_sensor_value[i]) >> 5) & 0x03FF) * 1000) + 4) >> 3;
        }
        
        for(i = 0; i < TMP05_COUNT; i++) {
            // Converting 1/100C to 1/1000C is easy.
            temperatures[i+TMP05_START] = tmp05_value[i] * 10;
        }
        
        // Die temperature is in degrees C
        temperatures[DIE_START] = die_temp * 1000;

        // Using input and output air temperatures, determine the PWM value for the fan control
        // get fan speeds
        // set the fan speed PWM directly in the fan controller
    }
}

/* [] END OF FILE */
