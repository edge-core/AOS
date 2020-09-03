/* MODULE NAME:  sysdrv_util.c
 * PURPOSE:
 *     This module provides the utility functions to access the registers
 *     through variaus bus according to the given arguments.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    10/31/2014 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysdrv_util.h"
#include "i2cdrv.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SYSDRV_UTIL_DEV_REG_INFO_TYPE_FAN,
    SYSDRV_UTIL_DEV_REG_INFO_TYPE_POWER,
}SYSDRV_UTIL_DevRegInfoType_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T SYSDRV_UTIL_GetRegLenByDevRegInfo(SYSDRV_UTIL_DevRegInfoType_T dev_reg_info_type, const void* dev_reg_info_p, UI8_T *reg_len_p);
static BOOL_T SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo(const SYS_HWCFG_i2cRegInfo_T *reg_info_p, UI8_T *reg_vals_p);
static BOOL_T SYSDRV_UTIL_ReadI2CRegValByI2CWithChannelRegInfo(const SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p, UI8_T *reg_vals_p);

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T SYSDRV_UTIL_GetRegLenByFanRegInfo(const SYS_HWCFG_FanRegInfo_T *reg_info_p, UI8_T *reg_len_p)
{
    if (reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_len_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_len_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return SYSDRV_UTIL_GetRegLenByDevRegInfo(SYSDRV_UTIL_DEV_REG_INFO_TYPE_FAN, (const void*)reg_info_p, reg_len_p);
}

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
BOOL_T SYSDRV_UTIL_GetRegLenByPowerRegInfo(const SYS_HWCFG_PowerRegInfo_T *reg_info_p, UI8_T *reg_len_p)
{
    if (reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_len_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_len_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return SYSDRV_UTIL_GetRegLenByDevRegInfo(SYSDRV_UTIL_DEV_REG_INFO_TYPE_POWER, (const void*)reg_info_p, reg_len_p);
}

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
BOOL_T SYSDRV_UTIL_ReadFanRegByRegInfo(const SYS_HWCFG_FanRegInfo_T *reg_info_p, UI8_T reg_vals_len, UI8_T *reg_vals_p)
{
    UI8_T reg_len_from_reg_info;

    if (reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_vals_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_vals_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (SYSDRV_UTIL_GetRegLenByFanRegInfo(reg_info_p, &reg_len_from_reg_info)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d): Failed to get reg length from reg_info_p\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_vals_len<reg_len_from_reg_info)
    {
        BACKDOOR_MGR_Printf("%s(%d): The length of given reg_vals_p is not large enough(given size=%hu, need size=%hu)\r\n",
            __FUNCTION__, __LINE__, reg_vals_len, reg_len_from_reg_info);
        return FALSE;
    }

    switch (reg_info_p->access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            return SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo(&(reg_info_p->info.i2c), reg_vals_p);
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
            return SYSDRV_UTIL_ReadI2CRegValByI2CWithChannelRegInfo(&(reg_info_p->info.i2c_with_channel), reg_vals_p);
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d): Unsupport access method(%d).\r\n", __FUNCTION__,
                __LINE__, (int)(reg_info_p->access_method));
            break;
    }

    return FALSE;
}

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
BOOL_T SYSDRV_UTIL_ReadPowerRegByRegInfo(const SYS_HWCFG_PowerRegInfo_T *reg_info_p, UI8_T reg_vals_len, UI8_T *reg_vals_p)
{
    UI8_T reg_len_from_reg_info;

    if (reg_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_vals_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d): reg_vals_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (SYSDRV_UTIL_GetRegLenByPowerRegInfo(reg_info_p, &reg_len_from_reg_info)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d): Failed to get reg length from reg_info_p\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (reg_vals_len<reg_len_from_reg_info)
    {
        BACKDOOR_MGR_Printf("%s(%d): The length of given reg_vals_p is not large enough(given size=%hu, need size=%hu)\r\n",
            __FUNCTION__, __LINE__, reg_vals_len, reg_len_from_reg_info);
        return FALSE;
    }

    switch (reg_info_p->access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            return SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo(&(reg_info_p->info.i2c), reg_vals_p);
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
            return SYSDRV_UTIL_ReadI2CRegValByI2CWithChannelRegInfo(&(reg_info_p->info.i2c_with_channel), reg_vals_p);
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d): Unsupport access method(%d).\r\n", __FUNCTION__,
                __LINE__, (int)(reg_info_p->access_method));
            break;
    }

    return FALSE;

}

/* LOCAL SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_GetRegLenByDevRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the length of the register value in byte according to the given
 *          device register info type.
 * INPUT:   dev_reg_info_type -- the type of the device register info.
 *          dev_reg_info_p    -- the pointer to the device register info. the real
 *                               type pointed by this argument depends on
 *                               dev_reg_info_type.
 * OUTPUT:  reg_len_p    -- the length of the device register in the device register
 *                          info.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   None.
 *---------------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_UTIL_GetRegLenByDevRegInfo(SYSDRV_UTIL_DevRegInfoType_T dev_reg_info_type, const void* dev_reg_info_p, UI8_T *reg_len_p)
{
    union
    {
        SYS_HWCFG_FanRegInfo_T   const *fan_reg_info_p;
        SYS_HWCFG_PowerRegInfo_T const *power_reg_info_p;
    } local_dev_reg_info_p;
    union
    {
        SYS_HWCFG_i2cRegInfo_T           const *i2c_p;
        SYS_HWCFG_i2cRegAndChannelInfo_T const *i2c_with_channel_p;
        void                                   *generic_p;
    } local_reg_info_p;
    UI8_T access_method;

    switch (dev_reg_info_type)
    {
        case SYSDRV_UTIL_DEV_REG_INFO_TYPE_FAN:
            local_dev_reg_info_p.fan_reg_info_p=(SYS_HWCFG_FanRegInfo_T*)dev_reg_info_p;
            access_method=local_dev_reg_info_p.fan_reg_info_p->access_method;
            local_reg_info_p.generic_p = (void*)(&(local_dev_reg_info_p.fan_reg_info_p->info));
            break;
        case SYSDRV_UTIL_DEV_REG_INFO_TYPE_POWER:
            local_dev_reg_info_p.power_reg_info_p=(SYS_HWCFG_PowerRegInfo_T*)dev_reg_info_p;
            access_method=local_dev_reg_info_p.power_reg_info_p->access_method;
            local_reg_info_p.generic_p = (void*)(&(local_dev_reg_info_p.power_reg_info_p->info));
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d): Unsupport reg info type(%d).\r\n", __FUNCTION__,
                __LINE__, (int)(dev_reg_info_type));
            return FALSE;
        break;
    }

    switch (access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            *reg_len_p = local_reg_info_p.i2c_p->data_len;
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
            *reg_len_p = local_reg_info_p.i2c_with_channel_p->i2c_reg_info.data_len;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d): Unsupport access method(%d).\r\n", __FUNCTION__,
                __LINE__, (int)(access_method));
            return FALSE;
            break;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the register value according to the given I2C register info
 * INPUT:   reg_info     -- the information about the way to access the register
 * OUTPUT:  reg_vals_p   -- the value read from the register will be output
 *                          to this array. The length of the array must be the
 *                          same with reg_info.info.i2c.data_len
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   1. This function is only called within this .c file.
 *          2. All of the caller shall validate the arguments before calling
 *             this function. This function will not check the input arguments.
 *---------------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo(const SYS_HWCFG_i2cRegInfo_T *reg_info_p, UI8_T *reg_vals_p)
{
    return I2CDRV_TwsiDataReadWithBusIdx(reg_info_p->bus_idx,
        reg_info_p->dev_addr, reg_info_p->op_type, reg_info_p->validOffset,
        reg_info_p->offset, reg_info_p->moreThen256, reg_info_p->data_len,
        reg_vals_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_UTIL_ReadI2CRegValByI2CWithChannelRegInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the register value according to the given I2C with channel info
 * INPUT:   reg_info     -- the information about the way to access the register
 * OUTPUT:  reg_vals_p   -- the value read from the register will be output
 *                          to this array. The length of the array must be the
 *                          same with reg_info.info.i2c.data_len
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   1. This function is only called within this .c file.
 *          2. All of the caller shall validate the arguments before calling
 *             this function. This function will not check the input arguments.
 *---------------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_UTIL_ReadI2CRegValByI2CWithChannelRegInfo(const SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p, UI8_T *reg_vals_p)
{
    BOOL_T rc,ret=TRUE;

    rc = I2CDRV_SetAndLockMux(reg_info_p->i2c_mux_index, reg_info_p->channel_val);
    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d): Failed to set and lock mux.(i2c_mux_index=%hu,channel_val=0x%08lX)\r\n",
            __FUNCTION__, __LINE__, reg_info_p->i2c_mux_index, (unsigned long)reg_info_p->channel_val);
        return FALSE;
    }

    rc = SYSDRV_UTIL_ReadI2CRegValByI2CRegInfo(&(reg_info_p->i2c_reg_info),reg_vals_p);
    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d): Failed to Read reg val(bus_idx=%hu,dev_addr=0x%02X,offset=0x%02X)\r\n",
            __FUNCTION__, __LINE__, reg_info_p->i2c_reg_info.bus_idx,
            reg_info_p->i2c_reg_info.dev_addr, reg_info_p->i2c_reg_info.offset);
        ret=FALSE;
    }

    rc = I2CDRV_UnLockMux(reg_info_p->i2c_mux_index);
    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d): Failed to unlock mux.(i2c_mux_index=%hu,channel_val=0x%08lX)\r\n",
            __FUNCTION__, __LINE__, reg_info_p->i2c_mux_index, (unsigned long)reg_info_p->channel_val);
        ret=FALSE;
    }

    return ret;
}

