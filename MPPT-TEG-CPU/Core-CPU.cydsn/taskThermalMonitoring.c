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
#include "tmp100.h"
#include "mcuData.h"
#include "thermalMonitoring.h"



#define HOT_SIDE_HOT_THRESHOLD (85 << 3)    // We don't want the hot-side over 85C
#define HOT_SIDE_WARM_THRESHOLD (80 << 3)   // Turn on dump valve at 80C
#define HOT_SIDE_COOL_THRESHOLD (75 << 3)   // Turn off dump valve at 75C

int16 temperatures[TEMPERATURE_COUNT];  // 0.125C LSB
uint16 fan_speed[2];  // RPM
uint32 flow_rate;     // mL/min
int emergency_shutdown = 0;
int pump_on = 0;
int dump_valve_open = 0;

SemaphoreHandle_t fanOverrideFull;
SemaphoreHandle_t shutdownPump;

static uint16 spi_sensor_get_raw_value(int index);
static int16 convert_temperature(uint16 raw_value, int shifts, uint32 mask_off_bits);
void water_flow_clear(void);
uint16 water_flow_read(void);

#define FAN_ALERT 0x01
#define FAN_FAIL 0x02
#define FAN_SHUTDOWN 0x04

void FAN_IRQ_Interrupt_InterruptCallback(void)
{
    static BaseType_t preempted = pdFALSE;
    static BaseType_t localPreempted = pdFALSE;
    uint8 status = FAN_STATUS_REG_Read();   // will clear the bits on read
    
    if(status & FAN_ALERT) {
        // Cold-side is over the threshold.
        xSemaphoreGiveFromISR(fanOverrideFull, &localPreempted);
        preempted = (localPreempted || preempted ? pdTRUE : pdFALSE);
    }
    
    if(status & FAN_FAIL || status & FAN_SHUTDOWN) {
        // Fan has failed or shutdown has occurred
        xSemaphoreGiveFromISR(shutdownPump, &localPreempted);
        preempted = (localPreempted || preempted ? pdTRUE : pdFALSE);
    }
    portYIELD_FROM_ISR(preempted);
}

void water_flow_clear(void)
{
    WATER_FLOW_Write(0x03);
}

uint16 water_flow_read(void)
{
    uint16 reading = WATER_FLOW_COUNT_ReadCounter();
    WATER_FLOW_Write(0x03);
    return reading;
}

void setupThermalMonitor(void)
{
    fanOverrideFull = xSemaphoreCreateBinary();
    shutdownPump = xSemaphoreCreateBinary();
    MAX31760_initialize();
    TMP100_initialize();
    PUMP_ENABLE_Write(1);  // Turn on the circulation pump
    DUMP_VALVE_ENABLE_Write(0);  // Turn off dump valve
    water_flow_clear(); // Reset and enable the water flow counter
}

static uint16 spi_sensor_get_raw_value(int index) {
    if (index >= SPI_SENSOR_COUNT) {
        return 0;
    }
    
    SENSOR_SELECT_Write(index);
    SPIM_SENSORS_WriteTxData(0x0000);  // all of these sensors are send-only, but we need to TX to RX
    return SPIM_SENSORS_ReadRxData();
}

