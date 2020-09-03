/*
 * Module Name : ADT7475.C
 * Purposes
 *
 * Notes:
 * History:
 *    02/28/2008       -- River.Xu, Porting for adt7475
 *    04/20/2012       -- Kenneth Tzeng, add support for adt7470
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#include "i2c.h"
#include "i2cdrv.h"
#include "adt7475.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "stdio.h"
#include "sys_adpt.h"
#include "uc_mgr.h"

#define ADT7470_REG_PWM_1    0x32
#define ADT7475_REG_PWM_1    0x30
#define MAX_INIT_RETRY       10
#define INVALID_BOARD_ID     0xFFFFFFFFUL
static UI32_T unit_id, board_id=INVALID_BOARD_ID;
#ifdef ES4627MB
/* For ES4651MB-FLF-EC bid = 3, which has two fans.
 * We assume they are fan index 1 and 2,
 * but they are fan index 2 and 3 in production.
 * We'll then get wrong fan status and fail to control
 * them. Here's the workaround.
 */
static UI8_T fan_idx_translation_tbl[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {2,3,4,1};
#endif

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - ADT7475_GetBoardId
 *---------------------------------------------------------------------------
 * PURPOSE:  Get board id
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   board id
 * NOTE:
 *---------------------------------------------------------------------------
 */
static UI32_T ADT7475_GetBoardId(void)
{
    if (board_id==INVALID_BOARD_ID)
    {
        UC_MGR_Sys_Info_T uc_sys_info;
        if (!UC_MGR_GetSysInfo(&uc_sys_info))
        {
            printf("\r\nGet UC System Information Fail. Halt.");

            /* severe problem, while loop here
             */
            while (TRUE);
        }
        board_id=uc_sys_info.board_id;
    }
    return board_id;
}

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - ADT7475_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init fan controller chip
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T ADT7475_Init()
{
    SYS_HWCFG_FanRegInfo_T reg_info;
    UC_MGR_Sys_Info_T         uc_sys_info;
    UI8_T  i, data = 0, i2c_dev_addr;
    BOOL_T result = FALSE;
    static BOOL_T init_flag = FALSE;

    result = init_flag;
    if(init_flag != TRUE)
    {
        /*
         * Get the fan register info according to board_id(if required)
         * which is used to setup the channel.
         */
        if (!UC_MGR_GetSysInfo(&uc_sys_info))
        {
            printf("\r\nGet UC System Information Fail. Halt.");

            /* severe problem, while loop here
             */
            while (TRUE);
        }
        board_id=uc_sys_info.board_id;

        if(SYS_HWCFG_GetFanRegInfo(board_id, 0, &reg_info)==FALSE)
        {
            printf("%s():Failed to get fan register info.\r\n", __FUNCTION__);
            return FALSE;
        }

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
        #ifdef INCLUDE_DIAG
            if(I2C_SetMux(reg_info.info.i2c_with_channel.i2c_mux_index,
                reg_info.info.i2c_with_channel.channel_val)==I2C_ERROR)
        #else
            if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
                reg_info.info.i2c_with_channel.channel_val)==FALSE)
        #endif
            {
                printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
                return FALSE;
            }
            i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
        }
        else
        {
            i2c_dev_addr = reg_info.info.i2c.dev_addr;
        }

        /* On ECS4910-28F, I2C transaction might fail frequently without delay
         * So define SYS_HWCFG_ADT7475_INIT_DELAY for projects that need to do delay
         */
        #if defined(SYS_HWCFG_ADT7475_INIT_DELAY)
        SYSFUN_Sleep(SYS_HWCFG_ADT7475_INIT_DELAY);
        #endif

        for(i = 0; i < SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
        {
        #if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
            if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(i)))==0)
                continue;
        #endif
    #if defined(ASF4626BA)
            data = 0xE0;
        #ifdef INCLUDE_DIAG
            if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x5C + i), 1, &data) != I2C_SUCCESS)
        #else
            if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x5C + i), 1, &data) != TRUE)
        #endif
            {
                I2C_DEBUG_OUT("Initialize fan PWM%d to manual mode fail\n", i);
                goto LABEL_ADT7475_Init;
            }
            data = 0;
        #ifdef INCLUDE_DIAG
            I2C_Transaction(I2C_MASTER_RCV, i2c_dev_addr, 0x5c+i, 1, &data);
        #else
            I2CDRV_GetI2CInfo(i2c_dev_addr, 0x5c+i, 1, &data);
        #endif
    #elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
            /* set PWM Configuration Registers (0x68, 0x69):
             * setup manual mode for fan 1~2(0x68) fan 3~4(0x69)
             * bit6, bit7 should be set to zero.
             */
            data = 0x0;
            if(i==0 || i==2)
            {
            #ifdef INCLUDE_DIAG
                if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x68 + i/2), 1, &data) != I2C_SUCCESS)
            #else
                if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x68 + i/2), 1, &data) != TRUE)
            #endif
                {
                    I2C_DEBUG_OUT("Initialize fan PWM%d PWM%d to manual mode fail\n", i+1, i+2);
                    goto LABEL_ADT7475_Init;
                }
            }

    #elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)
            /* set PWM Configuration Registers (0x5C-0x5E)
             */
            data = 0xE2;
        #ifdef INCLUDE_DIAG
            if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x5C + i), 1, &data) != I2C_SUCCESS)
        #else
            if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x5C + i), 1, &data) != TRUE)
        #endif
            {
                I2C_DEBUG_OUT("Initialize fan PWM%d to manual mode fail\n", i);
                goto LABEL_ADT7475_Init;
            }

            /* set Temp T-range/PWM Frequency Registers (0x5F-0x61)
             */
            #if defined(SYS_HWCFG_ADT7475_PWM_FREQ_INIT_VALUE)
            #define ADT7475_PWM_FREQ_MASK 0x07
            data = 0xC0 | (SYS_HWCFG_ADT7475_PWM_FREQ_INIT_VALUE & ADT7475_PWM_FREQ_MASK);
            #undef ADT7475_PWM_FREQ_MASK
            #else
            data = 0xC7;
            #endif

            #if (SYS_HWCFG_ADT7475_PWM_HIGH_FREQ_OUTPUT == TRUE)
            data |= BIT_3;
            #endif
            /* set Remote 1 T RANGE/PWM1 frequency Register (0x5F) 
             */
        #ifdef INCLUDE_DIAG
            if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x5F + i), 1, &data) != I2C_SUCCESS)
        #else
            if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x5F + i), 1, &data) != TRUE)
        #endif
            {
                I2C_DEBUG_OUT("Initialize fan PWM%d frequency fail\n", i);
                goto LABEL_ADT7475_Init;
            }

            /* set PWM Minimum Duty Cycle Registers (0x64-0x66)
             */
            data = 0x00;
        #ifdef INCLUDE_DIAG
            if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x64 + i), 1, &data) != I2C_SUCCESS)
        #else
            if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x64 + i), 1, &data) != TRUE)
        #endif
            {
                I2C_DEBUG_OUT("Initialize fan PWM%d Min to 0%% fail\n", i);
                goto LABEL_ADT7475_Init;
            }

            /* set Maximim PWM Duty Cycle Registers (0x38-0x3A)
             */
            data = 0xFF;
        #ifdef INCLUDE_DIAG
            if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (0x38 + i), 1, &data) != I2C_SUCCESS)
        #else
            if(I2CDRV_SetI2CInfo(i2c_dev_addr, (0x38 + i), 1, &data) != TRUE)
        #endif
            {
                I2C_DEBUG_OUT("Initialize fan PWM%d Max to 100%% fail\n", i);
                goto LABEL_ADT7475_Init;
            }
    #endif /* End of #if defined(ASF4626BA) */
        }

        result = TRUE;
        init_flag = TRUE;

