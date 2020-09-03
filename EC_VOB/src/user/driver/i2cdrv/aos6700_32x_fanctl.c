/* MODULE NAME:  aos6700_32x_fanctl.c
 * PURPOSE:
 *   This module implements the device driver APIs for functionalities reatled
 *   to fan control, which is provided by the CPLD on aos6700_32x.
 *
 * NOTES:
 *
 * HISTORY
 *    12/16/2014 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Technology Coporation, 2014
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
#define MAX_NBR_OF_FAN_SPEED_DETECT 10

/* max number of fan speed control supported by this ASIC
 */
#define MAX_NBR_OF_FAN_SPEED_CONTROL 1

/* FAN Speed Detect
 */
/* the fan speed count for the front fans -- START */
#define AOS6700_32X_REG_FAN_COUNT_1 0x0
#define AOS6700_32X_REG_FAN_COUNT_2 0x2
#define AOS6700_32X_REG_FAN_COUNT_3 0x4
#define AOS6700_32X_REG_FAN_COUNT_4 0x6
#define AOS6700_32X_REG_FAN_COUNT_5 0x8
/* the fan speed count for the front fans -- END   */

/* the fan speed count for the rear fans -- START */
#define AOS6700_32X_REG_FANR_COUNT_1 0x1
#define AOS6700_32X_REG_FANR_COUNT_2 0x3
#define AOS6700_32X_REG_FANR_COUNT_3 0x5
#define AOS6700_32X_REG_FANR_COUNT_4 0x7
#define AOS6700_32X_REG_FANR_COUNT_5 0x9
/* the fan speed count for the rear fans -- END   */

/* RPM = Counter Clock * 60(sec->min) / 4 (two-pole fan) / Fan sensor value
 * Counter clock = 74.17KHz
 * RPM = 74170 * 60 / 4 / Fan sensor value = 1112550 / Fan sensor value
 */
#define AOS6700_32X_FANCTL_FAN_COUNT_FACTOR 1112550

/* Fan Speed Control */
#define AOS6700_32X_REG_FAN_SPEED_CTL 0x20

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* fan controller functions
 */
static BOOL_T AOS6700_32X_FANCTL_FanChipInit(UI32_T fan_idx);
static BOOL_T AOS6700_32X_FANCTL_FanChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed);
static BOOL_T AOS6700_32X_FANCTL_FanChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p);
static BOOL_T AOS6700_32X_FANCTL_FanChipSetSpeed(UI8_T fan_idx, UI32_T speed);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for fan controller functions
 */
