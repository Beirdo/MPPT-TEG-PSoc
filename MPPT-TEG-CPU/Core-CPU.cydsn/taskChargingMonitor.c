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
#include "chargingMonitor.h"
#include "ina219.h"
#include "tca9534.h"

int powergood_12v;
int on_batt;
    
ina219_reading_t ina_readings[MAX_INA219_COUNT];
chargerBit_t chargerControl[MAX_BATTERIES][MAX_CHARGER_BIT_TYPE] = {
    // BATTERY_LEAD1
    {
        // CHARGER_ENABLED, CHARGER_POWERGOOD, CHARGER_STAT1
        {TCA9534_LEAD_STATUS, 3, 1, 0}, {TCA9534_LEAD_STATUS, 2, 0, 0}, {TCA9534_LEAD_STATUS, 0, 0, 0},
        // CHARGER_STAT2, CHARGER_DESULFATE, CHARGER_CAPACITY_9
        {TCA9534_LEAD_STATUS, 1, 0, 0}, {TCA9534_LEAD_CONTROL, 0, 1, 0}, {TCA9534_LEAD_CONTROL, 1, 1, 0},
        // CHARGER_CAPACITY_20
        {TCA9534_LEAD_CONTROL, 2, 1, 0}
    },
    // BATTERY_LEAD2
    {
        // CHARGER_ENABLED, CHARGER_POWERGOOD, CHARGER_STAT1
        {TCA9534_LEAD_STATUS, 7, 1, 0}, {TCA9534_LEAD_STATUS, 6, 0, 0}, {TCA9534_LEAD_STATUS, 4, 0, 0},
        // CHARGER_STAT2, CHARGER_DESULFATE, CHARGER_CAPACITY_9
        {TCA9534_LEAD_STATUS, 5, 0, 0}, {TCA9534_LEAD_CONTROL, 4, 1, 0}, {TCA9534_LEAD_CONTROL, 5, 1, 0},
        // CHARGER_CAPACITY_20
        {TCA9534_LEAD_CONTROL, 6, 1, 0}

    },
    // BATTERY_LIION
    {
        // CHARGER_ENABLED, CHARGER_POWERGOOD, CHARGER_STAT1
        {TCA9534_LIION_CONTROL, 0, 1, 0}, {TCA9534_LIION_CONTROL, 1, 0, 0}, {TCA9534_LIION_CONTROL, 2, 0, 0},
        // CHARGER_STAT2, CHARGER_DESULFATE, CHARGER_CAPACITY_9
        {TCA9534_LIION_CONTROL, 3, 0, 0}, {TCA9534_LIION_CONTROL, 4, 0, 0}, {TCA9534_LIION_CONTROL, 5, 0, 0},
        // CHARGER_CAPACITY_20
        {TCA9534_LIION_CONTROL, 6, 0, 0}
    }
};
chargerStatus_t chargerStatus[MAX_BATTERIES];

void setupChargingMonitor(void)
{
    INA219_initialize();
    TCA9534_initialize(TCA9534_LEAD_STATUS, TCA9534_BASE_ADDR, 0x77, 0x88);
    TCA9534_initialize(TCA9534_LEAD_CONTROL, TCA9534_BASE_ADDR + 1, 0x00, 0x77);
    TCA9534_initialize(TCA9534_LIION_CONTROL, TCA9534_BASE_ADDR + 2, 0x0E, 0x01);
    memset(chargerStatus, 0, sizeof(chargerStatus));
}

void doTaskChargingMonitor(void *args)
{
    (void)args;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i, j;
    chargerBit_t *bit;
    uint8 stat1;
    uint8 stat2;
    uint8 pgood;
    
    xLastWakeTime = xTaskGetTickCount(); 
    
    for(;;) {
        /* This task should execute every 100 milliseconds.  Time is measured
           in ticks.  The pdMS_TO_TICKS macro is used to convert milliseconds
           into ticks.  xLastWakeTime is automatically updated within vTaskDelayUntil()
           so is not explicitly updated by the task. */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        // Read the voltage/current/power from the charging board's monitoring section
        for (i = 0; i < MAX_INA219_COUNT; i++) {
            ina_readings[i] = INA219_read(i);
        }
        
        // Now read/write the external IOs
        for (i = 0; i < MAX_BATTERIES; i++) {
            for (j = 0; j < MAX_CHARGER_BIT_TYPE; j++) {
                bit = &chargerControl[i][j];
                if (bit->writeable) {
                    TCA9534_write_bit(bit->index, bit->bitnum, bit->value);
                } else {
                    bit->value = TCA9534_read_bit(bit->index, bit->bitnum);
                }
            }
        }
        
        // Deal with LiIon status
        stat1 = get_charger_bit(BATTERY_LIION, CHARGER_STAT1);
        stat2 = get_charger_bit(BATTERY_LIION, CHARGER_STAT2);
        pgood = get_charger_bit(BATTERY_LIION, CHARGER_POWERGOOD);
        
        if (stat1 && stat2 && pgood) {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_SHUTDOWN;
        } else if (stat1 && stat2 && !pgood) {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_STANDBY;
        } else if (!stat1 && stat2 && !pgood) {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_CHARGING;
        } else if (stat1 && !stat2 && !pgood) {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_DONE;
        } else if (!stat1 && !stat2 && !pgood) {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_TEST_MODE;
        } else {
            chargerStatus[BATTERY_LIION] = CHARGER_STATUS_UNKNOWN;
        }
        
        // Deal with LeadAcid status
        for (i = BATTERY_LEAD1; i <= BATTERY_LEAD2; i++) {
            stat1 = get_charger_bit(i, CHARGER_STAT1);
            stat2 = get_charger_bit(i, CHARGER_STAT2);
            pgood = get_charger_bit(i, CHARGER_POWERGOOD);
            
            if (!pgood) {
                chargerStatus[i] = CHARGER_STATUS_SHUTDOWN;
            } else if (stat1 && stat2) {
                chargerStatus[i] = CHARGER_STATUS_FLOAT_MODE;
            } else if (!stat1 && stat2) {
                chargerStatus[i] = CHARGER_STATUS_BULK_CHARGE;
            } else if (!stat1 && !stat2) {
                chargerStatus[i] = CHARGER_STATUS_BOOST_MODE;
            } else {
                chargerStatus[i] = CHARGER_STATUS_UNKNOWN;
            }
        }
    }
}

uint8 get_charger_bit(uint8 index, uint8 bitType) {
    if (index >= MAX_BATTERIES || bitType >= MAX_CHARGER_BIT_TYPE) {
        return 0;
    }

    chargerBit_t *bit = &chargerControl[index][bitType];
    return bit->value;
}

void set_charger_bit(uint8 index, uint8 bitType, uint8 value) {
    if (index >= MAX_BATTERIES || bitType >= MAX_CHARGER_BIT_TYPE) {
        return;
    }

    chargerBit_t *bit = &chargerControl[index][bitType];
    bit->value = value;
}


/* [] END OF FILE */
