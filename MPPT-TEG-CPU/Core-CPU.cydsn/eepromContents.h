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
    
typedef struct _eeprom_contents {
    int16 backlight_m;
    int16 backlight_b;
    uint8 fan_poles[2];
} eeprom_contents_t;

static eeprom_contents_t *eeprom_contents = (eeprom_contents_t *)(CYDEV_EE_BASE);

#define EEOFFSETOF(x) (((eeprom_contents *)(0)->(x)) - (eeprom_contents *)(0))
    
#endif // __eepromContents_h__

/* [] END OF FILE */