SYS_HWCFG_FanControllerOps_T fan_controller_ops_aos6700_32x =
{
    AOS6700_32X_FANCTL_FanChipInit,
    AOS6700_32X_FANCTL_FanChipGetSpeedInRpm,
    AOS6700_32X_FANCTL_FanChipGetSpeedInDutyCycle,
    AOS6700_32X_FANCTL_FanChipSetSpeed,
};

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* fan controller related functions START */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - AOS6700_32X_FANCTL_FanChipInit
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to do fan init for the given fan index
 * INPUT:   fan_idx        -  system-wised fan index (start from 1)
 * OUTPUT:
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */
static BOOL_T AOS6700_32X_FANCTL_FanChipInit(UI32_T fan_idx)
{
    /* do nothing
     */
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - AOS6700_32X_FANCTL_FanChipGetSpeedInRpm
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in RPM for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in RPM.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE: The mapping between fan_idx and the CPLD1 registers
 *
 *       Fan Index       CPLD1 register name/address
 *       -------------------------------------------
 *           1           FAN1  Speed (0x00)
 *           2           FANR1 Speed (0x01)
 *           3           FAN2  Speed (0x02)
 *           4           FANR2 Speed (0x03)
 *           5           FAN3  Speed (0x04)
 *           6           FANR3 Speed (0x05)
 *           7           FAN4  Speed (0x06)
 *           8           FANR4 Speed (0x07)
 *           9           FAN5  Speed (0x08)
 *          10           FANR5 Speed (0x09)
 *
 */   
static BOOL_T AOS6700_32X_FANCTL_FanChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    UI32_T max_detected_fan_speed;
    UI8_T  fan_count;
    BOOL_T ret_val = TRUE, rc;
    UI8_T fan_count_regs[MAX_NBR_OF_FAN_SPEED_DETECT] = 
        {
            AOS6700_32X_REG_FAN_COUNT_1,
            AOS6700_32X_REG_FANR_COUNT_1,
            AOS6700_32X_REG_FAN_COUNT_2,
            AOS6700_32X_REG_FANR_COUNT_2,
            AOS6700_32X_REG_FAN_COUNT_3,
            AOS6700_32X_REG_FANR_COUNT_3,
            AOS6700_32X_REG_FAN_COUNT_4,
            AOS6700_32X_REG_FANR_COUNT_4,
            AOS6700_32X_REG_FAN_COUNT_5,
            AOS6700_32X_REG_FANR_COUNT_5,
        };
    int i;

    if ((fan_idx == 0) || (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT))
    {
        return FALSE;
    }

    if (speed==NULL)
    {
        return FALSE;
    }

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
    {
        return FALSE;
    }

    if (fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx > MAX_NBR_OF_FAN_SPEED_DETECT)
    {
        printf("%s(%d): Invalid fan controller internal fan speed detect index %lu(fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx, fan_idx);
        return FALSE;
    }

    /* Read fan count register
     */
    rc = I2CDRV_TwsiDataReadWithBusIdx(fan_ctl_info.reg_info.info.i2c.bus_idx,
        fan_ctl_info.reg_info.info.i2c.dev_addr,
        fan_ctl_info.reg_info.info.i2c.op_type,
        TRUE,
        fan_count_regs[fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx],
        FALSE,
        1, &fan_count);

    if (rc == FALSE)
    {
        printf("%s(%d): Failed to read fan speed count register(i2c bus=%hu, i2c adr=0x%02X, offset=0x%02X, internal fan speed detect index %lu, fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.reg_info.info.i2c.bus_idx,
            fan_ctl_info.reg_info.info.i2c.dev_addr,
            fan_count_regs[fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx],
            fan_ctl_info.fan_ctl_internal_fan_speed_detect_idx,
            fan_idx);
        ret_val=FALSE;
    }
    else
    {
        if (fan_count==255)
        {
            *speed=0;
        }
        else
        {
            if (fan_count==0)
            {
                /* if fan_count is 0, it will result to divided-by-zero exception
                 */
                fan_count=1;
            }

            *speed=AOS6700_32X_FANCTL_FAN_COUNT_FACTOR/fan_count;
        }

    }
    return ret_val;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - AOS6700_32X_FANCTL_FanChipGetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  duty_cycle_p   - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T AOS6700_32X_FANCTL_FanChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    BOOL_T ret_val=TRUE, rc;
    UI8_T reg_val;

    if (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
        return FALSE;

    if (duty_cycle_p == NULL)
        return FALSE;

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
        return FALSE;

    if (fan_ctl_info.fan_ctl_internal_fan_speed_control_idx > MAX_NBR_OF_FAN_SPEED_CONTROL)
    {
        printf("%s(%d): Invalid fan controller internal fan speed control index %lu(fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx, fan_idx);
        return FALSE;
    }

    /* Read fan speed control registr
     */
    rc = I2CDRV_TwsiDataReadWithBusIdx(fan_ctl_info.reg_info.info.i2c.bus_idx,
        fan_ctl_info.reg_info.info.i2c.dev_addr,
        fan_ctl_info.reg_info.info.i2c.op_type,
        TRUE,
        AOS6700_32X_REG_FAN_SPEED_CTL,
        FALSE,
        1, &reg_val);

    if (rc == FALSE)
    {
        printf("%s(%d): Failed to read fan speed control register(i2c bus=%hu, i2c adr=0x%02X, offset=0x%02X, internal fan speed control index %lu, fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.reg_info.info.i2c.bus_idx,
            fan_ctl_info.reg_info.info.i2c.dev_addr,
            AOS6700_32X_REG_FAN_SPEED_CTL,
            fan_ctl_info.fan_ctl_internal_fan_speed_control_idx,
            fan_idx);
        ret_val=FALSE;
    }
    else
    {

        /* 6 levels available to set the duty cycle
         * 100%(0x7) 75%(0x6) 62.5%(0x5) 50%(0x4) 37.5%(0x3) 0%(0x0)
         */
        switch (reg_val)
        {
            case 0:
                *duty_cycle_p = 0;
                break;
            case 3:
                *duty_cycle_p = 38;
                break;
            case 4:
                *duty_cycle_p = 50;
                break;
            case 5:
                *duty_cycle_p = 63;
                break;
            case 6:
                *duty_cycle_p = 75;
                break;
            case 7:
                *duty_cycle_p = 100;
                break;
            default:
                printf("%s(%d): Invalid reg value(%d)\r\n", __FUNCTION__, __LINE__, (int)reg_val);
                ret_val=FALSE;
                break;
        }
    }

    return ret_val;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - AOS6700_32X_FANCTL_FanChipSetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to set fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T AOS6700_32X_FANCTL_FanChipSetSpeed(UI8_T fan_idx, UI32_T speed)
{
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    BOOL_T ret_val=TRUE, rc;
    UI8_T data;

    if (fan_idx > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
        return FALSE;

    if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info) == FALSE)
        return FALSE;

    if (fan_ctl_info.fan_ctl_internal_fan_speed_control_idx > MAX_NBR_OF_FAN_SPEED_CONTROL)
    {
        printf("%s(%d): Invalid fan controller internal fan speed control index %lu(fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.fan_ctl_internal_fan_speed_control_idx,
            fan_idx);
        return FALSE;
    }

    /* speed is duty cycle, which should be in range 0 to 100
     * forced the value to be 100 if it is greater than 100
     */
    if(speed>100)
        speed=100;

    /* 6 levels available to set the duty cycle
     * 100%(0x7) 75%(0x6) 62.5%(0x5) 50%(0x4) 37.5%(0x3) 0%(0x0)
     *
     * convert duty cycle(range:0-100) to register value(0,3,4,5,7)
     */
    if (speed==100)
    {
        data=7;
    }
    else if (speed>=75)
    {
        data=6;
    }
    else if (speed>=63)
    {
        data=5;
    }
    else if (speed>=50)
    {
        data=4;
    }
    else if (speed>=38)
    {
        data=3;
    }
    else
    {
        data=0;
    }

    /* Write fan speed control registr */
    rc = I2CDRV_TwsiDataWriteWithBusIdx(fan_ctl_info.reg_info.info.i2c.bus_idx,
            fan_ctl_info.reg_info.info.i2c.dev_addr,
            fan_ctl_info.reg_info.info.i2c.op_type,
            TRUE,
            AOS6700_32X_REG_FAN_SPEED_CTL,
            FALSE,
            &data, 1);

    if (rc == FALSE)
    {
        printf("%s(%d): Failed to write fan speed control register(i2c bus=%hu, i2c adr=0x%02X, offset=0x%02X, internal fan speed control index %lu, fan_idx=%hu)\r\n",
            __FUNCTION__, __LINE__, fan_ctl_info.reg_info.info.i2c.bus_idx,
            fan_ctl_info.reg_info.info.i2c.dev_addr,
            AOS6700_32X_REG_FAN_SPEED_CTL,
            fan_ctl_info.fan_ctl_internal_fan_speed_control_idx,
            fan_idx);
        ret_val=FALSE;
    }

    return ret_val;

}
/* fan controller related functions END */

