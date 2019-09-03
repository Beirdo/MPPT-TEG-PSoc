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
#include "tegchannel.h"
#include "tca9534.h"

int main(void)
{
    int i;
    uint32 lastTimer;
    uint8 newEnables;
    
    // Disable Timer 0
    // Set it up for 10Hz, cascading to Timer 1
    // Enable Timer 0 and 1
    
    lastTimer = 0;
    
    initializeChannels();
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    for(;;)
    {
        // Read the enable DIP switch via I2C 0x38
        newEnables = TCA9534_read(TCA9534_ENABLES_INPUT);
        
        // Iterate through each channel, then wait for 100ms timer to hit before starting groundhog's day.
        for(i = 0; i < TEG_CHANNEL_COUNT; i++) {
            teg_channel_t *ch = &teg_channels[i];
            if (!processChannel(ch, newEnables)) {
                continue;
            }
    
            // Channel is still enabled, any cleanup?
        }

        // Now we want to send the entire teg_channel block to the CPU for display and upstream to IOT
        
        // And finally, wait for the 100ms timer to tick, and do it all again
        while(lastTimer == CySysTimerGetCount(1)) {
           CySysPmSleep();
        }
        lastTimer = CySysTimerGetCount(1);
    }
}

/* [] END OF FILE */
