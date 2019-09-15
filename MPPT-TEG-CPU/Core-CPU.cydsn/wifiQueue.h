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

#ifndef __wifiQueue_h__
#define __wifiQueue_h__
    
#include "project.h"
#include "server_drv.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
    
#define WIFI_QUEUE_SIZE 16
    
typedef enum _wifiRequest {
    WIFI_INITIALIZE,
    WIFI_GET_FIRMWARE_REV,
    WIFI_SCAN_NETWORKS,
    WIFI_SET_ENCRYPTION_TYPE,
    WIFI_SET_SSID,
    WIFI_SET_CREDS,
    WIFI_SET_ENABLED,
    WIFI_LOAD_CONFIG,
    WIFI_SAVE_CONFIG,
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    WIFI_GET_RSSI,
    WIFI_GET_EPOCH_TIME,
    WIFI_CONNECT_TCP,
    WIFI_DISCONNECT_TCP,
    WIFI_SEND_TCP_DATA,
    WIFI_TIMEOUT,
} wifiRequest_t;

typedef struct CY_PACKED_ATTR _wifiConnectTcpData {
    uint32 ip;
    uint8 *hostname;
    uint16 port;
    tProtMode mode;
} wifiConnectTcpData_t;

typedef struct CY_PACKED_ATTR _wifiSendTcpData {
    uint8 sock;
    uint8 data_len;
    uint8 *data;
} wifiSendTcpData_t;

typedef struct CY_PACKED_ATTR _wifiScanItem {
    uint8 ssid[WL_SSID_MAX_LENGTH+1];
    uint8 encryptionType;
    uint8 channelId;
    int32 rssi;
} wifiScanItem_t;

typedef struct CY_PACKED_ATTR _wifiQueueItem {
    SemaphoreHandle_t semaphore;
    uint8 *data;
    uint8 data_len;
    uint8 request_type;
} wifiQueueItem_t;
    
extern QueueHandle_t wifiRequestQueue;

int sendWifiRequest(wifiRequest_t type, SemaphoreHandle_t semaphore, uint8 *data, uint8 data_len, TickType_t timeout);
wifiQueueItem_t getWifiResponse(SemaphoreHandle_t semaphore, TickType_t timeout);
int isWifiConnected(void);
int isWifiEnabled(void);
    
#endif // __wifiQueue_h__

/* [] END OF FILE */
