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
#include "wifiQueue.h"
#include "systemTasks.h"
#include "convertTime.h"
#include "mcuData.h"
#include "thermalMonitoring.h"
#include "iotDefines.h"
#include "eepromContents.h"
#include "chargingMonitor.h"
#include "FSQueue.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "utils.h"

#include "spi_drv.h"

#include "qcbor.h"

static const int idiot_check_now = 1568536025; // 9/15/2019 1:27AM PST
SemaphoreHandle_t wifiCommandSemaphore;
SemaphoreHandle_t fsCommandSemaphore;
uint8 cbor_buffer[WIFI_SOCKET_BUFFER_SIZE];
uint32 remote_ip;
uint16 remote_port;
uint8 remote_mode;

static int cbor_encode_channel(uint32 now, int index, UsefulBufC *output);
static int cbor_encode_system_data(uint32 now, UsefulBufC *output);

void setupIotTask(void) {
    wifiCommandSemaphore = xSemaphoreCreateBinary();
    fsCommandSemaphore = xSemaphoreCreateBinary();
}

void doIotTask(void *args) {
    TickType_t xLastWakeTime;
    const TickType_t xPeriodLong = pdMS_TO_TICKS(10000);
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);
    uint32 now;
    int rtc_correct = 0;
    int need_now = 0;
    int wifi_connected = 0;
    int wifi_enabled = 0;
    int remote_configured = 0;
    int sdcard_detected = 0;
    uint8 socket = 0xFF;
    wifiQueueItem_t item;
    int i;
    UsefulBufC Encoded;
    (void)args;
    

    xLastWakeTime = xTaskGetTickCount(); 
    
    while(1) {
        /* This task should execute every 10 seconds until WiFi is up and the RTC is correct, and then every second.
           Time is measured in ticks.  The pdMS_TO_TICKS macro is used to convert milliseconds
           into ticks.  xLastWakeTime is automatically updated within vTaskDelayUntil()
           so is not explicitly updated by the task. */
        if (!wifi_connected || !rtc_correct) {
            vTaskDelayUntil(&xLastWakeTime, xPeriodLong);
        } else {
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
        }
        
        wifi_connected = isWifiConnected();
        wifi_enabled = isWifiEnabled();
        
        if (!wifi_connected) {
            if (wifi_enabled) {
                if (sendWifiRequest(WIFI_CONNECT, wifiCommandSemaphore, 0, 0, pdMS_TO_TICKS(10))) {
                    item = getWifiResponse(wifiCommandSemaphore, xPeriodLong / 2);
                    wifi_connected = (int)item.data;
                }
            }
        }
        
        need_now = 1;
        if (wifi_connected && !rtc_correct) {
            if (sendWifiRequest(WIFI_GET_EPOCH_TIME, wifiCommandSemaphore, 0, 0, pdMS_TO_TICKS(10))) {
                item = getWifiResponse(wifiCommandSemaphore, xPeriod);
                now = (uint32)item.data;
                need_now = 0;
                if (now >= idiot_check_now) {
                    // Yay, we have a time, set the RTC
                    struct tm time;
                    RTC_TIME_DATE rtctime;
                    
                    __secs_to_tm(now, &time);
                    RTC_WriteTime(&rtctime);
                    
                    // Since this STUPID RTC has no capability of using epoch time, we have our own timers for that
                    // Due to space constraints, I had to manually ripple from the low 16bit to the high 16 bit, so
                    // when we write a new value, we need to make sure it doesn't ripple while we do it.  To do that:
                    // clear lower 16bits, write upper 16 bits, write lower 16 bits.
                    PPS_LO_WriteCounter(0x0000);
                    PPS_HI_WriteCounter(WORD_AB(now));
                    PPS_LO_WriteCounter(WORD_CD(now));
                    rtc_correct = 1;
                }
            }
        }
        
        if (!remote_configured) {
            remote_ip = eeprom_contents->wifi_remote_ip;
            remote_port = eeprom_contents->wifi_remote_port;
            remote_mode = eeprom_contents->wifi_remote_mode;
            
            if (remote_ip != 0xFFFFFFFF && remote_port != 0xFFFF && remote_mode != 0xFF) {
                remote_configured = 1;
            }
        }
        
        if (wifi_connected && socket == 0xFF && remote_configured) {
            wifiConnectTcpData_t connectRequest;
            connectRequest.ip = remote_ip; // hostnames take too much space
            connectRequest.port = remote_port;
            connectRequest.mode = remote_mode;
            if (sendWifiRequest(WIFI_CONNECT_TCP, wifiCommandSemaphore, (uint8 *)&connectRequest, sizeof(connectRequest), pdMS_TO_TICKS(10))) {
                item = getWifiResponse(wifiCommandSemaphore, xPeriodLong);
                if (item.request_type != WIFI_TIMEOUT) {
                    socket = (uint8)((uint32)item.data);
                }
            }
        }
        
        if (need_now) {
            // Read our RTC timers.  To make sure it didn't ripple between readings: read HI, read LO, read HI again if LOW <= 1
            uint16 hi = PPS_HI_ReadCounter();
            uint16 low = PPS_LO_ReadCounter();
            if (low <= 1) {
                hi = PPS_HI_ReadCounter();
            }
            now = TO_WORD_AB(hi) | TO_WORD_CD(low);
            need_now = 0;
        }
        
        // Check if an SD card is in.
        sdcard_detected = !(CARD_DETECT_Read());  // Active low signal
        
        // Time to generate our CBOR output.  If the RTC is not correct, we will get seconds since reboot, and we will only
        // log to the SD card.  If the wifi is connected and our socket is connected and we have RTC, we will also send the
        // data out to the IOT collector over wifi.
        for(i = 0; i < TEG_CHANNEL_COUNT; i++) {
            if (cbor_encode_channel(now, i, &Encoded)) {
                // Send on Wifi
                if (socket != 0xFF && rtc_correct) {
                    wifiSendTcpData_t sendData;
                    sendData.sock = socket;
                    sendData.data = (uint8 *)Encoded.ptr;
                    sendData.data_len = Encoded.len; 
                    if (sendWifiRequest(WIFI_SEND_TCP_DATA, wifiCommandSemaphore, (uint8 *)&sendData, sizeof(sendData), pdMS_TO_TICKS(10))) {
                        getWifiResponse(wifiCommandSemaphore, pdMS_TO_TICKS(500));
                    }
                }
                // Write to logfile
                if (sdcard_detected) {
                    sendFSRequest(now, i, fsCommandSemaphore, (uint8 *)Encoded.ptr, Encoded.len, pdMS_TO_TICKS(20));
                    getFSResponse(fsCommandSemaphore, pdMS_TO_TICKS(100));
                }
            }
        }

        if (cbor_encode_system_data(now, &Encoded)) {
            // Send on wifi
            if (socket != 0xFF && rtc_correct) {
                wifiSendTcpData_t sendData;
                sendData.sock = socket;
                sendData.data = (uint8 *)Encoded.ptr;
                sendData.data_len = Encoded.len; 
                if (sendWifiRequest(WIFI_SEND_TCP_DATA, wifiCommandSemaphore, (uint8 *)&sendData, sizeof(sendData), pdMS_TO_TICKS(10))) {
                    getWifiResponse(wifiCommandSemaphore, pdMS_TO_TICKS(500));
                }
            }
            // Write to logfile
            if (sdcard_detected) {
                sendFSRequest(now, i, fsCommandSemaphore, (uint8 *)Encoded.ptr, Encoded.len, pdMS_TO_TICKS(20));
                getFSResponse(fsCommandSemaphore, pdMS_TO_TICKS(100));
            }
        }
    }
}

