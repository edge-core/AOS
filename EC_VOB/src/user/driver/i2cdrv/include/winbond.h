/*---------------------------------------------------------------------
 * File_Name : WINBOND.H
 *
 * Purpose   : 
 * 
 * Copyright(C)      Accton Corporation, 2006
 *
 * Note    : 
 *---------------------------------------------------------------------
 */

#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_WIN83782)
#ifndef  __WINBOND_H__
#define  __WINBOND_H__

#define W38782D_ADDR	      0x29

/* Thermal Temperature */
#define W38782D_THERMAL_1	0x27
#define W38782D_THERMAL_2	0x50
#define W38782D_THERMAL_3	0x50

/* Thermal Over-temperature */
#define W38782D_THERMAL_OVER_H     0x55
#define W38782D_THERMAL_OVER_L     0x56

/* FAN Speed Status */
#define W38782D_FAN_COUNT_0		0x28
#define W38782D_FAN_COUNT_1		0x29
#define W38782D_FAN_COUNT_2		0x2A
/*----------------------------------*/
#define W38782D_FAN_DIVISOR_0	0x47
#define W38782D_FAN_DIVISOR_1	0x4B
#define W38782D_FAN_DIVISOR_2	0x5D


/* Fan Speed Control */
#define W38782D_FAN_CTL_0	0x5B
#define W38782D_FAN_CTL_1	0x5E
#define W38782D_FAN_CTL_2	0x5F

/* Band Setting */
#define W38782D_BANK		0x4E

#define W38782D_DIODE_SELECT          0x59
#define W38782D_MONITOR_CLKINSEL  0x4B
#define W38782D_MONITOR_CHIP_ID    0x58

#if 0
BOOL_T WINBOND_Init();
BOOL_T WINBOND_GetThermal(UI8_T index, I8_T* temperature);
BOOL_T WINBOND_SetThreshold(UI8_T index, I8_T temperature);


BOOL_T WINBOND_GetFan(UI8_T index, UI32_T* speed);
BOOL_T WINBOND_SetFan(UI8_T index, UI32_T speed);
#endif 


#endif  /* __WINBOND_H__ */
#endif

