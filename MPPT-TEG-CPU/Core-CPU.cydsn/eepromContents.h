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

#ifndef __eepromContents_h__
#define __eepromContents_h__
    
#include "project.h"
#include "wl_definitions.h"
    
typedef struct CY_PACKED_ATTR _eeprom_contents {
    int16 backlight_m;
    int16 backlight_b;
    uint8 fan_poles[2];
    uint8 wifi_enabled;
    uint8 wifi_encryptionType;
    uint8 wifi_ssid[WL_SSID_MAX_LENGTH+1];  // 33
    uint8 wifi_creds[WL_WPA_KEY_MAX_LENGTH+1];  // 64 for WPA, 14 for WEP
} eeprom_contents_t;

static eeprom_contents_t *eeprom_contents = (eeprom_contents_t *)(CYDEV_EE_BASE);

#define EEOFFSETOF(x) (((eeprom_contents *)(0)->(x)) - (eeprom_contents *)(0))

void UpdateEEPROMValue(uint8 *addr, uint8 *buffer, uint8 len);
    
#endif // __eepromContents_h__

/* [] END OF FILE */
