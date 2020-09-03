/* MODULE NAME:  win83782.C
 * PURPOSE:
 *   This module implements the device driver APIs for the Hardware Monitor IC
 *   Winbond W83782D/W83782G.
 *
 * NOTES:
 *
 * HISTORY
 *    11/25/2012 - Charlie Chen, Created
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
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* max number of fan speed detection supported by this ASIC
 */
#define MAX_NBR_OF_FAN_SPEED_DETECT 3

/* max number of fan speed control supported by this ASIC
 */
#define MAX_NBR_OF_FAN_SPEED_CONTROL 4

/* max number of thermal sensors supported by this ASIC
 */
#define MAX_NBR_OF_THERMAL_SENSOR 3

/* Thermal Temperature */
#define W38782D_REG_THERMAL_1 0x27 /* Bank 0 */
#define W38782D_REG_THERMAL_2 0x50 /* Bank 1 */
#define W38782D_REG_THERMAL_3 0x50 /* Bank 2 */

/* FAN Speed Detect (Bank 0) */
#define W83782D_REG_FAN_COUNT_0 0x28
#define W83782D_REG_FAN_COUNT_1 0x29
#define W83782D_REG_FAN_COUNT_2 0x2A
/*----------------------------------*/
#define W83782D_REG_FAN_DIVISOR_0 0x47
#define W83782D_REG_FAN_DIVISOR_1 0x4B
#define W83782D_REG_FAN_DIVISOR_2 0x5D

/* Fan Speed Control (Bank 0) */
#define W38782D_REG_FAN_SPEED_CTL_0 0x5B
#define W38782D_REG_FAN_SPEED_CTL_1 0x5A
#define W38782D_REG_FAN_SPEED_CTL_2 0x5E
#define W38782D_REG_FAN_SPEED_CTL_3 0x5F

/* Bank Setting */
#define WIN83782_REG_BANK 0x4E

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef union
{
    SYS_HWCFG_i2cRegInfo_T *i2c_p; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C */
    SYS_HWCFG_i2cRegAndChannelInfo_T *i2c_with_channel_p; /* used when access_method is SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL */
}WIN83782D_ctl_info_ptr_U;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* common utility functions
 */
static BOOL_T WIN83782D_PreAction(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method);
static BOOL_T WIN83782D_PostAction(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method);
static BOOL_T WIN83782D_DoRegRead(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p);
static BOOL_T WIN83782D_DoRegWrite(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T reg_addr, UI8_T *data_p, UI8_T data_len);
static BOOL_T WIN83782D_SetBank(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T num);

/* fan controller functions
 */
static BOOL_T WIN83782D_FanChipInit(UI32_T fan_idx);
static BOOL_T WIN83782D_FillCtlInfoFromFanCtlInfo(SYS_HWCFG_FanControllerInfo_T *fan_ctl_info_p, WIN83782D_ctl_info_ptr_U *ctl_info_p);
static BOOL_T WIN83782D_FanChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed);
static BOOL_T WIN83782D_FanChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p);
static BOOL_T WIN83782D_FanChipSetSpeed(UI8_T fan_idx, UI32_T speed);

/* thermal controller functions
 */
static BOOL_T WIN83782D_ThermalChipInit(UI32_T thermal_idx);
static BOOL_T WIN83782D_FillCtlInfoFromThermalCtlInfo(SYS_HWCFG_ThermalControlInfo_T* thermal_ctl_info_p, WIN83782D_ctl_info_ptr_U *ctl_info_p);
static BOOL_T WIN83782D_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for fan controller functions
 */
SYS_HWCFG_FanControllerOps_T fan_controller_ops_win83782 =
{
    WIN83782D_FanChipInit,
    WIN83782D_FanChipGetSpeedInRpm,
    WIN83782D_FanChipGetSpeedInDutyCycle,
    WIN83782D_FanChipSetSpeed,
};

/* exported variable for thermal functions
 */
