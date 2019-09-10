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
#include "max31760.h"

#define THERMOCOUPLE_START 0
#define THERMOCOUPLE_COUNT 3
#define THERMISTOR_START (THERMOCOUPLE_START + THERMOCOUPLE_COUNT)
#define THERMISTOR_COUNT 2
#define TMP05_START (THERMISTOR_START + THERMISTOR_COUNT)
#define TMP05_COUNT 4
#define DIE_START (TMP05_START + TMP05_COUNT)
#define DIE_COUNT 1
#define FAN_CONTROLLER_START (DIE_START + DIE_COUNT)
#define FAN_CONTROLLER_COUNT 2

#define INDEX_COLD_SIDE (THERMOCOUPLE_START + 2)
#define INDEX_AMBIENT_AIR (THERMISTOR_START)
#define FAN_PWM_DELTA 0x05

#define SPI_SENSOR_COUNT (THERMISTOR_START + THERMISTOR_COUNT)
#define TEMPERATURE_COUNT (FAN_CONTROLLER_START + FAN_CONTROLLER_COUNT)

int16 temperatures[TEMPERATURE_COUNT];
uint16 fan_speed[2];

SemaphoreHandle_t needToReadTMP05;

static int16 convert_temperature(uint16 raw_value);

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
    MAX31760_initialize();
}

void doTaskThermalMonitor(void *args)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i;
    uint16 spi_sensor_value[SPI_SENSOR_COUNT];
    int16 tmp05_value[TMP05_COUNT];
    int16 die_temp;
    uint16 fan_temp[FAN_CONTROLLER_COUNT];
    int16 prevTempDiff = 0;
    int16 tempDiff = 0;
    int16 deltaTempDiff;
    int8 deltaPwm;
    uint8 fan_pwm_value = 0;
    
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

        // Get fan controller temperatures and fan speeds
        for(i = 0; i < FAN_CONTROLLER_COUNT; i++) {
            fan_temp[i] = MAX31760_read_temperature(i);
        }
        
        for(i = 0; i < 2; i++) {
            fan_speed[i] = MAX31760_read_fan_speed(i);
        }
        
                
        // Convert all our temperatures into a common format (int16, LSB of 0.125C), 
        // updating the externally visible table
        for(i = THERMOCOUPLE_START; i < THERMOCOUPLE_START + THERMOCOUPLE_COUNT; i++) {
            // Thermocouple range is 0 - 1023.75C, int11.2 (12bits, with a dummy sign bit), left justified
            // to convert to 1/8C...  value * 2
            temperatures[i] = ((spi_sensor_value[i] >> 3) & 0x0FFF) << 1;            
        }
        
        for(i = THERMISTOR_START; i < THERMISTOR_START + THERMISTOR_COUNT; i++) {
            // Termistor readings have an LSB of 0.125C and are signed.  int8.3 format (11 bits total), left justified
            // Just need to be right justified, retaining the sign
            temperatures[i] = convert_temperature(spi_sensor_value[i]);
        }
        
        for(i = 0; i < TMP05_COUNT; i++) {
            // Converting 1/100C to 1/8C, retaining sign, rounding to nearest 1/8.
            temperatures[i+TMP05_START] = ((tmp05_value[i] << 3) + 50) / 100;
        }
        
        // Die temperature is in degrees C
        temperatures[DIE_START] = die_temp << 3;

        for(i = 0; i < FAN_CONTROLLER_COUNT; i++) {
            // Same format as the thermistors
            temperatures[FAN_CONTROLLER_START + i] = convert_temperature(fan_temp[i]);
        }
        
        // Using cold-side temperature (heat sink) and ambient air temperature, create a control loop controlling the
        // fan RPM via PWM.
        prevTempDiff = tempDiff;
        tempDiff = temperatures[INDEX_COLD_SIDE] - temperatures[INDEX_AMBIENT_AIR];
        deltaTempDiff = tempDiff - prevTempDiff;
        
        if(abs(deltaTempDiff) < 4) { // within 0.5C, slow the fan
            deltaPwm = - FAN_PWM_DELTA;
        } else if(deltaTempDiff > 5 << 3) { // differential rose 5C or more, speed up quickly
            deltaPwm = 4 * FAN_PWM_DELTA;
        } else if(deltaTempDiff > 0) { // differential rose some, speed up medium
            deltaPwm = 2 * FAN_PWM_DELTA;
        } else { // differential decreased.  We are starting to catch up to the set-point
            deltaPwm = FAN_PWM_DELTA;
        }
        fan_pwm_value = clamp(fan_pwm_value + deltaPwm, 0, 255);
        MAX31760_set_fan_pwm(fan_pwm_value);
    }
}

static int16 convert_temperature(uint16 raw_value) {
    int16 value = *(int16 *)&raw_value;
    return value >> 5;
}

/* [] END OF FILE */
