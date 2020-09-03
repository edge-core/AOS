/* MODULE NAME:  sysdrv_util.h
 * PURPOSE:
 *     This module provides the utility functions to access the registers
 *     through variaus bus according to the given arguments.
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    10/31/2014 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */
#ifndef SYSDRV_UTIL_H
#define SYSDRV_UTIL_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_hwcfg_common.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_GetRegLenByFanRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the length of the register value in byte according to the given
 *          fan register info
 * INPUT:   reg_info     -- the information about the way to access the Fan
 *                          register
 * OUTPUT:  reg_len_p    -- the length of the fan register in the fan register
 *                          info.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   None.
 *---------------------------------------------------------------------------------
 */
BOOL_T SYSDRV_UTIL_GetRegLenByFanRegInfo(const SYS_HWCFG_FanRegInfo_T *reg_info_p, UI8_T *reg_len_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_GetRegLenByPowerRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the length of the register value in byte according to the given
 *          fan register info
 * INPUT:   reg_info     -- the information about the way to access the Power
 *                          register
 * OUTPUT:  reg_len_p    -- the length of the power register in the power register
 *                          info.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   None.
 *---------------------------------------------------------------------------------
 */
BOOL_T SYSDRV_UTIL_GetRegLenByPowerRegInfo(const SYS_HWCFG_PowerRegInfo_T *reg_info_p, UI8_T *reg_len_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_ReadFanRegByRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the register value according to the given fan register info
 * INPUT:   reg_info     -- the information about the way to access the Fan
 *                          register
 *          reg_vals_len -- the size of reg_vals_p in byte.  The length of the
 *                          array must be the larger than or equal to the value
 *                          in reg_info.info.i2c.data_len or
 *                          reg_info.info.i2c_with_channel.i2c_reg_info.data_len
 *                          (can be got through function SYSDRV_UTIL_GetRegLenByFanRegInfo)
 * OUTPUT:  reg_vals_p   -- the value read from the register will be output
 *                          to this array.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   None.
 *---------------------------------------------------------------------------------
 */
BOOL_T SYSDRV_UTIL_ReadFanRegByRegInfo(const SYS_HWCFG_FanRegInfo_T *reg_info_p, UI8_T reg_vals_len, UI8_T *reg_vals_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_ReadPowerRegByRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the register value according to the given power register info
 * INPUT:   reg_info     -- the information about the way to access the Power
 *                          register
 *          reg_vals_len -- the size of reg_vals_p in byte.  The length of the
 *                          array must be the larger than or equal to the value
 *                          in reg_info.info.i2c.data_len or
 *                          reg_info.info.i2c_with_channel.i2c_reg_info.data_len
 *                          (can be got through function SYSDRV_UTIL_GetRegLenByPowerRegInfo)
 * OUTPUT:  reg_vals_p   -- the value read from the register will be output
 *                          to this array.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   None.
 *---------------------------------------------------------------------------------
 */
BOOL_T SYSDRV_UTIL_ReadPowerRegByRegInfo(const SYS_HWCFG_PowerRegInfo_T *reg_info_p, UI8_T reg_vals_len, UI8_T *reg_vals_p);

#endif    /* End of SYSDRV_UTIL_H */