SYS_HWCFG_ThermalOps_T thermal_ops_w83782 =
{
    WIN83782D_ThermalChipInit,
    WIN83782D_ThermalChipGetTemperature
};

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* common utility functions -- START */
/* FUNCTION NAME: WIN83782D_PreAction
 *-----------------------------------------------------------------------------
 * PURPOSE: Before doing operation to the thermal ASIC, need to call this
 *          function to do preparation operations such as open channel on
 *          I2C mux.
 *-----------------------------------------------------------------------------
 * INPUT   : ctl_info      - information about the way to control ASIC
 *           access_method - One of the constant in sys_hwcfg_common.h prefixed
 *                           with "SYS_HWCFG_REG_ACCESS_METHOD_"
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_PreAction(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method)
{
    switch (access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = ctl_info.i2c_with_channel_p;

            if (I2CDRV_SetAndLockMux(reg_info_p->i2c_mux_index, reg_info_p->channel_val)==FALSE)
                return FALSE;
        }
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, access_method);
            return FALSE;
            break;
    }

    return TRUE;
}

/* FUNCTION NAME: WIN83782D_PostAction
 *-----------------------------------------------------------------------------
 * PURPOSE: After doing operation to the thermal ASIC, need to call this
 *          function to do clean-up operations such as close channel on
 *          I2C mux.
 *-----------------------------------------------------------------------------
 * INPUT   : ctl_info      - information about the way to control ASIC
 *           access_method - One of the constant in sys_hwcfg_common.h prefixed
 *                           with "SYS_HWCFG_REG_ACCESS_METHOD_"
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_PostAction(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method)
{
    switch (access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = ctl_info.i2c_with_channel_p;

            if (I2CDRV_UnLockMux(reg_info_p->i2c_mux_index)==FALSE)
                return FALSE;
        }
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, access_method);
            return FALSE;
            break;
    }
    return TRUE;
}

/* FUNCTION NAME: WIN83782D_DoRegRead
 *-----------------------------------------------------------------------------
 * PURPOSE: Perform register read operation to the ASIC.
 *-----------------------------------------------------------------------------
 * INPUT   : ctl_info_p         - information about the way to control ASIC
 *           access_method      - One of the constant in sys_hwcfg_common.h prefixed
 *                                with "SYS_HWCFG_REG_ACCESS_METHOD_"
 *           reg_addr           - the register address to be read
 *           data_len           - the length of data to be read
 * OUTPUT  : data_p             - the data read from the given register address
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_DoRegRead(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p)
{
    UI8_T i;

    switch (access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegInfo_T *reg_info_p=&((ctl_info.i2c_with_channel_p)->i2c_reg_info);

            for (i=0; i<data_len; i++)
            {
                if (I2CDRV_TwsiDataReadWithBusIdx(reg_info_p->bus_idx, 
                    reg_info_p->dev_addr, reg_info_p->op_type,
                    TRUE, reg_addr + i,
                    reg_info_p->moreThen256, 1, data_p+i) == FALSE)
                    return FALSE;
            }
        }
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, access_method);
            return FALSE;
            break;
    }
    return TRUE;
}

/* FUNCTION NAME: WIN83782D_DoRegWrite
 *-----------------------------------------------------------------------------
 * PURPOSE: Perform register write operation to the ASIC.
 *-----------------------------------------------------------------------------
 * INPUT   : ctl_info_p         - information about the way to control ASIC
 *           access_method      - One of the constant in sys_hwcfg_common.h prefixed
 *                                with "SYS_HWCFG_REG_ACCESS_METHOD_"
 *           reg_addr           - the register address to be written
 *           data_p             - the data to be written to the given register address
 *           data_len           - the length of data to write
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_DoRegWrite(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T reg_addr, UI8_T *data_p, UI8_T data_len)
{
    UI8_T i;

    switch (access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegInfo_T *reg_info_p=&((ctl_info.i2c_with_channel_p)->i2c_reg_info);

            for (i=0; i<data_len; i++)
            {
                if (I2CDRV_TwsiDataWriteWithBusIdx(reg_info_p->bus_idx, 
                    reg_info_p->dev_addr, reg_info_p->op_type,
                    TRUE, reg_addr+i,
                    reg_info_p->moreThen256, data_p+i, 1) == FALSE)
                    return FALSE;
            }
        }
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, access_method);
            return FALSE;
            break;
    }
    return TRUE;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_SetBank
 *---------------------------------------------------------------------------
 * PURPOSE: Set W83782 bank
 * INPUT:   ctl_info       -  controller info
 *          num            -  W83782 bank num to be set
 * OUTPUT:
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE: This function can only be called after WIN83782D_PreAction() is called.
 */
