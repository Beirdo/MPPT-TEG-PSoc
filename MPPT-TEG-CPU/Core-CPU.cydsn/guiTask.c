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
#include "semphr.h"
#include "task.h"
#include "GUI.h"

// INIT_SEQUENCE from https://github.com/adafruit/Adafruit_CircuitPython_HX8357
const uint8 lcd_init_sequence[] = 
    "\x01\x80\x64"             // _SWRESET and Delay 100ms
    "\xB9\x83\xFF\x83\x57\xFF" // _SETC and delay 500ms
    "\xB3\x04\x80\x00\x06\x06" // _SETRGB 0x80 enables SDO pin (0x00 disables)
    "\xB6\x01\x25"             // _SETCOM -1.52V
    "\xB0\x01\x68"             // _SETOSC Normal mode 70Hz, Idle mode 55 Hz
    "\xCC\x01\x05"             // _SETPANEL BGR, Gate direction swapped
    "\xB1\x06\x00\x15\x1C\x1C\x83\xAA"     // _SETPWR1 Not deep standby BT VSPR VSNR AP
    "\xC0\x06\x50\x50\x01\x3C\x1E\x08"     // _SETSTBA OPON normal OPON idle STBA GEN
    "\xB4\x07\x02\x40\x00\x2A\x2A\x0D\x78" // _SETCYC NW 0x02 RTN DIV DUM DUM GDON GDOFF
    "\xE0\x22\x02\x0A\x11\x1d\x23\x35\x41\x4b\x4b\x42\x3A\x27\x1B\x08\x09\x03\x02\x0A"
    "\x11\x1d\x23\x35\x41\x4b\x4b\x42\x3A\x27\x1B\x08\x09\x03\x00\x01" // _SETGAMMA
    "\x3A\x01\x55"             // _COLMOD 16 bit
    "\x36\x01\xC0"             // _MADCTL
    "\x35\x01\x00"             // _TEON TW off
    "\x44\x02\x00\x02"         // _TEARLINE
    "\x11\x80\x96"             // _SLPOUT and delay 150 ms
    "\x36\x01\xA0"
    "\x29\x80\x32";            // _DISPON and delay 50 ms
 
void lcd_initialize_by_sequence(uint8* sequence, int len, int data_as_commands);

// Code "borrowed" from:  CircuitPython, ported to C (not C++!), adapted to FreeRTOS
// https://github.com/adafruit/circuitpython/blob/master/shared-module/displayio/Display.c

#define DELAY 0x80
SemaphoreHandle_t initializeTransaction;

void setupGUITask(void) {
    initializeTransaction = xSemaphoreCreateBinary();
    xSemaphoreGive(initializeTransaction);
    
    /* Start the parallel interface */
    LCDScreen_Start();

    /* Reset - High, low (reset), High */
    LCD_RESET_Write(1);
    vTaskDelay(pdMS_TO_TICKS(20));
    LCD_RESET_Write(0);
    vTaskDelay(pdMS_TO_TICKS(100));
    LCD_RESET_Write(1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    lcd_initialize_by_sequence((uint8 *)lcd_init_sequence, sizeof(lcd_init_sequence), 0);
    
    // Start up the touchscreen
    ResistiveTouch_Start();
}

void lcd_initialize_by_sequence(uint8* sequence, int len, int data_as_commands) {
    int i = 0;
    while (i < len) {
        uint8 *cmd = sequence + i;
        uint8 data_size = *(cmd + 1);
        int delay = (data_size & DELAY) != 0;
        data_size &= ~DELAY;
        uint8 *data = cmd + 2;
        
        xSemaphoreTake(initializeTransaction, portMAX_DELAY);

        if (data_as_commands) {
            uint8_t full_command[data_size + 1];
            full_command[0] = cmd[0];
            memcpy(full_command + 1, data, data_size);
            LCDScreen_WriteM8_A0(full_command, data_size + 1);
        } else {
            LCDScreen_Write8_A0(*cmd);
            LCDScreen_WriteM8_A1(data, data_size);
        }
       
        xSemaphoreGive(initializeTransaction);
        
        int delay_length_ms = 10;
        if (delay) {
            data_size++;
            delay_length_ms = *(cmd + 1 + data_size);
            if (delay_length_ms == 255) {
                delay_length_ms = 500;
            }
        }
      
        vTaskDelay(pdMS_TO_TICKS(delay_length_ms));
        i += 2 + data_size;
    }
}

void doGUITask(void *args)
{
    (void)args;
    GUI_Init();
    
    while(1) {
        // until I have anything to run here.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Let's attach the touch screen to the window manager too!
void GUI_TOUCH_X_ActivateX(void) {
    ResistiveTouch_ActivateX();
}

void GUI_TOUCH_X_ActivateY(void) {
    ResistiveTouch_ActivateY();
}

void GUI_TOUCH_X_Disable  (void) {
    ResistiveTouch_Stop();
}

int  GUI_TOUCH_X_MeasureX (void) {
    return ResistiveTouch_Measure();
}

int  GUI_TOUCH_X_MeasureY (void) {
    return ResistiveTouch_Measure();
}

/* [] END OF FILE */