LABEL_ADT7475_Init:

#ifndef INCLUDE_DIAG
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
#endif
        return result;
    }
    return result;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADT7475_GetSpeedInRPM
 *---------------------------------------------------------------------------
 * PURPOSE: get fan speed
 * INPUT:   index -- fan index.
 * OUTPUT:  speed_p -- fan speed (in RPM)
 * RETURN:  TRUE/FALSE
 * NOTE:    1.ADT7470: read from Fan Tach Reading Registers(0x2A ~ 0x31)
 *          2.ADT7475: currently, no projects support this.
 *---------------------------------------------------------------------------
 */
static BOOL_T ADT7475_GetSpeedInRPM(UI8_T index, UI32_T* speed_p)
{
    SYS_HWCFG_FanRegInfo_T reg_info;
    UI8_T   i, data[2]={0}, percent, i2c_dev_addr;
    BOOL_T  result = FALSE;

    if((index < 1) || (index > SYS_HWCFG_MAX_NBR_OF_FAN_PER_UNIT) || (speed_p == NULL))
        return result;

#ifdef ES4627MB
    if(ADT7475_GetBoardId()== 3)
        index = fan_idx_translation_tbl[index-1];
#endif

    *speed_p = 0;

    if(SYS_HWCFG_GetFanRegInfo(ADT7475_GetBoardId(), 0, &reg_info)==FALSE)
    {
        printf("%s():Failed to get fan register info.\r\n", __FUNCTION__);
        return FALSE;
    }

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
    #ifdef INCLUDE_DIAG
        if(I2C_SetMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==I2C_ERROR)
    #else
        if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==FALSE)
    #endif
        {
            printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

    i = index - 1;
#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
    /*
     * Fan Speed Measurement Registers : Tach 1 ~ 4
     * 0x2A 0x2B 0x2C 0x2D 0x2E 0x2F 0x30 0x31
     * 0x2A Tach 1 Low Byte   (EX 0xFF)
     * 0x2B Tach 1 High Byte  (EX 0x17)
     * Assume a fan with 2 pulses/revolution, fan speed is calculated by
     *     Fan Speed(RPM) = (90000 * 60)/Fan Tach Reading
     * Example:
     *     Fan Tach 1 Reading = 0x17FF = 6143 decimal
     *     Fan Speed = (90000 * 60) / 6143
     *               = 879 RPM
     */
#ifdef INCLUDE_DIAG
    if(I2C_Transaction(I2C_MASTER_RCV, i2c_dev_addr, (0x2a + i*2), 1, data) != I2C_SUCCESS)
#else
    if(I2CDRV_GetI2CInfo(i2c_dev_addr, (0x2a + i*2), 1, data) != TRUE)
#endif
    {
        I2C_DEBUG_OUT("ADT7475_GetSpeed:I2CDRV_GetI2CInfo fail !\n");
        goto LABEL_ADT7475_GetSpeedInRPM;
    }
#ifdef INCLUDE_DIAG
    if(I2C_Transaction(I2C_MASTER_RCV, i2c_dev_addr, (0x2a + i*2 + 1), 1, &data[1]) != I2C_SUCCESS)
#else
    if(I2CDRV_GetI2CInfo(i2c_dev_addr, (0x2a + i*2 + 1), 1, &data[1]) != TRUE)
#endif
    {
        I2C_DEBUG_OUT("ADT7475_GetSpeed:I2CDRV_GetI2CInfo fail !\n");
        goto LABEL_ADT7475_GetSpeedInRPM;
    }

    if((data[1]<<8)+data[0] == 0xffff)
        *speed_p = 0;
    else
        *speed_p = (90000*60)/((data[1]<<8)+data[0]);
#elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)
/* Currently, no projects called.*/
        *speed_p = 0;
#endif
    result = TRUE;

LABEL_ADT7475_GetSpeedInRPM:

#ifndef INCLUDE_DIAG
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
#endif
    /*add sleep for reading fan speed frequently */
    SYSFUN_Sleep(10);

    return result;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADT7475_GetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE: get fan speed
 * INPUT:   index -- fan index.
 * OUTPUT:  speed_p -- fan speed (in duty cycle)
 * RETURN:  TRUE/FALSE
 * NOTE:    1.Read from Current PWM Current Duty Cycle registers
 *            ADT7470: 0x32~0x35
 *            ADT7475: 0x30~0x32
 *---------------------------------------------------------------------------
 */
static BOOL_T ADT7475_GetSpeedInDutyCycle(UI8_T index, UI32_T* speed_p)
{
    SYS_HWCFG_FanRegInfo_T reg_info;
    UI8_T   i, data[2]={0}, percent, i2c_dev_addr, reg_pwm_1;
    BOOL_T  result = FALSE;

#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
    reg_pwm_1 = ADT7470_REG_PWM_1;
#elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)
    reg_pwm_1 = ADT7475_REG_PWM_1;
#endif

    if((index < 1) || (index > SYS_HWCFG_MAX_NBR_OF_FAN_PER_UNIT) || (speed_p == NULL))
        return result;

#ifdef ES4627MB
    if(ADT7475_GetBoardId()== 3)
        index = fan_idx_translation_tbl[index-1];
#endif

    *speed_p = 0;

    if(SYS_HWCFG_GetFanRegInfo(ADT7475_GetBoardId(), 0, &reg_info)==FALSE)
    {
        printf("%s():Failed to get fan register info.\r\n", __FUNCTION__);
        return FALSE;
    }

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
    #ifdef INCLUDE_DIAG
        if(I2C_SetMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==I2C_ERROR)
    #else
        if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==FALSE)
    #endif
        {
            printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

    i = index - 1;

#ifdef INCLUDE_DIAG
    if(I2C_Transaction(I2C_MASTER_RCV, i2c_dev_addr, (reg_pwm_1+i), 1, data) != I2C_SUCCESS)
#else
    if(I2CDRV_GetI2CInfo(i2c_dev_addr, (reg_pwm_1+i), 1, data) != TRUE)
#endif
    {
        I2C_DEBUG_OUT("ADT7475_GetSpeed:I2CDRV_GetI2CInfo fail !\n");
        goto LABEL_ADT7475_GetSpeedInDutyCycle;
    }
    *speed_p = 100;
    if(data[0] != 0xFF)
    {
        *speed_p = 0;
        if(data[0] != 0x00)  *speed_p = (data[0] * 39)/100 + 1;
    }
    result = TRUE;

LABEL_ADT7475_GetSpeedInDutyCycle:

#ifndef INCLUDE_DIAG
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
#endif

    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADT7475_SetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE:  set fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed -- fan speed(0~100).
 * RETURN:   TRUE/FALSE
 * NOTE:    1.Set PWM Current Duty Cycle registers
 *            ADT7470: 0x32~0x35
 *            ADT7475: 0x30~0x32
 *---------------------------------------------------------------------------
 */
static BOOL_T ADT7475_SetSpeed(UI8_T index, UI32_T speed)
{
    SYS_HWCFG_FanRegInfo_T reg_info;
    UI8_T  i, data = 0, i2c_dev_addr, reg_pwm_1;
    BOOL_T result = FALSE;

#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
    reg_pwm_1 = ADT7470_REG_PWM_1;
#elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)
    reg_pwm_1 = ADT7475_REG_PWM_1;
#endif

    if((index < 1) || (index > SYS_HWCFG_MAX_NBR_OF_FAN_PER_UNIT))
    {
        return result;
    }

#ifdef ES4627MB
    if(ADT7475_GetBoardId()== 3)
        index = fan_idx_translation_tbl[index-1];
#endif

    #if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
    if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(index-1)))==0)
        return TRUE;
    #endif

    if(SYS_HWCFG_GetFanRegInfo(ADT7475_GetBoardId(), 0, &reg_info)==FALSE)
    {
        printf("%s():Failed to get fan register info.\r\n", __FUNCTION__);
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
#ifdef INCLUDE_DIAG
        if(I2C_SetMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==I2C_ERROR)
#else
        if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val)==FALSE)
