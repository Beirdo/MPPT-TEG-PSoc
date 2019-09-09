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

#ifndef __teg_channel_h__
#define __teg_channel_h__
    
#include "project.h"
    
#define TEG_CHANNEL_COUNT 8
    
typedef enum _mppt_state {
    STATE_IDLE = 0,
    STATE_HUNT,
    STATE_FIRST_POINT,
    STATE_SECOND_POINT,
} mppt_state_t;
    
typedef struct _teg_channel {
    // Per-channel resources
    uint8 index;
    uint8 enabled;
    
    // State of the MPPT FSM for this channel
    mppt_state_t state;
    
    // From INA219 at I2C 0x40
    uint16 Vin;
    uint16 Iin;
    uint16 Pin;
    
    // For calculating I-V graph slope/intersects
    uint16 prevVin;
    uint16 prevIin;
    uint16 prevPin;
    
    // Calculated via linear regression
    uint16 Vopen;
    uint16 Ishort;
    
    // output value (0-256)
    uint8 PWMval;
    
    // From INA219 at I2C 0x41
    uint16 Vmid;
    uint16 Imid;
    uint16 Pmid;
    
    // From INA219 at I2C 0x42
    uint16 Vout;
    uint16 Iout;
    uint16 Pout;
} teg_channel_t;
    
extern teg_channel_t teg_channels[TEG_CHANNEL_COUNT];
extern uint8 enables;

void initializeChannels(void);
void initializeChannel(int index);
int processChannel(teg_channel_t *ch, uint8 newEnables);
void setPWMLevel(int index, uint16 value);
uint8 currentToPWM(uint16 current);

#endif // __teg_channel_h__

/* [] END OF FILE */
