/*
 * $Id: m41t11.c,v 1.9 1999/09/28 21:47:10 csm Exp $
 *
 * XGS M41-T11 TOD/I2C Driver
 *
 * Created : S.K.Yang 2002.12.4
 * 
 */

#ifndef __M41_T11_H
#define __M41_T11_H
#include "sys_hwcfg.h"
/*
 * M41T11  -Timekeeping Battery backed I2C.
 */
#define M41T11_ADDR               SYS_HWCFG_I2C_SLAVE_RTC /* RTC, M41T11 */
#define M41T11_ADDR_DATAADDR      0x0		/* RTC, M41T11 */
#define SECOND          0x00
#define MINUTE          0x01
#define HOUR            0x02
#define DAY_OF_WEEK     0x03
#define DAY             0x04
#define MONTH           0x05
#define YEAR            0x06    

#endif /*!__M41_T11_H */

