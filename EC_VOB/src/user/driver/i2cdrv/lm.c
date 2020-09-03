/* MODULE NAME:  lm.c
 * PURPOSE:
 *     Device driver for LM series thermal chips.
 *
 * NOTES:
 *     Current support LM chips is listed below:
 *         LM75
 *         LM77
 *
 * HISTORY
 *    06/13/2011 - Charlie Chen, Created
 *
 * Copyright(C)      Edge-Core Networks Corporation, 2011
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "i2cdrv.h"
#include "sysdrv.h"
#include "dev_swdrv.h"
#include "stktplg_om.h"
#include "sysfun.h"
#include "uc_mgr.h"
#if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC==TRUE)
#include "stktplg_board.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static I8_T LM_TranslateRawDataToTemperature_LM75(UI8_T raw_data[]);
static I8_T LM_TranslateRawDataToTemperature_LM77(UI8_T raw_data[]);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - LM_TranslateRawDataToTemperature_LM75
 *---------------------------------------------------------------------------
 * PURPOSE:  Translate the given raw data got from thermal sensor LM75 into
 *           temperature value(Celsius)
 * INPUT:    raw_data: The raw data read from the thermal sensor.
 * OUTPUT:   None
 * RETURN:   The translated temperature value(Celsius)
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static I8_T LM_TranslateRawDataToTemperature_LM75(UI8_T raw_data[])
{
    I16_T temperature_local;

    /* Most significant byte is raw_data[0]
     * raw_data[0] contains D15-D8, data[1] contains D7-D0
     * Bit D15-D7 : Temperature data. One LSB = 0.5 Celsius degree.
     *              Two's complement format.
     * Bit D6-D0  : Undefined
     */
    temperature_local = (raw_data[0]<<1 | ( (raw_data[1] & 0x80) >>7 ));
    if(temperature_local & 0x100) /* is minus number ? */
    {
        /* convert to negative value using two's complement
         */
        temperature_local = (~temperature_local & 0xFF) + 1;
        temperature_local = temperature_local * -1;
    }

    return (I8_T)(temperature_local/2);
    
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - LM_TranslateRawDataToTemperature_LM77
 *---------------------------------------------------------------------------
 * PURPOSE:  Translate the given raw data got from thermal sensor LM77 into
 *           temperature value(Celsius)
 * INPUT:    raw_data: The raw data read from the thermal sensor.
 * OUTPUT:   None
 * RETURN:   The translated temperature value(Celsius)
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static I8_T LM_TranslateRawDataToTemperature_LM77(UI8_T raw_data[])
{
    I16_T temperature_local;

    /* Most significant byte is data[0]
     * data[0] contains D15-D8, data[1] contains D7-D0
     * Bit D15-D12: Sign (Temperature data)
     * Bit D11-D3 : Temperature data. One LSB = 0.5 Celsius degree.
     *              Two's complement format.
     * Bit D2-D0  : Status bits
     */
    temperature_local = (raw_data[0] & 0x0f) << 5;
    temperature_local = temperature_local | ( (raw_data[1] & 0xf8) >>3 );
    if(temperature_local & 0x100) /* is minus number ? */
    {
        /* convert to negative value using two's complement
         */
        temperature_local = (~temperature_local & 0xFF) + 1;
        temperature_local = temperature_local * -1;
    }
    return (I8_T)(temperature_local/2);
}

#if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC==FALSE)
/* old design, shall be obsoleted */
static I8_T LM_TranslateRawDataToTemperature(UI8_T raw_data[]);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - __SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization on chip.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - The chip is initialized sucessfully.
 * NOTE:     Use compiler control to define seperate __SYSDRV_THERMAL_CHIP_Init
 *           if required.
 *---------------------------------------------------------------------------
 */
