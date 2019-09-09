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

teg_channel_t transfer_buffer[TEG_CHANNEL_COUNT];
static volatile _Bool dma_complete_flag;

void dma_complete(void) {
    uint32 mask = CyDmaGetInterruptSource();
    dma_complete_flag = 1;
    CyDmaClearInterruptSource(mask);
}

int main(void)
{
    int i;
    uint32 lastTimer;
    uint8 newEnables;
    
    // Disable all WDT Timers
    CySysWdtUnlock();
    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK | CY_SYS_WDT_COUNTER2_MASK);
    
    // Setup WDT0 as a WDT, set WDT1 as 10Hz, cascading to WDT2
    CySysWdtSetMode(0, CY_SYS_WDT_MODE_INT_RESET);
    CySysWdtSetMode(1, CY_SYS_WDT_MODE_NONE);
    CySysWdtSetMode(2, CY_SYS_WDT_MODE_NONE);
    CySysWdtSetClearOnMatch(1, 1);
    CySysWdtSetMatch(1, 3999);  // 40kHz source -> 10Hz cascade into timer 2
    CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK | CY_SYS_WDT_COUNTER2_MASK);
    CySysWdtSetCascade(CY_SYS_WDT_CASCADE_12);
    
    // Enable Timer 0 and 1
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK | CY_SYS_WDT_COUNTER2_MASK);
    CySysWdtLock();
    
    lastTimer = 0;
    
    initializeChannels();
    
    DMA_SPI_Init();
    DMA_SPI_SetInterruptCallback(dma_complete);
    dma_complete_flag = 0;
    CyDmaEnable();
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    for(;;)
    {
        // Feed the watchdog to keep the system from resetting
        CySysWatchdogFeed(CY_SYS_WDT_COUNTER0);
        
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
        memcpy(transfer_buffer, teg_channels, sizeof(transfer_buffer));
        SPI_SpiSetActiveSlaveSelect(SPI_SPI_SLAVE_SELECT0);
        SPI_Start();
        
        dma_complete_flag = 0;
        DMA_SPI_SetDstAddress(0, (void *)SPI_TX_FIFO_WR_PTR);
        DMA_SPI_SetSrcAddress(0, transfer_buffer);
        DMA_SPI_SetNumDataElements(0, sizeof(transfer_buffer));
        DMA_SPI_ValidateDescriptor(0);
        DMA_SPI_ChEnable();
        
        while(!dma_complete_flag) {
            CySysPmSleep();
        }
        
        SPI_Stop();
        
        // And finally, wait for the 100ms timer to tick, and do it all again
        while(lastTimer == CySysWdtGetCount(1)) {
           CySysPmSleep();
        }
        lastTimer = CySysWdtGetCount(1);
    }
}

/* [] END OF FILE */