void doTaskThermalMonitor(void *args)
{
    TickType_t xLastWakeTime;
    uint16 period;
    uint16 flow_reading = 0;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i;
    int16 die_temp;
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
                
        // Convert all our temperatures into a common format (int13.3, LSB of 0.125C), 
        // updating the externally visible table
        
        // Pull the thermocouple readings from SPI
        for(i = THERMOCOUPLE_START; i < THERMOCOUPLE_START + THERMOCOUPLE_COUNT; i++) {
            // Thermocouple range is 0 - 1023.75C, int11.2 (13bits including a dummy sign bit), left justified
            // to convert to 1/8C...  value * 2
            temperatures[i] = convert_temperature(spi_sensor_get_raw_value(i), 2, 0x0001);
        }
        
        // Pull the thermistor readings from SPI
        for(i = THERMISTOR_START; i < THERMISTOR_START + THERMISTOR_COUNT; i++) {
            // Termistor readings have an LSB of 0.125C and are signed.  int8.3 format (11 bits total), left justified
            // Just need to be right justified, retaining the sign
            temperatures[i] = convert_temperature(spi_sensor_get_raw_value(i), 5, 0x0000);
        }
        
        // Pull the TMP100 readings from I2C
        for(i = 0; i < TMP100_COUNT; i++) {
            // TMP100 readings have an LSB of 0.25C and are signed.  int8.2 (10 bits total), left justified, zero padded on right
            temperatures[TMP100_START + i] = convert_temperature(TMP100_get_temperature(i), 5, 0x0000);
        }
        
        // Pull the die temperature from the PSoC module (using the ADC)
        // Die temperature is in degrees C
        DieTemp_GetTemp(&die_temp);
        temperatures[DIE_START] = convert_temperature(die_temp, -3, 0x0007);

        // MCU already converted to 1/8C for us (after truncating!!)
        temperatures[INDEX_MCU_DIE] = mcu_system_data.die_temperature;
        
        // Pull the fan controller temperatures via I2C
        for(i = 0; i < FAN_CONTROLLER_COUNT; i++) {
            // Same format as the thermistors
            temperatures[FAN_CONTROLLER_START + i] = convert_temperature(MAX31760_read_temperature(i), 5, 0x0000);
        }

        // Pull the fan speeds over I2C as well
        for(i = 0; i < 2; i++) {
            fan_speed[i] = MAX31760_read_fan_speed(i);
        }

        // Time to control the fan speed.
        if(xSemaphoreTake(fanOverrideFull, 0) == pdPASS) {
            // Our cold-side temperature has hit the threshold.  Blast the fan full speed.
            fan_pwm_value = 0xFF;
        } else {
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
        }
        MAX31760_set_fan_pwm(fan_pwm_value);
        
        if(xSemaphoreTake(shutdownPump, 0) == pdPASS) {
            // Fan failure.  Shutdown the pump so the system can cool down, turn on dump valve
            emergency_shutdown = 1;
            PUMP_ENABLE_Write(0);
            pump_on = 0;
            DUMP_VALVE_ENABLE_Write(1);
            dump_valve_open = 1;
        }

        if(!emergency_shutdown) {
            if(temperatures[INDEX_HOT_SIDE] >= HOT_SIDE_HOT_THRESHOLD) {
                PUMP_ENABLE_Write(0);   // Hot side is getting too hot, stop circulation
                pump_on = 1;
            } else if(temperatures[INDEX_HOT_SIDE] >= HOT_SIDE_WARM_THRESHOLD) {
                DUMP_VALVE_ENABLE_Write(1);
                dump_valve_open = 1;
            } else {
                PUMP_ENABLE_Write(1);   // Ensure we turn the circulating pump on
                pump_on = 1;
                if(temperatures[INDEX_HOT_SIDE] <= HOT_SIDE_COOL_THRESHOLD) {
                    DUMP_VALVE_ENABLE_Write(0);
                    dump_valve_open = 0;
                }
            }
        }
        
        // Calculate the flow rate
        period = TICKS_TO_MS(xTaskGetTickCount() - xLastWakeTime);
        if(period) {  // should be about 100ms
            flow_reading = water_flow_read();  // pulse count (max per 100ms should be around 25)
            // (x pulses / y ms)(1000 ms/1 s)(60 s/1 min)(1L / 485 pulses)(1000mL / 1L) = (x * 60,000,000) / (y * 485)
            flow_rate = ((uint32)(flow_reading) * 60000000) / (uint32)(period * 485);   // output is mL/min
        }
    }
}

static int16 convert_temperature(uint16 raw_value, int shifts, uint32 mask_off_bits) {
    int16 value = *(int16 *)&raw_value;
    if (shifts < 0) {
        value <<= -shifts;
    } else {
        value >>= shifts;
    }
    return value & ~mask_off_bits;
}

/* [] END OF FILE */