BOOL_T __SYSDRV_THERMAL_CHIP_Init(void)
{
#if (SYS_HWCFG_THERMAL_DO_CRITICAL_TEMP_INITIAL == TRUE)
#if (SYS_HWCFG_THERMAL_TYPE == SYS_HWCFG_THERMAL_LM77)
    UI32_T board_id;
    SYS_HWCFG_ThermalRegInfo_T reg_info;
    UC_MGR_Sys_Info_T         uc_sys_info;
    I16_T  critical_temp;
    BOOL_T result;
    UI8_T data[2] = {0};
    UI8_T i2c_dev_addr, index;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("\r\nGet UC System Information Fail.");
        /* severe problem, while loop here
        */
        while (TRUE);
    }
    board_id=uc_sys_info.board_id;

    for(index=1; index <= SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; index++)
    {
        if(SYS_HWCFG_GetThermalRegInfo(board_id, index-1, &reg_info)==FALSE)
        {
            printf("%s():Failed to get thermal register info.\r\n",
                __FUNCTION__);
            return FALSE;
        }

        /* sanity check
         */
        if((reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C)&&
           (reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL))
        {
            printf("%s():Invalid access method: %u\r\n", __FUNCTION__, reg_info.access_method);
            return FALSE;
        }

        /* set and lock i2c channel if required
         */
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        {
            if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
                reg_info.info.i2c_with_channel.channel_val)==FALSE)
            {
                /* I2C operation might fail due to signal sometimes
                 * Use SYSFUN_Debug_Printf to hide the debug message
                 */
                SYSFUN_Debug_Printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
                return FALSE;
            }
            i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
        }
        else
            i2c_dev_addr = reg_info.info.i2c.dev_addr;

        if (SYS_HWCFG_GetThermalCriticalTemperature(board_id, index-1, &critical_temp) == TRUE)
        {
            /* set thermal device
             * T_CRIT_A    Register:offset 0x3
             * Most significant byte is data[0]
             * data[0] contains D15-D8, data[1] contains D7-D0
             * Bit D15-D12: Sign (Temperature data)
             * Bit D11-D3 : Temperature data. One LSB = 0.5 Celsius degree.
             *              Two's complement format.
             * Bit D2-D0  : Don't care
             */
            if (critical_temp<0)
            {
                data[0] |= 0xF0;
            }
            data[0] |= 0x0F & (((critical_temp*2) & 0x1E0) >> 5);
            data[1] |= 0xF8 & (((critical_temp*2) & 0x01F) << 3);
            result = I2CDRV_SetI2CInfo(i2c_dev_addr, 0x3, 2, data);
            if( result != TRUE)
            {
                /* I2C operation might fail due to signal sometimes
                 * Use SYSFUN_Debug_Printf to hide the debug message
                 */
                SYSFUN_Debug_Printf("%s():I2CDRV_GetI2CInfo fail(%x,%u)\r\n", __FUNCTION__,
                    reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr,
                    reg_info.info.i2c_with_channel.i2c_reg_info.offset);
                if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
                    I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
                return FALSE;
            }
        }

        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
    }
