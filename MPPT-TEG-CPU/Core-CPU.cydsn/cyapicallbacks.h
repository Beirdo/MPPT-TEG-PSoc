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
#ifndef CYAPICALLBACKS_H
#define CYAPICALLBACKS_H
    
#define LIGHT_SENSOR_IRQ_INTERRUPT_INTERRUPT_CALLBACK
void LIGHT_SENSOR_IRQ_Interrupt_InterruptCallback(void);

#define TMP05_EOC_ISR_INTERRUPT_INTERRUPT_CALLBACK
void TMP05_EOC_ISR_Interrupt_InterruptCallback(void);

#define FAN_IRQ_INTERRUPT_INTERRUPT_CALLBACK
void FAN_IRQ_Interrupt_InterruptCallback(void);

#define ESP_BUSY_IRQ_INTERRUPT_INTERRUPT_CALLBACK
void ESP_BUSY_IRQ_Interrupt_InterruptCallback(void);

#define SPIM_WIFI_TX_ISR_ENTRY_CALLBACK
void SPIM_WIFI_TX_ISR_EntryCallback(void);
        
#define SPIM_WIFI_TX_ISR_EXIT_CALLBACK
void SPIM_WIFI_TX_ISR_ExitCallback(void);

#define MCU_SPI_DONE_INTERRUPT_INTERRUPT_CALLBACK
void MCU_SPI_DONE_Interrupt_InterruptCallback(void);
            
#endif /* CYAPICALLBACKS_H */   
/* [] */