static BOOL_T WIN83782D_SetBank(WIN83782D_ctl_info_ptr_U ctl_info, UI8_T access_method, UI8_T num)
{
    UI8_T data;

    if (num > 7)
    {
        printf ("%s(%d) Invalid bank num(%hu)\r\n", __FUNCTION__, __LINE__, num);
        return FALSE;
    }

    data = 0x80|num;
    if (WIN83782D_DoRegWrite(ctl_info, access_method, WIN83782_REG_BANK, &data, 1)==FALSE)
    {
        printf("%s():I2C write error!\n", __FUNCTION__);
    }

    return TRUE;
}
/* Common utility functions -- END */

/* fan controller related functions START */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FanChipInit
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to do fan init for the given fan index
 * INPUT:   fan_idx        -  system-wised fan index (start from 1)
 * OUTPUT:
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */
static BOOL_T WIN83782D_FanChipInit(UI32_T fan_idx)
{
    /* do nothing
     */
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FillCtlInfoFromFanCtlInfo
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to convert data in SYS_HWCFG_FanControllerInfo_T
 *          to WIN83782D_ctl_info_ptr_U format.
 * INPUT:   fan_ctl_info_p - information about the way to control fan ASIC
 * OUTPUT:  ctl_info_p     - control information in WIN83782D_ctl_info_ptr_U format
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */
static BOOL_T WIN83782D_FillCtlInfoFromFanCtlInfo(SYS_HWCFG_FanControllerInfo_T *fan_ctl_info_p, WIN83782D_ctl_info_ptr_U *ctl_info_p)
{
    if (fan_ctl_info_p->reg_info.access_method == SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    {
        ctl_info_p->i2c_p = &(fan_ctl_info_p->reg_info.info.i2c);
        return TRUE;
    }
    else if (fan_ctl_info_p->reg_info.access_method == SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    {
        ctl_info_p->i2c_with_channel_p = &(fan_ctl_info_p->reg_info.info.i2c_with_channel);
        return TRUE;
    }
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FanChipGetSpeedInRpm
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in RPM for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in RPM.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T WIN83782D_FanChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    WIN83782D_ctl_info_ptr_U ctl_info;
    BOOL_T ret_val = TRUE;
    UI8_T divisor[3], count;
    UI8_T reg_val[3];
    UI8_T divisor_offset[3] = {W83782D_REG_FAN_DIVISOR_0, W83782D_REG_FAN_DIVISOR_1, W83782D_REG_FAN_DIVISOR_2};
    int i;

    if (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
        return FALSE;

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
        return FALSE;

    if (WIN83782D_FillCtlInfoFromFanCtlInfo(&fan_ctl_info, &ctl_info) == FALSE)
        return FALSE;

    if (fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx > MAX_NBR_OF_FAN_SPEED_DETECT)
    {
        printf("%s(%d): Invalid fan controller internal fan speed detect index %lu(fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx, fan_idx);
        return FALSE;
    }

    /* Call WIN83782D_PreAction() to do required preparation setup actions
     * before accessing W83782
     */
    if (WIN83782D_PreAction(ctl_info, fan_ctl_info.reg_info.access_method) == FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Select Bank 0
     */
    if (WIN83782D_SetBank(ctl_info, fan_ctl_info.reg_info.access_method, 0) != TRUE) 
    {
        printf("%s(%d): Failed to set bank reg on fan index %hu\r\n",
            __FUNCTION__, __LINE__, fan_idx);
        ret_val = FALSE;
        goto fail_exit;
    }

    /* Read Divisor Register
     */
    for (i=0; i < 3; i++)
    {
        if (WIN83782D_DoRegRead(ctl_info, fan_ctl_info.reg_info.access_method, divisor_offset[i], 1, reg_val+i) != TRUE)
        {
            printf("%s(%d): Failed to read divisor registers\r\n", __FUNCTION__, __LINE__);
            ret_val = FALSE;
            goto fail_exit;
        }
    }

    /* Calculate divisor
     */
    divisor[0] = ((reg_val[0] & 0x30)>>4) | ( (reg_val[2] & 0x20)>>3);
    divisor[1] = ((reg_val[0] & 0xC0)>>6) | ( (reg_val[2] & 0x40)>>4);
    divisor[2] = ((reg_val[1] & 0xC0)>>6) | ( (reg_val[2] & 0x80)>>5);

    for (i = 0; i < 3; i++)
    {
        divisor[i] = ( divisor[i] == 0 ) ? 1 : (2 << (divisor[i]-1));
    }

    /* Read Counter
     */
    if (WIN83782D_DoRegRead(ctl_info, fan_ctl_info.reg_info.access_method, W83782D_REG_FAN_COUNT_0+fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx, 1, &count) != TRUE)
    {
        printf("%s(%d): Failed to read counters\r\n", __FUNCTION__, __LINE__);
        ret_val = FALSE;
        goto fail_exit;

    }

    /* count = 255 mean the lowest speed so we think the speed is 0 */
    if (count == 0xFF)
    {
        *speed = 0;
        goto fail_exit;
    }
    
    if ((count != 0) && (divisor[fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx] != 0))
    {
        *speed = 1350000/(divisor[fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx] * count);
    }
    else
    {
        ret_val = FALSE;
        goto fail_exit;
    }

fail_exit:
    /* Call WIN83782D_PostAction() to do required clean-up actions
     * after accessing W83782
     */
    if (WIN83782D_PostAction(ctl_info, fan_ctl_info.reg_info.access_method)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FanChipGetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  duty_cycle_p   - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T WIN83782D_FanChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    WIN83782D_ctl_info_ptr_U ctl_info;
    BOOL_T ret_val=TRUE;
    UI8_T data;
    const UI8_T fan_speed_ctl_regs[] = {W38782D_REG_FAN_SPEED_CTL_0,
                                        W38782D_REG_FAN_SPEED_CTL_1,
                                        W38782D_REG_FAN_SPEED_CTL_2,
                                        W38782D_REG_FAN_SPEED_CTL_3};

    if (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
        return FALSE;

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
        return FALSE;

    if (fan_ctl_info.fan_ctl_internal_fan_speed_control_idx > MAX_NBR_OF_FAN_SPEED_CONTROL)
    {
        printf("%s(%d): Invalid fan controller internal fan speed control index %lu\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx);
        return FALSE;
    }

    if (WIN83782D_FillCtlInfoFromFanCtlInfo(&fan_ctl_info, &ctl_info) == FALSE)
        return FALSE;

    /* Call WIN83782D_PreAction() to do required preparation setup actions
     * before accessing W83782
     */
    if (WIN83782D_PreAction(ctl_info, fan_ctl_info.reg_info.access_method) == FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Select Bank 0
     */
    if (WIN83782D_SetBank(ctl_info, fan_ctl_info.reg_info.access_method, 0) != TRUE) 
    {
        printf("%s(%d): Failed to set bank reg on fan index %hu\r\n",
            __FUNCTION__, __LINE__, fan_idx);
        ret_val=FALSE;
        goto fail_exit;
    }

    /* Read fan speed control registr */
    if (WIN83782D_DoRegRead(ctl_info, fan_ctl_info.reg_info.access_method, fan_speed_ctl_regs[fan_ctl_info.fan_ctl_internal_fan_speed_control_idx], 1, &data) != TRUE)
    {
        printf("%s(%d): Failed to read fan speed control reg(fan_idx=%hu,internal_idx=%lu)\r\n", __FUNCTION__, __LINE__, fan_idx, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx);
        ret_val=FALSE;
        goto fail_exit;
    }

    /* convert reg value(range:0-255) to duty cycle(range:0-100)
     */
    *duty_cycle_p = data * 100UL / 255UL;

fail_exit:
    /* Call WIN83782D_PostAction() to do required clean-up actions
     * after accessing W83782
     */
    if (WIN83782D_PostAction(ctl_info, fan_ctl_info.reg_info.access_method)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FanChipSetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to set fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T WIN83782D_FanChipSetSpeed(UI8_T fan_idx, UI32_T speed)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    WIN83782D_ctl_info_ptr_U ctl_info;
    BOOL_T ret_val=TRUE;
    UI8_T data;
    const UI8_T fan_speed_ctl_regs[] = {W38782D_REG_FAN_SPEED_CTL_0,
                                        W38782D_REG_FAN_SPEED_CTL_1,
                                        W38782D_REG_FAN_SPEED_CTL_2,
                                        W38782D_REG_FAN_SPEED_CTL_3};

    if (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
        return FALSE;

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
        return FALSE;

    if (fan_ctl_info.fan_ctl_internal_fan_speed_control_idx > MAX_NBR_OF_FAN_SPEED_CONTROL)
    {
        printf("%s(%d): Invalid fan controller internal fan speed control index %lu\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx);
        return FALSE;
    }

    if (WIN83782D_FillCtlInfoFromFanCtlInfo(&fan_ctl_info, &ctl_info) == FALSE)
        return FALSE;

    /* Call WIN83782D_PreAction() to do required preparation setup actions
     * before accessing W83782
     */
    if (WIN83782D_PreAction(ctl_info, fan_ctl_info.reg_info.access_method) == FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Select Bank 0
     */
    if (WIN83782D_SetBank(ctl_info, fan_ctl_info.reg_info.access_method, 0) != TRUE) 
    {
        printf("%s(%d): Failed to set bank reg on fan index %hu\r\n",
            __FUNCTION__, __LINE__, fan_idx);
        ret_val=FALSE;
        goto fail_exit;
    }


    /* speed is duty cycle, which should be in range 0 to 100
     * forced the value to be 100 if it is greater than 100
     */
    if(speed>100)
        speed=100;

    /* convert duty cycle(range:0-100) to register value(range:0-255)
     */
    data = (UI8_T)((speed*255UL)/100UL);
    /* Write fan speed control registr */
    if (WIN83782D_DoRegWrite(ctl_info, fan_ctl_info.reg_info.access_method, fan_speed_ctl_regs[fan_ctl_info.fan_ctl_internal_fan_speed_control_idx], &data, 1) != TRUE)
    {
        printf("%s(%d): Failed to write fan speed control reg(fan_idx=%hu,internal_idx=%lu)\r\n", __FUNCTION__, __LINE__, fan_idx, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx);
        ret_val=FALSE;
        goto fail_exit;
    }

fail_exit:
    /* Call WIN83782D_PostAction() to do required clean-up actions
     * after accessing W83782
     */
    if (WIN83782D_PostAction(ctl_info, fan_ctl_info.reg_info.access_method)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;

}
/* fan controller related functions END */

/* thermal controller related functions START */
/* FUNCTION NAME: WIN83782D_ThermalChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do thermal init for the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_ThermalChipInit(UI32_T thermal_idx)
{
    /* do nothing */
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WIN83782D_FillCtlInfoFromThermalCtlInfo
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to convert data in SYS_HWCFG_ThermalControlInfo_T
 *          to WIN83782D_ctl_info_ptr_U format.
 * INPUT:   thermal_ctl_info_p - information about the way to control thermal ASIC
 * OUTPUT:  ctl_info_p         - control information in WIN83782D_ctl_info_ptr_U format
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */
static BOOL_T WIN83782D_FillCtlInfoFromThermalCtlInfo(SYS_HWCFG_ThermalControlInfo_T* thermal_ctl_info_p, WIN83782D_ctl_info_ptr_U *ctl_info_p)
{
    if (thermal_ctl_info_p->reg_info.access_method == SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    {
        ctl_info_p->i2c_p = &(thermal_ctl_info_p->reg_info.info.i2c);
        return TRUE;
    }
    else if (thermal_ctl_info_p->reg_info.access_method == SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    {
        ctl_info_p->i2c_with_channel_p = &(thermal_ctl_info_p->reg_info.info.i2c_with_channel);
        return TRUE;
    }
    return FALSE;
}

/* FUNCTION NAME: WIN83782D_ThermalChipGetTemperature
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the temperature in Celsius degree from the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : temperature    - the temperature in Celsius degree from the given thermal index
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T WIN83782D_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    WIN83782D_ctl_info_ptr_U       ctl_info;
    BOOL_T                         ret_val = TRUE;
    UI8_T                          data[2];
    const UI8_T thermal_regs[MAX_NBR_OF_THERMAL_SENSOR] =
        {
            W38782D_REG_THERMAL_1,
            W38782D_REG_THERMAL_2,
            W38782D_REG_THERMAL_3
        };
    const UI8_T thermal_banks[MAX_NBR_OF_THERMAL_SENSOR] = { 0, 1, 2 };

    if (STKTPLG_BOARD_GetThermalControllerInfo(thermal_idx, &thermal_ctl_info) == FALSE)
        return FALSE;

    if (thermal_ctl_info.thermal_ctl_internal_thermal_idx > MAX_NBR_OF_THERMAL_SENSOR)
    {
        printf("%s(%d): Invalid thermal controller internal thermal sensor index %lu\r\n",
            __FUNCTION__, __LINE__, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
        return FALSE;
    }

    if (WIN83782D_FillCtlInfoFromThermalCtlInfo(&thermal_ctl_info, &ctl_info)==FALSE)
        return FALSE;

    /* Call WIN83782D_PreAction() to do required preparation setup actions
     * before accessing W83782
     */
    if (WIN83782D_PreAction(ctl_info, thermal_ctl_info.reg_info.access_method) == FALSE)
    {
        printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Select Bank
     */
    {

        if (WIN83782D_SetBank(ctl_info, thermal_ctl_info.reg_info.access_method, thermal_banks[thermal_ctl_info.thermal_ctl_internal_thermal_idx]) != TRUE) 
        {
            printf("%s(%d):Failed to set bank reg on thermal index %hu\r\n",
                __FUNCTION__, __LINE__, thermal_idx);
            ret_val = FALSE;
            goto fail_exit;
        }
    }

    /* Read thermal sensor register value
     */
    if (thermal_ctl_info.thermal_ctl_internal_thermal_idx==0)
    {
        /* internal thermal sensor 1 has 8-bit value
         * MSB(bit 7) is signed bit
         * bit[6:0] is temperature value in unit of 1 Celsius degree
         */
        if (WIN83782D_DoRegRead(ctl_info, thermal_ctl_info.reg_info.access_method,
            thermal_regs[thermal_ctl_info.thermal_ctl_internal_thermal_idx],
            1, data) == FALSE)
        {
            printf("%s(%d):Failed to read thermal reg on thermal_idx %hu(internal idx=%lu)\r\n",
                __FUNCTION__, __LINE__, thermal_idx, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
            ret_val = FALSE;
            goto fail_exit;
        }
        *temperature = (I8_T)data[0];
    }
    else
    {
        UI16_T tmp_data;
        I16_T temp_val;

        /* internal thermal sensor 2/3 has 9-bit value
         * MSB is signed bit, bit[8:0] is temperature value in unit of 0.5 Celsius degree
         */
        if (WIN83782D_DoRegRead(ctl_info, thermal_ctl_info.reg_info.access_method,
            thermal_regs[thermal_ctl_info.thermal_ctl_internal_thermal_idx],
            2, data) == FALSE)
        {
            printf("%s(%d):Failed to read thermal reg on thermal_idx %hu(internal idx=%lu)\r\n",
                __FUNCTION__, __LINE__, thermal_idx, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
            ret_val = FALSE;
            goto fail_exit;
        }

        /* data[0] contains bit[9:1], data[1]:bit 7 contains bit[0]
         */
        tmp_data = data[0]<<1 | (data[1]>>7);

        if(tmp_data & BIT_9)
        {
            /* tmp_data contains 2-complenet negative value
             */
            temp_val = (-1) * (((~tmp_data) & 0x1FF) + 1);
        }
        else
            temp_val = (I16_T)tmp_data;

        *temperature = temp_val/2;
    }

fail_exit:
    /* Call WIN83782D_PostAction() to do required clean-up actions
     * after accessing W83782
     */
    if (WIN83782D_PostAction(ctl_info, thermal_ctl_info.reg_info.access_method)==FALSE)
    {
        printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;
}
/* thermal controller related functions END */