#endif /* #if (SYS_HWCFG_THERMAL_TYPE == SYS_HWCFG_THERMAL_LM77) */
#endif /* #ifdef SYS_HWCFG_THERMAL_DO_CRITICAL_TEMP_INITIAL */
    return TRUE;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization on chip.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - The chip is initialized sucessfully.
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_CHIP_Init(void)
{
    return __SYSDRV_THERMAL_CHIP_Init();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetTemperature
 *---------------------------------------------------------------------------
 * PURPOSE:  get cuurent temperature from Thermal
 * INPUT:    index: Thermal index (starts by 1)
 * OUTPUT:   temperature:Thermal sensor temperature (Celsius)
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */

BOOL_T SYSDRV_THERMAL_GetTemperature(UI8_T index, I8_T* temperature)
{
    UI32_T board_id;
    SYS_HWCFG_ThermalRegInfo_T reg_info;
#if !defined(INCLUDE_DIAG)
    UC_MGR_Sys_Info_T         uc_sys_info;
#endif
    BOOL_T result;
    UI8_T data[2] = {0};
    UI8_T i2c_dev_addr;

    if( (index < 1) || (index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        printf("%s():Invalid index(%hu)\r\n", __FUNCTION__, index);
        return FALSE;
    }

    if(temperature == NULL)
    {
        printf("%s():Invalid temperature\r\n", __FUNCTION__);
        return FALSE;
    }

#if !defined(INCLUDE_DIAG)
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("\r\n %s(): Get UC System Information Fail.", __FUNCTION__);
        /* severe problem, while loop here
        */
        while (TRUE);
    }
    board_id=uc_sys_info.board_id;
#else
    board_id = BOARD_GetBoardId();
#endif

    if(SYS_HWCFG_GetThermalRegInfo(board_id, index-1, &reg_info)==FALSE)
    {
        printf("%s():Failed to get thermal register info.\r\n",
            __FUNCTION__);
        return FALSE;
    }

    /* sanity check
     */
    if((reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C)&&
       (reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL))
    {
        printf("%s():Invalid access method: %u\r\n", __FUNCTION__, reg_info.access_method);
        return FALSE;
    }

    /* set and lock i2c channel if required
     */
    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    {
        if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==FALSE)
        {
            printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

    /* read from thermal device
     */
    result = I2CDRV_GetI2CInfo(i2c_dev_addr, 0, 2, data);

    if( result != TRUE)
    {
        /* EPR ID: ASF4628BBS5-FLF-EC-00289
         * Headline: VLAN:Delete vlan 2-4094 via CLI & Web will display error message.
         * Root cause: when deleting many vlans at a time, it would takes a lot of 
         * time(2-4094 vlan takes 10~12 minutes).
         * The I2C_Read may sometime return errno with EINTR(4) EIO(5).
         * So change to call SYSFUN_Debug_Printf to show
         * the error message
         */
        SYSFUN_Debug_Printf("%s():I2CDRV_GetI2CInfo fail(%x,%u)\r\n", __FUNCTION__,
            reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr,
            reg_info.info.i2c_with_channel.i2c_reg_info.offset);
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
        return FALSE;
    }
    else
    {
        *temperature=LM_TranslateRawDataToTemperature(data);
    }

    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_SetThreshold
 *---------------------------------------------------------------------------
 * PURPOSE:  set Thermal trap value
 * INPUT:    index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_SetThreshold(UI8_T index, I8_T  temperature) 
{
    /* not implement yet
     */
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
static I8_T LM_TranslateRawDataToTemperature(UI8_T raw_data[])
{

#if (SYS_HWCFG_THERMAL_TYPE == SYS_HWCFG_THERMAL_LM75)
    return LM_TranslateRawDataToTemperature_LM75(raw_data);
#elif (SYS_HWCFG_THERMAL_TYPE == SYS_HWCFG_THERMAL_LM77)
    return LM_TranslateRawDataToTemperature_LM77(raw_data);
#else
#error "Unsupported SYS_HWCFG_THERMAL_TYPE"
#endif

}

#else /* #if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == FALSE) */
/* max number of thermal sensors supported by this ASIC
 */
#define MAX_NBR_OF_THERMAL_SENSOR 1

static BOOL_T LM_ThermalChipInit(UI32_T thermal_idx);
static BOOL_T LM_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T LM_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);
static BOOL_T LM_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p);
static BOOL_T LM_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature);

/* exported variable for thermal functions
 */
SYS_HWCFG_ThermalOps_T thermal_ops_lm75 =
{
    LM_ThermalChipInit,
    LM_ThermalChipGetTemperature
};

SYS_HWCFG_ThermalOps_T thermal_ops_lm77 =
{
    LM_ThermalChipInit,
    LM_ThermalChipGetTemperature
};

/* FUNCTION NAME: LM_ThermalChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do thermal init for the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T LM_ThermalChipInit(UI32_T thermal_idx)
{
    /* do nothing */
    return TRUE;
}