#endif
        {
            printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

#if !defined(ASF4626BA)
    /* Register 0x40 - Configuration Register 1
     *   Bit [0]:Logic 1 enables monitoring and PWM control outputs based on the
     *           limit settings programmed.
     */
#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)
    data = 0x1;
#elif (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)
    data = 0x05;
#endif

#ifdef INCLUDE_DIAG
    if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, 0x40, 1, &data) != I2C_SUCCESS)
#else
    if(I2CDRV_SetI2CInfo(i2c_dev_addr, 0x40, 1, &data) != TRUE)
#endif
    {
        I2C_DEBUG_OUT("ADT7475_SetSpeed:%d I2CDRV_SetI2CInfo fail !\n", __LINE__);
        goto LABEL_ADT7475_SetSpeed;
    }
#endif

    data = 0xFF;
    if(speed < 100)
    {
        data = (speed * 100) / 39;
    }

    i = index - 1;
#ifdef INCLUDE_DIAG
    if(I2C_Transaction(I2C_MASTER_XMIT, i2c_dev_addr, (reg_pwm_1+i), 1, &data) != I2C_SUCCESS)
#else
    if(I2CDRV_SetI2CInfo(i2c_dev_addr, (reg_pwm_1+i), 1, &data) != TRUE)
#endif
    {
        I2C_DEBUG_OUT("ADT7475_SetSpeed: %d I2CDRV_SetI2CInfo %lu fail !\n", __LINE__, i);
        goto LABEL_ADT7475_SetSpeed;
    }

    result = TRUE;

