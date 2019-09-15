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

#ifndef __convertTime_h__
#define __convertTime_h__
    
#include "project.h"
#include <time.h>
    
int __secs_to_tm(long long t, struct tm *tm);
long long __tm_to_secs(const struct tm *tm);
void _tm_to_rtc(struct tm *tm, RTC_TIME_DATE *rtc);
void _rtc_to_tm(RTC_TIME_DATE *rtc, struct tm *tm);
    
#endif // __convertTime_h__

/* [] END OF FILE */
