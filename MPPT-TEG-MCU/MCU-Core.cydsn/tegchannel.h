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
#include "ina219.h"
#include "interprocessor.h"
        
typedef enum _mppt_state {
    STATE_IDLE = 0,
    STATE_HUNT,
    STATE_FIRST_POINT,
    STATE_SECOND_POINT,
} mppt_state_t;
    
extern teg_channel_t teg_channels[TEG_CHANNEL_COUNT];
extern uint8 enables;
extern teg_channel_t spi_buffer;

void initializeChannels(void);
void initializeChannel(int index);
int processChannel(teg_channel_t *ch, uint8 newEnables);
void setPWMLevel(int index, uint16 value);
uint8 currentToPWM(uint16 current);

#endif // __teg_channel_h__

/* [] END OF FILE */
