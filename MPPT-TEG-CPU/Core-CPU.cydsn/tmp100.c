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
#include "tmp100.h"
#include "i2cRegisters.h"

void TMP100_initialize(void) {
    for(int i = 0; i < MAX_TMP100_COUNT; i++) {
        uint8 addr = TMP100_BASE_ADDR + i;
        // Set to 10bit resolution (1/4C LSB) so we get 80ms conversions.
        // 11bit (1/8C LSB) would be preferrable, but 160ms is too long if we poll every 100ms
        i2c_register_write(addr, TMP100_CONFIG_REG, 0x20);
    }
}

int16 TMP100_get_temperature(int index) {
    if (index >= MAX_TMP100_COUNT) {
        return 0;
    }
    uint8 addr = TMP100_BASE_ADDR + index;
    return i2c_register_read16be(addr, TMP100_TEMPERATURE_REG);
}


/* [] END OF FILE */
