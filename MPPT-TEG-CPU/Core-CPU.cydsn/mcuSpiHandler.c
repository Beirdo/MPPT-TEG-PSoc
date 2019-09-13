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
#include "interprocessor.h"
#include "systemTasks.h"
#include "mcuData.h"

#include "FreeRTOS.h"
#include "semphr.h"

teg_channel_t teg_channels[TEG_CHANNEL_COUNT];
system_data_t mcu_system_data;
teg_channel_t spi_buffer;
uint8 dma_channel;
uint8 dma_td;
SemaphoreHandle_t mcuBufferFilled;

void setupMcuSpiHandler(void) {
    memset(teg_channels, 0, sizeof(teg_channels));
    
    mcuBufferFilled = xSemaphoreCreateBinary();
    
    // Setup DMA channel
    dma_channel = SPIS_DMA_DmaInitialize(1, 1, HI16((uint32)SPIS_MCU_RXDATA_PTR), HI16((uint32)&spi_buffer));
    dma_td = CyDmaTdAllocate();
    CyDmaChDisable(dma_channel);
 
    SPIS_MCU_Enable();
    SPIS_MCU_EnableInt();
}

void MCU_SPI_DONE_Interrupt_InterruptCallback(void) {
    static BaseType_t preempted = pdFALSE;
    CyDmaChDisable(dma_channel);
    xSemaphoreGiveFromISR(mcuBufferFilled, &preempted);
    portYIELD_FROM_ISR(preempted); 
}

void doMcuSpiHandler(void *args) {
    int i;
    teg_channel_t *ch;
    
    while(1) {
        // Setup the DMA channel and re-enable it
        CyDmaTdSetConfiguration(dma_td, sizeof(spi_buffer), CY_DMA_DISABLE_TD, SPIS_DMA__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
        CyDmaTdSetAddress(dma_td, LO16((uint32)SPIS_MCU_RXDATA_PTR), LO16((uint32)&spi_buffer)); 
        CyDmaChSetInitialTd(dma_channel, dma_td); 
        CyDmaChEnable(dma_channel, 1);
        
        xSemaphoreTake(mcuBufferFilled, portMAX_DELAY);
        
        // We now have a buffer to move
        i = spi_buffer.index;
        if (i == 0xFF) {
            // System data
            memcpy(&mcu_system_data, &spi_buffer, sizeof(system_data_t));
            continue;
        }
        
        if (i >= TEG_CHANNEL_COUNT) {
            continue;
        }
        
        ch = &teg_channels[i];
        memcpy(ch, &spi_buffer, sizeof(teg_channel_t));
    }
}


/* [] END OF FILE */
