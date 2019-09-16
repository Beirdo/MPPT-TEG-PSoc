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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#include "tegchannel.h"
#include "tca9534.h"

teg_channel_t spi_buffer;
system_data_t system_data;
SemaphoreHandle_t dmaCompleted;

void send_spi_with_dma(uint8 index);

void SPI_DMA_COMPLETE_Interrupt_InterruptCallback(void) {
    static BaseType_t preempted = pdFALSE;
    xSemaphoreGiveFromISR(dmaCompleted, &preempted);
    portYIELD_FROM_ISR(preempted);
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

    DMA_SPI_SetDstAddress(0, (void *)SPI_TX_FIFO_WR_PTR);
    DMA_SPI_SetSrcAddress(0, &spi_buffer);
    DMA_SPI_SetNumDataElements(0, sizeof(spi_buffer));
    DMA_SPI_ValidateDescriptor(0);
    DMA_SPI_ChEnable();

    xSemaphoreTake(dmaCompleted, portMAX_DELAY);
}


void setupTegControl(void) {
    initializeChannels();
    
    dmaCompleted = xSemaphoreCreateBinary();
    DMA_SPI_Init();
    ADC_SAR_SEQ_Start();
    CyDmaEnable();
}

void doTaskTegControl(void *args) {
    (void)args;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i;
    uint8 newEnables;
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        // Wake up every 100ms
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
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
    }
}
/* [] END OF FILE */
