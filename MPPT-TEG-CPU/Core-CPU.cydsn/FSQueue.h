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

#ifndef __FSQueue_h__
#define __FSQueue_h__
    
#include "project.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
    
typedef struct CY_PACKED_ATTR _fsQueueItem {
    uint32 timestamp;
    uint8 index;
    uint8 *buffer;
    uint8 len;
    SemaphoreHandle_t semaphore;
} fsQueueItem_t;

#define FS_QUEUE_SIZE 16

extern QueueHandle_t fsRequestQueue;

int sendFSRequest(uint32 now, uint8 index, SemaphoreHandle_t semaphore, uint8 *data, uint8 data_len, TickType_t timeout);
int getFSResponse(SemaphoreHandle_t semaphore, TickType_t timeout);
    
#endif // __FSQueue_h__

/* [] END OF FILE */
