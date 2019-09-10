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

#ifndef __utils_h__
#define __utils_h__

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define clamp(x, y, z) min(max((x), (y)), (z))
#define abs(x) ((x) < 0 ? -(x) : (x))
    
// 32-bit word => ABCD, 16 bit word => CD
#define BYTE_A(x)  (((x) >> 24) & 0xFF)
#define BYTE_B(x)  (((x) >> 16) & 0xFF)
#define BYTE_C(x)  (((x) >> 8)  & 0xFF)
#define BYTE_D(x)  ((x)         & 0xFF)

#define TO_BYTE_A(x) (((x) & 0xFF) << 24)
#define TO_BYTE_B(x) (((x) & 0xFF) << 16)
#define TO_BYTE_C(x) (((x) & 0xFF) << 8)
#define TO_BYTE_D(x) ((x) & 0xFF)
    
#define TICKS_TO_MS(x)  ((x) * 1000 / configTICK_RATE_HZ)
    
#endif // __utils_h__

/* [] END OF FILE */
