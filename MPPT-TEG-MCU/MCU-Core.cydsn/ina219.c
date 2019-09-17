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
#include "tca9546.h"
#include "i2cRegisters.h"
#include "interprocessor.h"

uint16 INA219_calibration[INA219_PER_INTERFACE] = {4194, 4194, 4194};
// +/- 320mV, avg 16 samples (8.51ms sample time), voltage and current
uint16 INA219_configuration[INA219_PER_INTERFACE] = {0x3E67, 0x3E67, 0x3E67};

void INA219_initialize(int interface) {
    int i;
    
    if (interface >= TEG_CHANNEL_COUNT) {
        return;
    }
    
    int board = interface / INTERFACE_COUNT_PER_BOARD;
    interface = interface % INTERFACE_COUNT_PER_BOARD;
    
    TCA9546_select_bus(board, interface);
    
    for (i = 0; i < INA219_PER_INTERFACE; i++) {
        uint8 addr = INA219_BASE_ADDR + i;
        
        // Send 0x8000 to the config register (0x00) - hard reset
        i2c_register_write16be(addr, 0x00, 0x8000);
        
        // Send the configuration value
        i2c_register_write16be(addr, 0x00, INA219_configuration[i]);

        // Send the calibration value for this chip to calibration register (0x05)
        i2c_register_write16be(addr, 0x05, INA219_calibration[i]);
    }
}

ina219_reading_t INA219_read(int interface, int index) {
    ina219_reading_t reading;
    
    if (interface >= TEG_CHANNEL_COUNT || index >= INA219_PER_INTERFACE) {
        memset(&reading, 0, sizeof(reading));
        return reading;
    }
    
    int board = interface / INTERFACE_COUNT_PER_BOARD;
    interface = interface % INTERFACE_COUNT_PER_BOARD;
    
    TCA9546_select_bus(board, interface);
    uint8 addr = INA219_BASE_ADDR + index;

    // Now we have the bus connected to the correct INA219 chips.  Read 3 registers
    // (0x02 = bus voltage, 0x03 = power, 0x04 = current)
    reading.voltage = i2c_register_read16be(addr, 0x02);
    reading.power   = i2c_register_read16be(addr, 0x03);
    reading.current = i2c_register_read16be(addr, 0x04);
    
    return reading;
}

/* [] END OF FILE */
