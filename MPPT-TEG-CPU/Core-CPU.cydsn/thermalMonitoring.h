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

#ifndef __thermalMonitoring_h__
#define __thermalMonitoring_h__
    
#include "project.h"
    
typedef enum {
    TEMP_THERMOCOUPLE_SOURCE,
    TEMP_THERMOCOUPLE_HOT_SIDE,
    TEMP_THERMOCOUPLE_COLD_SIDE,
    TEMP_THERMISTOR_FAN_INTAKE,
    TEMP_THERMISTOR_FAN_OUTFLOW,
    TEMP_DIE_CPU,
    TEMP_DIE_MCU,
    TEMP_FAN_CONTROLLER_LOCAL,
    TEMP_FAN_CONTROLLER_REMOTE,
    TEMPERATURE_COUNT
} temperatureSource_t;

#define FAN_PWM_DELTA 0x05

#define SPI_SENSOR_COUNT (TEMP_THERMISTOR_FAN_OUTFLOW - TEMP_THERMOCOUPLE_SOURCE + 1)
#define FAN_CONTROLLER_COUNT (TEMP_FAN_CONTROLLER_REMOTE - TEMP_FAN_CONTROLLER_LOCAL + 1)

extern int16 temperatures[TEMPERATURE_COUNT];  // 0.125C LSB
extern uint16 fan_speed[2];  // RPM
extern uint32 flow_rate;     // mL/min
extern int emergency_shutdown;
extern int pump_on;
extern int dump_valve_open;

#endif // __thermalMonitoring_h__

/* [] END OF FILE */
