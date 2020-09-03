/*---------------------------------------------------------------------
 * File_Name : ADT7475.H
 *
 * Purpose   : 
 * 
 * Copyright(C)      Accton Corporation, 2006
 *
 * Note    : 
 *---------------------------------------------------------------------
 */
#include "sys_hwcfg.h"

#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475) || (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
#ifndef  __ADT7475_H__
#define  __ADT7475_H__

#define ADT7475_ADDR			SYS_HWCFG_I2C_SLAVE_FAN_MONITOR_0

#endif  /* __ADT7475_H__ */
#endif