static int cbor_encode_channel(uint32 now, int index, UsefulBufC *output) {
    QCBOREncodeContext EC;
    if (index >= TEG_CHANNEL_COUNT) {
        return 0;
    }
    teg_channel_t *ch = &teg_channels[index];

    QCBOREncode_Init(&EC, UsefulBuf_FROM_BYTE_ARRAY(cbor_buffer));

    QCBOREncode_OpenMap(&EC);
    QCBOREncode_AddSZStringToMapN(&EC, IOT_BOARD_ID, (const char *)eeprom_contents->board_id);
    QCBOREncode_AddDateEpochToMapN(&EC, IOT_TIMESTAMP, now);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_INDEX, index);
    QCBOREncode_AddBoolToMapN(&EC, IOT_TEG_ENABLED, ch->enabled);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_STATE, ch->state);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_INPUT_VOLTAGE, ch->input.voltage);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_INPUT_CURRENT, ch->input.current);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_INPUT_POWER, ch->input.power);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_MIDDLE_VOLTAGE, ch->middle.voltage);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_MIDDLE_CURRENT, ch->middle.current);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_MIDDLE_POWER, ch->middle.power);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_OUTPUT_VOLTAGE, ch->output.voltage);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_OUTPUT_CURRENT, ch->output.current);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_OUTPUT_POWER, ch->output.power);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_VOPEN, ch->Vopen);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_ISHORT, ch->Ishort);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_TEG_PWM_VALUE, ch->PWMval);
    QCBOREncode_CloseMap(&EC);

    return (QCBOREncode_Finish(&EC, output) == QCBOR_SUCCESS);
}