LABEL_ADT7475_SetSpeed:

#ifndef INCLUDE_DIAG
    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
#endif

    return result;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init fan controller chip
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_CHIP_Init()
{
    UI8_T retry_count;

    for(retry_count=0; ADT7475_Init() != TRUE && retry_count < MAX_INIT_RETRY; retry_count++)
    {
        SYSFUN_Sleep(10);
    }
    if(retry_count == MAX_INIT_RETRY)
    {
        printf("%s fail\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInRPM
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInRPM(UI8_T index, UI32_T* speed_p)
{
#if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
    if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(index-1)))==0)
        return TRUE;
#endif
    if(ADT7475_GetSpeedInRPM(index, speed_p) != TRUE)
    {
        printf("%s:index=%lu fail\n", __FUNCTION__, index);
        return FALSE;
    }
    return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInDutyCycle(UI8_T index, UI32_T* speed_p)
{
#if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
    if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(index-1)))==0)
        return TRUE;
#endif
    if(ADT7475_GetSpeedInDutyCycle(index, speed_p) != TRUE)
    {
        printf("%s:index=%lu fail\n", __FUNCTION__, index);
        return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_SetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE:  set fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed -- fan speed.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_SetSpeed(UI8_T index, UI32_T speed)
{
#if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
    if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(index-1)))==0)
    {
        return TRUE;
    }
#endif

    if(ADT7475_SetSpeed(index, speed) != TRUE)
    {
        printf("%s(%lu, %lu) fail\n", __FUNCTION__, index, speed);
        return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan status
 * INPUT:    None
 * OUTPUT:   status_p -- fan status
 *           bit 0~7 represent the fan fail status of fan 1~8
 *           if fan fails, set the corresponding bit to 1.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetStatus(UI8_T* status_p)
{
    UI32_T speed;
    UI8_T i;

    *status_p = 0;
    for(i= 1; i<= SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
    {
        speed = 0;
        if(ADT7475_GetSpeedInRPM(i, &speed) != TRUE)
        {
            printf("%s:index=%lu fail\n", __FUNCTION__, i);
            return FALSE;
        }
        if(speed == 0)
            *status_p = *status_p | (1<<(i-1));
    }
    return TRUE;
}

