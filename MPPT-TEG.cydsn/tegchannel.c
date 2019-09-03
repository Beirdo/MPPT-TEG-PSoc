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
#include "tegchannel.h"
#include "tca9534.h"
#include "ina219.h"

teg_channel_t teg_channels[TEG_CHANNEL_COUNT];
uint8 enables = 0;

typedef void timerPWMFunc_t(uint32);
timerPWMFunc_t *writeCompareBufFunc[] = {
    Timer_1_WriteCompareBuf, Timer_2_WriteCompareBuf, Timer_3_WriteCompareBuf, Timer_4_WriteCompareBuf,
    Timer_5_WriteCompareBuf, Timer_6_WriteCompareBuf, Timer_7_WriteCompareBuf, Timer_8_WriteCompareBuf,
};
timerPWMFunc_t *setCompareSwapFunc[] = {
    Timer_1_SetCompareSwap, Timer_2_SetCompareSwap, Timer_3_SetCompareSwap, Timer_4_SetCompareSwap,
    Timer_5_SetCompareSwap, Timer_6_SetCompareSwap, Timer_7_SetCompareSwap, Timer_8_SetCompareSwap,
};

void initializeChannels(void) {
    int i;
    memset(&teg_channels, 0x00, sizeof(teg_channels));
    for(i = 0; i < TEG_CHANNEL_COUNT; i++) {
        initializeChannel(i);
    }
}

void initializeChannel(int index) {
    teg_channel_t *ch = &teg_channels[index];
    memset(ch, 0x00, sizeof(teg_channel_t));
    ch->index = index;
    ch->ADCVoff = (index * 2) + 1;
    ch->ADCVon = index * 2;
}

int processChannel(teg_channel_t *ch, uint8 newEnables) {
    int i = ch->index;
    uint8 enableMask = 1 << i;
    uint8 oldEnableBit = ch->enabled & enableMask;
    uint8 newEnabledBit = newEnables & enableMask;
    
    // Channel just got turned on or turned off
    if (oldEnableBit != newEnabledBit) {               
        enables &= ~enableMask;
        enables |= newEnabledBit;
        ch->enabled = newEnabledBit;
        
        // Wipe the channel data (also puts the channel into IDLE state)
        initializeChannel(i);
        
        // Actually enable the circuitry
        TCA9534_write(TCA9534_ENABLES_OUTPUT, enables);
    }
    
    if (!ch->enabled) {
        return 0;
    }
    
    if (ch->state == STATE_IDLE) {
        INA219_initialize(ch->index);
        ch->PWMval = 50;  // about 20%, should be "safe"
        setPWMLevel(i, ch->PWMval);
        ch->state = STATE_FIRST_POINT;
        return 1;
    }
    
    // Store previous values
    ch->prevVin = ch->Vin;
    ch->prevIin = ch->Iin;
    ch->prevPin = ch->Pin;
    
    // We need to read all our current, voltage and power readings from the INA219 chips for the channel
    INA219_read(INA219_INPUT_ADDR, i, &ch->Vin, &ch->Pin, &ch->Iin);
    INA219_read(INA219_MID_ADDR, i, &ch->Vmid, &ch->Pmid, &ch->Imid);
    INA219_read(INA219_OUTPUT_ADDR, i, &ch->Vout, &ch->Pout, &ch->Iout);
    INA219_disconnect();
    
    // We also need to read our Von and Voff from the ADC
    ch->Voff = ADC_1_GetResult16(ch->ADCVoff);
    ch->Von = ADC_1_GetResult16(ch->ADCVon);
    
    // Now need the FSM states (other than IDLE which is covered above)
    
    return 1;
}

void setPWMLevel(int index, uint16 value) {
    timerPWMFunc_t *writeFunc = writeCompareBufFunc[index];
    timerPWMFunc_t *swapFunc = setCompareSwapFunc[index];
    
    writeFunc(value);
    swapFunc(1);
}

/* [] END OF FILE */
