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
#include "wifiQueue.h"
#include "eepromContents.h"
#include "utils.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "WiFi.h"
#include "WiFiClient.h"

QueueHandle_t wifiRequestQueue;
static SemaphoreHandle_t wifiResponseMutex;
static wifiRequest_t wifiResponseItem;

static int initialized = 0;
static uint8 wifi_ssid[WL_SSID_MAX_LENGTH+1];  // 33
static uint8 wifi_creds[WL_WPA_KEY_MAX_LENGTH+1];  // 64 for WPA, 14 for WEP
static uint8 wifi_encryptionType;
static uint8 wifi_enabled = 0;
static int connected = 0;
static int scanCount = 0;
static wifiScanItem_t scanTable[WL_NETWORKS_LIST_MAXNUM];

static int checkInitialized(void);

int isWifiConnected(void) {
    return connected;
}

int isWifiEnabled(void) {
    return wifi_enabled;
}

int sendWifiRequest(wifiRequest_t type, SemaphoreHandle_t semaphore, uint8 *data, uint8 data_len, TickType_t timeout) {
    wifiQueueItem_t item;
    
    item.request_type = type;
    item.semaphore = semaphore;
    item.data = data;
    item.data_len = data_len;
    return xQueueSend(wifiRequestQueue, &item, timeout);
}

wifiQueueItem_t getWifiResponse(SemaphoreHandle_t semaphore, TickType_t timeout) {
    wifiQueueItem_t response;
    
    memset(&response, 0, sizeof(response));
    if (xSemaphoreTake(semaphore, timeout) == pdPASS) {
        memcpy(&response, &wifiResponseItem, sizeof(response));
    } else {
        response.request_type = WIFI_TIMEOUT;
    }
           
    xSemaphoreGive(wifiResponseMutex);
    return response;
}
    
void setupWiFiTask(void){
    wifiRequestQueue = xQueueCreate(WIFI_QUEUE_SIZE, sizeof(wifiQueueItem_t));
    wifiResponseMutex = xSemaphoreCreateMutex();
    sendWifiRequest(WIFI_LOAD_CONFIG, 0, 0, 0, portMAX_DELAY);
    sendWifiRequest(WIFI_INITIALIZE, 0, 0, 0, portMAX_DELAY);
}

static int checkInitialized(void) {
    int localInitialized = 1;
    int i;
    
    for (i = 0; i < (int)sizeof(wifi_ssid); i++) {
        if (wifi_ssid[i] == 0xFF) {
            memset(wifi_ssid, 0, sizeof(wifi_ssid));
            localInitialized = 0;
            break;
        }
    }
    
    for (i = 0; i < (int)sizeof(wifi_creds); i++) {
        if (wifi_creds[i] == 0xFF) {
            memset(wifi_creds, 0, sizeof(wifi_creds));
            localInitialized = 0;
            break;
        }
    }
    
    if (wifi_encryptionType == 0xFF) {
        wifi_encryptionType = AUTH_MODE_AUTO;
        localInitialized = 0;
    }
    
    if (wifi_enabled == 0xFF) {
        wifi_enabled = 0;
        localInitialized = 0;
    }
    initialized = localInitialized;
    return initialized;
}

