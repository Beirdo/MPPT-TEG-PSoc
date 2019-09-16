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

#ifndef __chargingMonitor_h__
#define __chargingMonitor_h__
    
#include "project.h"
#include "interprocessor.h"
#include "ina219.h"

typedef struct CY_PACKED_ATTR _chargerBit {
    uint8 index;
    uint8 bitnum;
    uint8 writeable;
    uint8 value;
} chargerBit_t;

typedef enum {
    CHARGER_ENABLED,
    CHARGER_POWERGOOD,
    CHARGER_STAT1,
    CHARGER_STAT2,
    CHARGER_DESULFATE,   // only valid for Lead-Acid
    CHARGER_CAPACITY_9,  // in AH, only valid for Lead-Acid
    CHARGER_CAPACITY_20, // in AH, only valid for Lead-Acid
    MAX_CHARGER_BIT_TYPE,
} chargerBitType_t;

typedef enum {
    BATTERY_LEAD1,
    BATTERY_LEAD2,
    BATTERY_LIION,
    MAX_BATTERIES,
} batteryType_t;

typedef enum {
    CHARGER_STATUS_SHUTDOWN,
    CHARGER_STATUS_STANDBY,
    CHARGER_STATUS_CHARGING,
    CHARGER_STATUS_DONE,
    CHARGER_STATUS_TEST_MODE,
    CHARGER_STATUS_FLOAT_MODE,  // Lead Acid
    CHARGER_STATUS_BULK_CHARGE, // Lead Acid
    CHARGER_STATUS_BOOST_MODE,  // Lead Acid
    CHARGER_STATUS_UNKNOWN,
} chargerStatus_t;

typedef enum {
    INA_RAIL_24V,
    INA_LEAD_BATTERY1,
    INA_LEAD_BATTERY2,
    INA_LIION_BATTERY,
} inaReadingType_t;

extern int powergood_12v;
extern int on_batt;
    
extern ina219_reading_t ina_readings[MAX_INA219_COUNT];
extern chargerBit_t chargerControl[MAX_BATTERIES][MAX_CHARGER_BIT_TYPE];
extern chargerStatus_t chargerStatus[MAX_BATTERIES];

uint8 get_charger_bit(uint8 index, uint8 bitType);
void set_charger_bit(uint8 index, uint8 bitType, uint8 value);

#endif // __chargingMonitor_h__

/* [] END OF FILE */
