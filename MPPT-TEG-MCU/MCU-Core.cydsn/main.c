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

teg_channel_t spi_buffer;
system_data_t system_data;
static volatile _Bool dma_complete_flag;

void send_spi_with_dma(uint8 index);

void SPI_DMA_COMPLETE_Interrupt_InterruptCallback(void) {
    dma_complete_flag = 1;
}

void send_spi_with_dma(uint8 index) {
    if (index < TEG_CHANNEL_COUNT) {
        teg_channel_t *ch = &teg_channels[index];
        memcpy(&spi_buffer, ch, sizeof(teg_channel_t));
    } else if (index == 0xFF) {
        memset(&spi_buffer, 0, sizeof(teg_channel_t));
        memcpy(&spi_buffer, &system_data, sizeof(system_data));
    } else {
        return;
    }

    dma_complete_flag = 0;
    DMA_SPI_SetDstAddress(0, (void *)SPI_TX_FIFO_WR_PTR);
    DMA_SPI_SetSrcAddress(0, &spi_buffer);
    DMA_SPI_SetNumDataElements(0, sizeof(spi_buffer));
    DMA_SPI_ValidateDescriptor(0);
    DMA_SPI_ChEnable();
    
    while(!dma_complete_flag) {
        CySysPmSleep();
    }
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
    ADC_SAR_SEQ_Start();
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
        
        // update the die temperature in the system data
        system_data.die_temperature = (int16)(DIE_TEMP_CountsTo_Celsius(ADC_SAR_SEQ_GetResult16(0)) << 3);

        // Now we want to send the entire teg_channel block to the CPU for display and upstream to IOT (one channel per message)
        SPI_SpiSetActiveSlaveSelect(SPI_SPI_SLAVE_SELECT0);
        SPI_Start();

        // Iterate through each channel, then wait for 100ms timer to hit before starting groundhog's day.
        for(i = 0; i < TEG_CHANNEL_COUNT; i++) {
            send_spi_with_dma(i);
        }
        
        // And now system data
        send_spi_with_dma(0xFF);
        
        SPI_Stop();
        
        // And finally, wait for the 100ms timer to tick, and do it all again
        while(lastTimer == CySysWdtGetCount(1)) {
           CySysPmSleep();
        }
        lastTimer = CySysWdtGetCount(1);
    }
}

/* [] END OF FILE */
