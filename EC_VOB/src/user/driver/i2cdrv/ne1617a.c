/* MODULE NAME:  ne1617a.c
 * PURPOSE:
 *   This module implements the device driver APIs for the thermal sensor IC
 *   NE1617A.
 *
 * NOTES:
 *
 * HISTORY
 *    11/27/2012 - Charlie Chen, Created
 *
 * Copyright(C)      Edge-Core Networks, 2012
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_type.h"
#include "sys_hwcfg.h"
#include "stktplg_board.h"
#include "i2cdrv.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* max number of thermal sensors supported by this ASIC
 */
#define MAX_NBR_OF_THERMAL_SENSOR 2

/* Thermal Temperature */
#define NE1617A_REG_THERMAL_1 0x00 /* local thermal sensor temperature */
#define NE1617A_REG_THERMAL_2 0x01 /* remote sensor temperature */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T NE1617A_ThermalChipInit(UI32_T thermal_idx);
static BOOL_T NE1617A_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T NE1617A_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T NE1617A_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p);
static BOOL_T NE1617A_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for thermal functions
 */
SYS_HWCFG_ThermalOps_T thermal_ops_ne1617a =
{
    NE1617A_ThermalChipInit,
    NE1617A_ThermalChipGetTemperature
};

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: NE1617A_ThermalChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do thermal init for the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T NE1617A_ThermalChipInit(UI32_T thermal_idx)
{
    /* do nothing */
    return TRUE;
}

/* FUNCTION NAME: NE1617A_PreAction
 *-----------------------------------------------------------------------------
 * PURPOSE: Before doing operation to the thermal ASIC, need to call this
 *          function to do preparation operations such as open channel on
 *          I2C mux.
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_ctl_info_p - information about the way to control thermal ASIC
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T NE1617A_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = &(thermal_ctl_info_p->reg_info.info.i2c_with_channel);

            if (I2CDRV_SetAndLockMux(reg_info_p->i2c_mux_index, reg_info_p->channel_val)==FALSE)
                return FALSE;
        }
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            /* do nothing */
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, thermal_ctl_info_p->reg_info.access_method);
            return FALSE;
            break;
    }

    return TRUE;
}

/* FUNCTION NAME: NE1617A_PostAction
 *-----------------------------------------------------------------------------
 * PURPOSE: After doing operation to the thermal ASIC, need to call this
 *          function to do clean-up operations such as close channel on
 *          I2C mux.
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_ctl_info_p - information about the way to control thermal ASIC
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T NE1617A_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = &(thermal_ctl_info_p->reg_info.info.i2c_with_channel);

            if (I2CDRV_UnLockMux(reg_info_p->i2c_mux_index)==FALSE)
                return FALSE;
        }
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            /* do nothing */
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, thermal_ctl_info_p->reg_info.access_method);
            return FALSE;
            break;
    }
    return TRUE;
}

/* FUNCTION NAME: NE1617A_DoRegRead
 *-----------------------------------------------------------------------------
 * PURPOSE: Perform register read operation to the ASIC.
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_ctl_info_p - information about the way to control thermal ASIC
 *           reg_addr           - the register address to be read
 *           data_len           - the length of data to be read
 * OUTPUT  : data_p             - the data read from the given register address
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T NE1617A_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p)
{
    UI8_T i;

    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
        {
            SYS_HWCFG_i2cRegInfo_T *reg_info_p=&(thermal_ctl_info_p->reg_info.info.i2c_with_channel.i2c_reg_info);

            for (i=0; i<data_len; i++)
            {
                if (I2CDRV_TwsiDataReadWithBusIdx(reg_info_p->bus_idx,
                    reg_info_p->dev_addr, I2C_7BIT_ACCESS_MODE,
                    TRUE, reg_addr + i,
                    FALSE, 1, data_p+i) == FALSE)
                    return FALSE;
            }
        }
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, thermal_ctl_info_p->reg_info.access_method);
            return FALSE;
            break;
    }
    return TRUE;
}

/* FUNCTION NAME: NE1617A_ThermalChipGetTemperature
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the temperature in Celsius degree from the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : temperature    - the temperature in Celsius degree from the given thermal index
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T NE1617A_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    const UI8_T thermal_regs[MAX_NBR_OF_THERMAL_SENSOR] = {NE1617A_REG_THERMAL_1, NE1617A_REG_THERMAL_2};
    UI8_T reg_val;
    BOOL_T ret_val=TRUE;

    if (thermal_idx > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
        return FALSE;

    if (STKTPLG_BOARD_GetThermalControllerInfo(thermal_idx, &thermal_ctl_info)==FALSE)
    {
        return FALSE;
    }

    if (thermal_ctl_info.thermal_ctl_internal_thermal_idx>MAX_NBR_OF_THERMAL_SENSOR)
    {
        printf("%s(%d): Invalid internal thermal sensor index %lu(thermal_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, thermal_ctl_info.thermal_ctl_internal_thermal_idx, thermal_idx);
        return FALSE;
    }

    if (NE1617A_PreAction(&thermal_ctl_info)==FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (NE1617A_DoRegRead(&thermal_ctl_info, thermal_regs[thermal_ctl_info.thermal_ctl_internal_thermal_idx],
        1, &reg_val)==TRUE)
    {
        /* temperature register value is consisted by 8 bits.
         * bit [7] is signed bit. bit [6:0] is value in Celsius degree
         */
        *temperature = (I8_T)reg_val;
    }
    else
    {
        ret_val=FALSE;
        printf("%s(%d): Failed to read thermal reg.(thermal_idx=%hu,internal_thermal_idx=%lu)\r\n", __FUNCTION__,
            __LINE__, thermal_idx, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
    }

    /* Call NE1617A_PostAction() to do required clean-up actions
     * after accessing NE1617A
     */
    if (NE1617A_PostAction(&thermal_ctl_info)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;
}