static int cbor_encode_system_data(uint32 now, UsefulBufC *output) {
    QCBOREncodeContext EC;
    int i, j, k;

    QCBOREncode_Init(&EC, UsefulBuf_FROM_BYTE_ARRAY(cbor_buffer));

    QCBOREncode_OpenMap(&EC);
    QCBOREncode_AddSZStringToMapN(&EC, IOT_BOARD_ID, (const char *)eeprom_contents->board_id);
    QCBOREncode_AddDateEpochToMapN(&EC, IOT_TIMESTAMP, now);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_INDEX, 0xFF);
    for (i = 0; i < TEMPERATURE_COUNT; i++) {
        QCBOREncode_AddInt64ToMapN(&EC, IOT_SYS_TEMPERATURE_BASE + i, temperatures[i]);
    }
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_INPUT_FAN_SPEED, fan_speed[0]);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_OUTPUT_FAN_SPEED, fan_speed[1]);
    QCBOREncode_AddBoolToMapN(&EC, IOT_SYS_PUMP_ON, pump_on);
    QCBOREncode_AddBoolToMapN(&EC, IOT_SYS_DUMP_VALVE_OPEN, dump_valve_open);
    QCBOREncode_AddBoolToMapN(&EC, IOT_SYS_EMERGENCY_SHUTDOWN, emergency_shutdown);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_FLOW_RATE, flow_rate);
    QCBOREncode_AddBoolToMapN(&EC, IOT_SYS_12V_POWERGOOD, PGOOD_12V_Read());
    QCBOREncode_AddBoolToMapN(&EC, IOT_SYS_ON_BATT, ON_BATT_Read());
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_24V_VOLTAGE, ina_readings[INA_RAIL_24V].voltage);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_24V_CURRENT, ina_readings[INA_RAIL_24V].current);
    QCBOREncode_AddUInt64ToMapN(&EC, IOT_SYS_24V_POWER, ina_readings[INA_RAIL_24V].power);
    for (i = 0, j = IOT_SYS_LEAD1_VOLTAGE; i < MAX_BATTERIES; i++) {
        uint8 capacity = 0;
        QCBOREncode_AddUInt64ToMapN(&EC, j++, ina_readings[INA_LEAD_BATTERY1 + i].voltage);
        QCBOREncode_AddUInt64ToMapN(&EC, j++, ina_readings[INA_LEAD_BATTERY1 + i].current);
        QCBOREncode_AddUInt64ToMapN(&EC, j++, ina_readings[INA_LEAD_BATTERY1 + i].power);
        for (k = CHARGER_ENABLED; k < CHARGER_DESULFATE; k++) {
            QCBOREncode_AddBoolToMapN(&EC, j++, get_charger_bit(i, k));
        }
        QCBOREncode_AddUInt64ToMapN(&EC, j++, chargerStatus[i]);
        if (i == BATTERY_LIION) {
            continue;
        }
            
        QCBOREncode_AddBoolToMapN(&EC, j++, get_charger_bit(i, k++));
        if (get_charger_bit(i, k++)) {
            capacity = 9;
        } else if (get_charger_bit(i, k++)) {
            capacity = 20;
        }
        QCBOREncode_AddUInt64ToMapN(&EC, j++, capacity);
    }

    QCBOREncode_CloseMap(&EC);

    return (QCBOREncode_Finish(&EC, output) == QCBOR_SUCCESS);
}

/* [] END OF FILE */
