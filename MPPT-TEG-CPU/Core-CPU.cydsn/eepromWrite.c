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
#include "eepromContents.h"
#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t eepromMutex;

void EEPROM_initialize(void) {
    EEPROM_Start();
    eepromMutex = xSemaphoreCreateMutex();
}

void UpdateEEPROMValue(uint8 *addr, uint8 *buffer, uint8 len) {
    int i;
    uint8 *eepromBase = (uint8 *)eeprom_contents;
    int startOffset = addr - (uint8 *)eeprom_contents;
    
    if (startOffset + len > (int)EEPROM_EEPROM_SIZE) {
        // hell no
        return;
    }

    xSemaphoreTake(eepromMutex, portMAX_DELAY);
    
    EEPROM_UpdateTemperature();

    if (len < 4) {
        for(i = 0; i < len; i++) {
            EEPROM_WriteByte(buffer[i], startOffset + i);
        }
        xSemaphoreGive(eepromMutex);
        return;
    }

    uint8 rowData[SIZEOF_EEPROM_ROW];
    int rowNum = startOffset / SIZEOF_EEPROM_ROW;
    int rowAddr = startOffset % SIZEOF_EEPROM_ROW;

    while(len > 0) {
        int count = min(len, SIZEOF_EEPROM_ROW - rowAddr );
        if (rowAddr != 0) {
            memcpy(rowData, &eepromBase[rowNum * SIZEOF_EEPROM_ROW], rowAddr);
        }
        memcpy(&rowData[rowAddr], buffer, count);
        buffer += count;
        rowAddr += count;
        len -= count;
        
        if (rowAddr < (int)SIZEOF_EEPROM_ROW) {
            memcpy(&rowData[rowAddr], &eepromBase[rowNum * SIZEOF_EEPROM_ROW + rowAddr], SIZEOF_EEPROM_ROW - rowAddr);
        }
        
        EEPROM_Write(rowData, rowNum);
        while(EEPROM_Query() == CYRET_STARTED) {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        
        rowNum++;
        rowAddr = 0;
    }
    xSemaphoreGive(eepromMutex);
}

/* [] END OF FILE */
