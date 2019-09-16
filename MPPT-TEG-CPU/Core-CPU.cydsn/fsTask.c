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
#include "FS.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "FSQueue.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

QueueHandle_t fsRequestQueue;
static SemaphoreHandle_t fsResponseMutex;
static int fsResponse;
static int32 runNumber = -1;

void setupFSTask(void) {
    FS_Init();
    FS_FAT_SupportLFN();
}

void doFSTask(void *args) {
    (void)args;
    fsQueueItem_t rxItem;
    FS_FILE *pFile;
    uint8 buf[32];
    int response = 0;

    while(1) {
        if (!xQueueReceive(fsRequestQueue, &rxItem, portMAX_DELAY)) {
            continue;
        }
        
        // We have a file to create.  First make sure we have our directory.
        if (runNumber == -1) {
            pFile = FS_FOpen("jobfile.txt", "r");
            if (!pFile) {
                runNumber = 0;
            } else {
                if (!FS_FRead(buf, 32, 1, pFile)) {
                    runNumber = 0;
                } else {
                    runNumber = atoi((const char *)buf) + 1;
                }
                FS_FClose(pFile);
            }
            
            pFile = FS_FOpen("jobfile.txt", "w");
            if (pFile) {
                snprintf((char *)buf, 32, "%ld", runNumber);
                FS_FWrite(buf, ustrlen(buf), 1, pFile);
                FS_FClose(pFile);
            }
            
            snprintf((char *)buf, 32, "run-%08lX", runNumber);
            FS_MkDir((const char *)buf);
        }
        
        snprintf((char *)buf, 32, "run-%08lX\\%d_%ld.cbor", runNumber, rxItem.index, rxItem.timestamp);
        pFile = FS_FOpen((const char *)buf, "wb");
        if (pFile) {
            response = FS_FWrite((const char *)rxItem.buffer, rxItem.len, 1, pFile);
            FS_FClose(pFile);
        }
        
        if (rxItem.semaphore) {
            xSemaphoreTake(fsResponseMutex, portMAX_DELAY);
            fsResponse = response;
            xSemaphoreGive(rxItem.semaphore);
        } 
    }
}

int sendFSRequest(uint32 now, uint8 index, SemaphoreHandle_t semaphore, uint8 *data, uint8 data_len, TickType_t timeout) {
    fsQueueItem_t item;
    
    item.timestamp = now;
    item.index = index;
    item.semaphore = semaphore;
    item.buffer = data;
    item.len = data_len;
    return xQueueSend(fsRequestQueue, &item, timeout);
}

int getFSResponse(SemaphoreHandle_t semaphore, TickType_t timeout) {
    int response;
    if (xSemaphoreTake(semaphore, timeout) == pdPASS) {
        response = fsResponse;
    } else {
        response = 0;
    }
    xSemaphoreGive(fsResponseMutex);
    return response;
}
    


/* [] END OF FILE */
