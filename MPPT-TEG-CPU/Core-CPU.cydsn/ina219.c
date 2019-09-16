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
#include "ina219.h"
#include "interprocessor.h"
#include "i2cRegisters.h"

// TODO: calculate these
uint16 INA219_calibration[MAX_INA219_COUNT] = {4194, 4194, 4194, 4194};

// Likewise... TODO: calculate these
// +/- 320mV, avg 16 samples (8.51ms sample time), voltage and current
uint16 INA219_configuration[] = {0x3E67, 0x3E67, 0x3E67, 0x3E67};

void INA219_initialize(void) {
    int i;
    
    for (i = 0; i < MAX_INA219_COUNT; i++) {
        uint8 addr = INA219_BASE_ADDR + i;

        // Send 0x8000 to the config register (0x00) - hard reset
        i2c_register_write16be(addr, 0x00, 0x8000);
        
        // Send the configuration value
        i2c_register_write16be(addr, 0x00, INA219_configuration[i]);

        // Send the calibration value for this chip to calibration register (0x05)
        i2c_register_write16be(addr, 0x05, INA219_calibration[i]);
    }
}

ina219_reading_t INA219_read(int index) {
    ina219_reading_t reading;
    uint8 addr = INA219_BASE_ADDR + index;
    
    memset(&reading, 0, sizeof(reading));
    if (index < 0 || index >= MAX_INA219_COUNT) {
        return reading;
    }
    
    // (0x02 = bus voltage, 0x03 = power, 0x04 = current)
    reading.voltage = i2c_register_read16be(addr, 0x02);
    reading.power   = i2c_register_read16be(addr, 0x03);
    reading.current = i2c_register_read16be(addr, 0x04);
    
    return reading;
}

/* [] END OF FILE */