/* FUNCTION NAME: LM_PreAction
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
static BOOL_T LM_PreAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = &(thermal_ctl_info_p->reg_info.info.i2c_with_channel);

            if (I2CDRV_SetAndLockMux(reg_info_p->i2c_mux_index, reg_info_p->channel_val)==FALSE)
            {
                SYSFUN_Debug_Printf("%s():I2CDRV_SetAndLockMux fail(mux_idx=%02X,channel_val=0x%02X)\r\n", __FUNCTION__,
                    reg_info_p->i2c_mux_index,
                    reg_info_p->channel_val);
                return FALSE;
            }
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

/* FUNCTION NAME: LM_PostAction
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
static BOOL_T LM_PostAction(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p)
{
    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
        {
            SYS_HWCFG_i2cRegAndChannelInfo_T *reg_info_p = &(thermal_ctl_info_p->reg_info.info.i2c_with_channel);

            if (I2CDRV_UnLockMux(reg_info_p->i2c_mux_index)==FALSE)
            {
                SYSFUN_Debug_Printf("%s():I2CDRV_UnLockMux fail(mux_idx=%02X)\r\n", __FUNCTION__,
                    reg_info_p->i2c_mux_index);
                return FALSE;
            }
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

/* FUNCTION NAME: LM_DoRegRead
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
static BOOL_T LM_DoRegRead(SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p, UI8_T reg_addr, UI8_T data_len, UI8_T *data_p)
{
    SYS_HWCFG_i2cRegInfo_T *reg_info_p=NULL;
    UI8_T i;

    switch (thermal_ctl_info_p->reg_info.access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
            reg_info_p=&(thermal_ctl_info_p->reg_info.info.i2c_with_channel.i2c_reg_info);
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            reg_info_p=&(thermal_ctl_info_p->reg_info.info.i2c);
            break;
        default:
            printf("%s(%d): Not support access method(%hu)\r\n", __FUNCTION__,
                __LINE__, thermal_ctl_info_p->reg_info.access_method);
            return FALSE;
            break;
    }

    for (i=0; i<data_len; i++)
    {
        if (I2CDRV_TwsiDataReadWithBusIdx(reg_info_p->bus_idx, 
            reg_info_p->dev_addr, I2C_7BIT_ACCESS_MODE,
            TRUE, reg_addr + i,
            FALSE, 1, data_p+i) == FALSE)
        {
            /* EPR ID: ASF4628BBS5-FLF-EC-00289
             * Headline: VLAN:Delete vlan 2-4094 via CLI & Web will display error message.
             * Root cause: when deleting many vlans at a time, it would takes a lot of 
             * time(2-4094 vlan takes 10~12 minutes).
             * The I2C_Read may sometime return errno with EINTR(4) EIO(5).
             * So change to call SYSFUN_Debug_Printf to show
             * the error message
             */
            SYSFUN_Debug_Printf("%s():I2CDRV_TwsiDataReadWithBusIdx fail(bus_idx=%d, dev_adr=0x%02X,reg=0x%02X)\r\n", __FUNCTION__,
                (int)(reg_info_p->bus_idx),
                reg_info_p->dev_addr,
                reg_addr + i);
                return FALSE;
        }
    }

    return TRUE;
}

/* FUNCTION NAME: LM_ThermalChipGetTemperature
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the temperature in Celsius degree from the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : temperature    - the temperature in Celsius degree from the given thermal index
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T LM_ThermalChipGetTemperature(UI8_T thermal_idx, I8_T* temperature)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    UI8_T reg_val_ar[2];
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

    if (LM_PreAction(&thermal_ctl_info)==FALSE)
    {
        SYSFUN_Debug_Printf("%s(%d): Failed to do pre-action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (LM_DoRegRead(&thermal_ctl_info, 0, 2, &(reg_val_ar[0]))==TRUE)
    {
        switch (thermal_ctl_info.thermal_type)
        {
            case SYS_HWCFG_THERMAL_LM75:
                *temperature = LM_TranslateRawDataToTemperature_LM75(reg_val_ar);
                break;
            case SYS_HWCFG_THERMAL_LM77:
                *temperature = LM_TranslateRawDataToTemperature_LM77(reg_val_ar);
                break;
            default:
                printf("%s(%d): Thermal type(%lu) is not supported.\r\n", __FUNCTION__, __LINE__, thermal_ctl_info.thermal_type);
                ret_val=FALSE;
                break;
        }
    }
    else
    {
        ret_val=FALSE;
        SYSFUN_Debug_Printf("%s(%d): Failed to read thermal reg.(thermal_idx=%hu,internal_thermal_idx=%lu)\r\n", __FUNCTION__,
            __LINE__, thermal_idx, thermal_ctl_info.thermal_ctl_internal_thermal_idx);
    }

    /* Call LM_PostAction() to do required clean-up actions
     * after accessing NE1617A
     */
    if (LM_PostAction(&thermal_ctl_info)==FALSE)
    {
        SYSFUN_Debug_Printf("%s(%d): Failed to do post action.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return ret_val;
}

#endif /* end of #if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == FALSE) */

