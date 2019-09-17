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
#include "tca9546.h"
#include "tegchannel.h"
#include "i2cRegisters.h"

void TMP100_initialize(void) {
    int i, j, k;
    
    for (i = 0; i < INTERFACE_BOARD_COUNT; i++) {
        for (j = 0; j < INTERFACE_COUNT_PER_BOARD; j++) {
            TCA9546_select_bus(i, j);
            for (k = 0; k < TMP100_PER_INTERFACE; k++) {
                uint8 addr = TMP100_BASE_ADDR + k;
                // Set to 10bit resolution (1/4C LSB) so we get 80ms conversions.
                // 11bit (1/8C LSB) would be preferrable, but 160ms is too long if we poll every 100ms
                i2c_register_write(addr, TMP100_CONFIG_REG, 0x20);
            }
        }
    }
    TCA9546_reset();
}

int16 TMP100_get_temperature(int interface, int index) {
    if ((interface >= TEG_CHANNEL_COUNT) || (index >= TMP100_PER_INTERFACE)) {
        return 0;
    }
    int board = interface / INTERFACE_COUNT_PER_BOARD;
    interface = interface % INTERFACE_COUNT_PER_BOARD;
    uint8 addr = TMP100_BASE_ADDR + index;
    uint16 value;
    
    TCA9546_select_bus(board, interface);
    value = i2c_register_read16be(addr, TMP100_TEMPERATURE_REG);
    
    // Don't bother resetting as we often will be switching right back anyways.  Wasted time.
    return value;
}


/* [] END OF FILE */
