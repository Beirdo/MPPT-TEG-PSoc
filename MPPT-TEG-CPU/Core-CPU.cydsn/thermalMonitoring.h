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
    
#define THERMOCOUPLE_START 0
#define THERMOCOUPLE_COUNT 3
#define THERMISTOR_START (THERMOCOUPLE_START + THERMOCOUPLE_COUNT)
#define THERMISTOR_COUNT 2
#define TMP100_START (THERMISTOR_START + THERMISTOR_COUNT)
#define TMP100_COUNT 8
#define DIE_START (TMP100_START + TMP100_COUNT)
#define DIE_COUNT 2
#define FAN_CONTROLLER_START (DIE_START + DIE_COUNT)
#define FAN_CONTROLLER_COUNT 2

#define INDEX_HOT_SIDE (THERMOCOUPLE_START + 1)
#define INDEX_COLD_SIDE (THERMOCOUPLE_START + 2)
#define INDEX_AMBIENT_AIR (THERMISTOR_START)
#define INDEX_MCU_DIE (DIE_START + 1)

#define FAN_PWM_DELTA 0x05

#define SPI_SENSOR_COUNT (THERMISTOR_START + THERMISTOR_COUNT)
#define TEMPERATURE_COUNT (FAN_CONTROLLER_START + FAN_CONTROLLER_COUNT)
    
extern int16 temperatures[TEMPERATURE_COUNT];  // 0.125C LSB
extern uint16 fan_speed[2];  // RPM
extern uint32 flow_rate;     // mL/min
extern int emergency_shutdown;
extern int pump_on;
extern int dump_valve_open;

#endif // __thermalMonitoring_h__

/* [] END OF FILE */
