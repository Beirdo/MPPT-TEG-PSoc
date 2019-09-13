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
uint16 Imax = 10000;
uint16 Vmax = 15400;
uint16 deltaPMax = 1500;
uint16 huntDeltaI = 100;

#define PWM_MAX_COUNT 256
#define ABS(x)  ((x) < 0 ? -(x) : (x))

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
}

int processChannel(teg_channel_t *ch, uint8 newEnables) {
    int i = ch->index;
    uint8 enableMask = 1 << i;
    uint8 oldEnableBit = ch->enabled & enableMask;
    uint8 newEnabledBit = newEnables & enableMask;
    uint8 oldPWM;
    double slope;
    int16 deltaI;
    int16 deltaP;
    
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
        ch->PWMval = currentToPWM(huntDeltaI);
        setPWMLevel(i, ch->PWMval);
        ch->state = STATE_FIRST_POINT;
        return 1;
    }
    
    // Store previous values
    ch->prevVin = ch->input.voltage;
    ch->prevIin = ch->input.current;
    ch->prevPin = ch->input.power;
    
    // We need to read all our current, voltage and power readings from the INA219 chips for the channel
    ch->input = INA219_read(INA219_INPUT_ADDR, i);
    ch->middle = INA219_read(INA219_MID_ADDR, i);
    ch->output = INA219_read(INA219_OUTPUT_ADDR, i);
    INA219_disconnect();
    
    // Calculate the deltas
    deltaI = ch->input.current - ch->prevIin;
    deltaP = ch->input.power - ch->prevPin;
    
    oldPWM = ch->PWMval;
    
    // Now need the FSM states (other than IDLE which is covered above)
    switch(ch->state) {
        case STATE_FIRST_POINT:
            ch->state = STATE_SECOND_POINT;
            break;
        case STATE_SECOND_POINT:
            // calculate the Open-Circuit voltage and Short-Circuit current
            if(ch->prevIin == ch->input.current || ch->prevVin == ch->input.voltage) {
                // OK, admit defeat, this gives a vertical line or horizontal line, try another point!
                ch->PWMval = currentToPWM(ch->input.current + huntDeltaI);
                break;
            }
            slope = (double)(ch->input.voltage - ch->prevVin) / (double)(ch->input.current - ch->prevIin);
            ch->Ishort = ch->prevIin - (uint16)((double)(ch->prevVin) / slope);
            ch->Vopen = ch->prevIin - (uint16)((double)(ch->prevIin) * slope);
            
            // pick a point at half of Short-Circuit current, which is approx the optimum
            ch->PWMval = currentToPWM(ch->Ishort / 2);
            ch->state = STATE_HUNT;
            break;
        case STATE_HUNT:
            // Check for large delta, if so, stop hunting
            if (deltaP >= deltaPMax || deltaP <= -deltaPMax) {
                ch->PWMval = currentToPWM(ch->input.current + deltaI);
                ch->state = STATE_SECOND_POINT;
                break;
            }
            // move delta up or down towards the peak
            if (deltaP > 0) {
                if (deltaI > 0) {
                    deltaI = huntDeltaI;
                } else {
                    deltaI = -huntDeltaI;
                }
            } else {
                if (deltaI > 0) {
                    deltaI = -huntDeltaI;
                } else {
                    deltaI = huntDeltaI;
                }
            }
            ch->PWMval = currentToPWM(ch->input.current + deltaI);
            break;
        default:
            // This should never happen!!!
            initializeChannel(i);
            ch->state = STATE_IDLE;
            break;
    }
    
    // Update the PWM value if we changed it.
    if (ch->PWMval != oldPWM) {
        setPWMLevel(i, ch->PWMval);
    }
    
    return 1;
}

uint8 currentToPWM(uint16 current) {
    if (current > Imax) {
        current = Imax;
    }
    return (uint8)((uint32)(PWM_MAX_COUNT) * (uint32)(current) / Imax);
}

void setPWMLevel(int index, uint16 value) {
    timerPWMFunc_t *writeFunc = writeCompareBufFunc[index];
    timerPWMFunc_t *swapFunc = setCompareSwapFunc[index];
    
    writeFunc(value);
    swapFunc(1);
}

/* [] END OF FILE */
