/* MODULE NAME:  max6581.c
 * PURPOSE:
 *   This module implements the device driver APIs for the thermal sensor IC
 *   MAX6581
 *
 * NOTES:
 *   1. The setting of EXTRANGE in REG 0x41 shall be initialized through
 *      I2CDRV_InitPeripheral(). This driver only read the setting of EXTRANGE
 *      and translate the temperature accordingly
 *
 * HISTORY
 *    12/3/2014 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2014
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
#define MAX_NBR_OF_THERMAL_SENSOR 8

/* Thermal Temperature */
#define MAX6581_REG_THERMAL_1 0x07 /* local thermal sensor temperature */
#define MAX6581_REG_THERMAL_2 0x01 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_3 0x02 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_4 0x03 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_5 0x04 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_6 0x05 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_7 0x06 /* remote sensor temperature */
#define MAX6581_REG_THERMAL_8 0x08 /* remote sensor temperature */

#define MAX6581_REG_THERMAL_EXT_1 0x57
#define MAX6581_REG_THERMAL_EXT_2 0x51
#define MAX6581_REG_THERMAL_EXT_3 0x52
#define MAX6581_REG_THERMAL_EXT_4 0x53
#define MAX6581_REG_THERMAL_EXT_5 0x54
#define MAX6581_REG_THERMAL_EXT_6 0x55
#define MAX6581_REG_THERMAL_EXT_7 0x56
#define MAX6581_REG_THERMAL_EXT_8 0x58

#define MAX6581_REG_CONFIG    0x41 /* Configuration register */

#define THERMAL_VALUE_ARRAY_EXT_INDEX    0
#define THERMAL_VALUE_ARRAY_NORMAL_INDEX 1

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* MAX6581_Config_T: This structure is used to keep the configuration in the
 *                   MAX6581.
 */
typedef struct
{
    BOOL_T is_init;  /* Is the data in this struct already initialized? TRUE => initialized */
    BOOL_T extrange; /* Read from REG 0x41(Configuration Register) bit 1 */
} MAX6581_Config_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T MAX6581_ThermalChipInit(UI32_T thermal_idx);
static BOOL_T MAX6581_CheckAsicConfig(UI32_T thermal_idx, SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T MAX6581_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T MAX6581_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T MAX6581_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p);
static BOOL_T MAX6581_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for thermal functions
 */
SYS_HWCFG_ThermalOps_T thermal_ops_max6581 =
{
    MAX6581_ThermalChipInit,
    MAX6581_ThermalChipGetTemperature
};

/* STATIC VARIABLE DECLARATIONS
 */
static MAX6581_Config_T asic_config_ar[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT]={{FALSE}};

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: MAX6581_ThermalChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do thermal init for the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T MAX6581_ThermalChipInit(UI32_T thermal_idx)
{
    /* do nothing */
    return TRUE;
}

/* FUNCTION NAME: MAX6581_CheckAsicConfig
 *-----------------------------------------------------------------------------
 * PURPOSE: This function checks that whether the corresponding ASIC config
 *          info in asic_config_ar has been initialized. It will do
 *          initialization if the config info has not been initialized yet.
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx        - system-wised thermal index (start from 1)
 *           thermal_ctl_info_p - information about the way to control thermal ASIC
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:    Caller must validate the input arguments before calling this function.
 */
static BOOL_T MAX6581_CheckAsicConfig(UI32_T thermal_idx, SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    UI8_T data;

    if (asic_config_ar[thermal_idx-1].is_init==TRUE)
    {
        return TRUE;
    }

    if (MAX6581_DoRegRead(thermal_ctl_info_p, MAX6581_REG_CONFIG, 1, &data)==TRUE)
    {
        asic_config_ar[thermal_idx-1].is_init==TRUE;
        asic_config_ar[thermal_idx-1].extrange = (data & BIT_1) ? TRUE : FALSE;
        return TRUE;
    }

    return FALSE;
}

/* FUNCTION NAME: MAX6581_PreAction
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
static BOOL_T MAX6581_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
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

/* FUNCTION NAME: MAX6581_PostAction
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
static BOOL_T MAX6581_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
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

/* FUNCTION NAME: MAX6581_DoRegRead
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
static BOOL_T MAX6581_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p)
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

/* FUNCTION NAME: MAX6581_ThermalChipGetTemperature
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the temperature in Celsius degree from the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : temperature    - the temperature in Celsius degree from the given thermal index
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T MAX6581_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    I32_T temperature_offset, eval_temperature;
    const UI8_T thermal_regs[MAX_NBR_OF_THERMAL_SENSOR] = {MAX6581_REG_THERMAL_1,
        MAX6581_REG_THERMAL_2, MAX6581_REG_THERMAL_3, MAX6581_REG_THERMAL_4,
        MAX6581_REG_THERMAL_5, MAX6581_REG_THERMAL_6, MAX6581_REG_THERMAL_7,
        MAX6581_REG_THERMAL_8};
    const UI8_T thermal_ext_regs[MAX_NBR_OF_THERMAL_SENSOR] = {MAX6581_REG_THERMAL_EXT_1,
        MAX6581_REG_THERMAL_EXT_2, MAX6581_REG_THERMAL_EXT_3, MAX6581_REG_THERMAL_EXT_4,
        MAX6581_REG_THERMAL_EXT_5, MAX6581_REG_THERMAL_EXT_6, MAX6581_REG_THERMAL_EXT_7,
        MAX6581_REG_THERMAL_EXT_8};
    UI8_T reg_val[2];
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

    if (MAX6581_PreAction(&thermal_ctl_info)==FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (MAX6581_CheckAsicConfig(thermal_idx, &thermal_ctl_info)==FALSE)
    {
        printf("%s(%d): MAX6581_CheckAsicConfig error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (asic_config_ar[thermal_idx-1].extrange==TRUE)
    {
        temperature_offset=64;
    }
    else
    {
        temperature_offset=0;
    }

    /* According to the datasheet, the thermal ext register should be read first
     * to ensure the value got from the thermal register is for the measurement
     * of the same time point
     */
    if ((MAX6581_DoRegRead(&thermal_ctl_info, thermal_ext_regs[thermal_ctl_info.thermal_ctl_internal_thermal_idx], 1, &reg_val[THERMAL_VALUE_ARRAY_EXT_INDEX])==TRUE)&&
        (MAX6581_DoRegRead(&thermal_ctl_info, thermal_regs[thermal_ctl_info.thermal_ctl_internal_thermal_idx], 1, &reg_val[THERMAL_VALUE_ARRAY_NORMAL_INDEX])==TRUE))
    {
        /* eval_temperature is in unit of 0.125 Celsius degree */
    	eval_temperature = (reg_val[THERMAL_VALUE_ARRAY_NORMAL_INDEX] - temperature_offset) << 3;
    	eval_temperature |= reg_val[THERMAL_VALUE_ARRAY_EXT_INDEX] >> 5;

        *temperature = (eval_temperature*125)/1000;
    }
    else
    {
        ret_val=FALSE;
        printf("%s(%d): Failed to read thermal reg.(thermal_idx=%hu,internal_thermal_idx=%lu)\r\n", __FUNCTION__,
            __LINE__, thermal_idx, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
    }

    /* Call MAX6581_PostAction() to do required clean-up actions
     * after accessing MAX6581
     */
    if (MAX6581_PostAction(&thermal_ctl_info)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;
}

