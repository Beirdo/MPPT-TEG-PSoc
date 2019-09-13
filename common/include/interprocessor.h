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
#ifndef __interprocessor_h__
#define __interprocessor_h__
    
#include "project.h"
    
#define TEG_CHANNEL_COUNT 8
    
typedef struct CY_PACKED_ATTR _ina219_reading {
    uint16 voltage;
    uint16 current;
    uint16 power;
} ina219_reading_t;

typedef struct CY_PACKED_ATTR _teg_channel {
    // Per-channel resources
    uint8 index;
    uint8 enabled;
    
    // State of the MPPT FSM for this channel
    uint8 state;
    
    // From INA219 at I2C 0x40
    ina219_reading_t input;

    // From INA219 at I2C 0x41
    ina219_reading_t middle;
    
    // From INA219 at I2C 0x42
    ina219_reading_t output;
    
    // For calculating I-V graph slope/intersects
    uint16 prevVin;
    uint16 prevIin;
    uint16 prevPin;
    
    // Calculated via linear regression
    uint16 Vopen;
    uint16 Ishort;
    
    // output value (0-256)
    uint8 PWMval;
} teg_channel_t;
    
typedef struct CY_PACKED_ATTR _system_data {
    uint8 index;
    
    // Die temperature in 1/8C LSB format
    int16 die_temperature;
} system_data_t;

#endif // __interprocessor_h__   
/* [] */