void doWiFiTask(void *args) {
    wifiQueueItem_t rxItem;
    SemaphoreHandle_t itemSemaphore;
    int responseNeeded;
    (void)args;
    
    memset(wifi_ssid, 0, sizeof(wifi_ssid));
    memset(wifi_creds, 0, sizeof(wifi_creds));
    
    while(1) {
        if (!xQueueReceive(wifiRequestQueue, &rxItem, portMAX_DELAY)) {
            continue;
        }
        
        itemSemaphore = rxItem.semaphore;
        responseNeeded = 0;
        
        switch(rxItem.request_type) {
            case WIFI_INITIALIZE:
                WiFi_init();
                break;
                
            case WIFI_GET_FIRMWARE_REV:
                responseNeeded = 1;
                rxItem.data = WiFi_firmwareVersion();
                rxItem.data_len = ustrlen(rxItem.data) + 1;
                break;
                
            case WIFI_SCAN_NETWORKS: 
                {
                   int i;
                    
                    memset(scanTable, 0, sizeof(scanTable));
                    scanCount = WiFi_scanNetworks();
                    for (i = 0; i < scanCount; i++) {
                        wifiScanItem_t *item = &scanTable[i];
                        memcpy(item->ssid, WiFi_SSID_index(i), sizeof(item->ssid));
                        item->encryptionType = WiFi_encryptionType_index(i);
                        item->channelId = WiFi_channel_index(i);
                        item->rssi = WiFi_RSSI_index(i);
                    }
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)scanTable;
                    rxItem.data_len = scanCount * sizeof(wifiScanItem_t);
                }
                break;
                
            case WIFI_SET_ENCRYPTION_TYPE:
                wifi_encryptionType = (uint8)((uint32)rxItem.data);
                if (connected) {
                    sendWifiRequest(WIFI_DISCONNECT, 0, 0, 0, portMAX_DELAY);
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                } else if (checkInitialized()) {
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                }
                break;
                
            case WIFI_SET_SSID:
                memcpy(wifi_ssid, rxItem.data, min(rxItem.data_len, sizeof(wifi_ssid)));
                if (connected) {
                    sendWifiRequest(WIFI_DISCONNECT, 0, 0, 0, portMAX_DELAY);
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                } else if (checkInitialized()) {
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                }
                break;
                
            case WIFI_SET_CREDS:
                memcpy(wifi_creds, rxItem.data, min(rxItem.data_len, sizeof(wifi_creds)));
                if (connected) {
                    sendWifiRequest(WIFI_DISCONNECT, 0, 0, 0, portMAX_DELAY);
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                } else if (checkInitialized()) {
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                }
                break;
                
            case WIFI_SET_ENABLED: 
                {
                    uint32 newEnabled = (uint32)rxItem.data;
                    uint8 oldEnabled = wifi_enabled;
                    wifi_enabled = !(!newEnabled);
                    if (connected && !wifi_enabled) {
                        sendWifiRequest(WIFI_DISCONNECT, 0, 0, 0, portMAX_DELAY);
                    } else if (initialized && wifi_enabled && oldEnabled) {
                        sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                    }
                }
                break;
                
            case WIFI_LOAD_CONFIG:
                memcpy(wifi_ssid, eeprom_contents->wifi_ssid, sizeof(wifi_ssid));
                memcpy(wifi_creds, eeprom_contents->wifi_creds, sizeof(wifi_creds));
                wifi_enabled = eeprom_contents->wifi_enabled;
                wifi_encryptionType = eeprom_contents->wifi_encryptionType;
                if (checkInitialized()) {
                    sendWifiRequest(WIFI_CONNECT, 0, 0, 0, portMAX_DELAY);
                }
                break;

            case WIFI_SAVE_CONFIG:
                UpdateEEPROMValue(eeprom_contents->wifi_ssid, wifi_ssid, sizeof(wifi_ssid));
                UpdateEEPROMValue(eeprom_contents->wifi_creds, wifi_creds, sizeof(wifi_creds));
                UpdateEEPROMValue(&eeprom_contents->wifi_enabled, &wifi_enabled, 1);
                UpdateEEPROMValue(&eeprom_contents->wifi_encryptionType, &wifi_encryptionType, 1);
                break;
                
            case WIFI_CONNECT:
                {
                    int success = 0;
                    if (checkInitialized()) {
                        switch(wifi_encryptionType) {
                            case AUTH_MODE_OPEN_SYSTEM:
                                success = WiFi_begin_open(wifi_ssid);
                                break;
                            case AUTH_MODE_SHARED_KEY:
                                success = WiFi_begin_WEP(wifi_ssid, 0, wifi_creds);
                                break;
                            case AUTH_MODE_AUTO:
                            case AUTH_MODE_WPA:
                            case AUTH_MODE_WPA_PSK:
                            case AUTH_MODE_WPA2:
                            case AUTH_MODE_WPA2_PSK:
                                success = WiFi_begin_passphrase(wifi_ssid, wifi_creds);
                                break;
                            default:
                                break;
                        }
                    }
                    connected = success;
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)success;
                }
                break;
            case WIFI_DISCONNECT:
                {
                    int success = 0;
                    connected = 0;
                    success = WiFi_disconnect();
                    
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)success;
                }
                break;
            case WIFI_GET_RSSI:
                responseNeeded = 1;
                rxItem.data = (uint8 *)WiFi_RSSI();
                break;
            case WIFI_GET_EPOCH_TIME:
                responseNeeded = 1;
                rxItem.data = (uint8 *)WiFi_getTime();
                break;
            case WIFI_CONNECT_TCP:
                {
                    int socket = 0xFF;
                    wifiConnectTcpData_t *request = (wifiConnectTcpData_t *)rxItem.data;
                    if (rxItem.data_len == sizeof(wifiConnectTcpData_t)) {
                        if (request->ip) {
                            if (request->mode == TLS_MODE) {
                                socket = WiFiClient_connectSSL(request->ip, request->port);
                            } else {
                                socket = WiFiClient_connect(request->ip, request->port);
                            }
                        } else if (request->hostname) {
                            if (request->mode == TLS_MODE) {
                                socket = WiFiClient_connectHostname(request->hostname, request->port);
                            } else {
                                socket = WiFiClient_connectSSLHostname(request->hostname, request->port);
                            }
                        }
                    }
                    
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)socket;
                }
                break;
            case WIFI_DISCONNECT_TCP:
                {
                    WiFiClient_stop((uint8)((uint32)rxItem.data));
                    int success = 1;
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)success;
                }
                break;
                
            case WIFI_SEND_TCP_DATA:
                {
                    int success = 0;
                    wifiSendTcpData_t *request = (wifiSendTcpData_t *)rxItem.data;
                    if (rxItem.data_len == sizeof(wifiSendTcpData_t)) {
                        if (WiFiClient_connected(request->sock)) {
                            success = WiFiClient_write(request->sock, request->data, request->data_len);
                        }
                    }
                    
                    responseNeeded = 1;
                    rxItem.data = (uint8 *)success;
                }
                break;
            default:
                break;
        }
        
        if (itemSemaphore) {
            xSemaphoreTake(wifiResponseMutex, portMAX_DELAY);

            if (responseNeeded) {
                memcpy(&wifiResponseItem, &rxItem, sizeof(wifiResponseItem));
            } else {
                memset(&wifiResponseItem, 0, sizeof(wifiResponseItem));
            }
            
            xSemaphoreGive(itemSemaphore);
        }
    }
}

/* [] END OF FILE */
