/*
 * $Id: m41t11.c,v 1.9 1999/09/28 21:47:10 csm Exp $
 *
 * XGS M41-T11 TOD/I2C Driver
 *
 * Created : S.K.Yang 2002.12.4
 * Porting : River.Xu 2008. 2.28
 *
 * --------------------------------------------------
 *
 * RTC : I2C Switch 1, Channel 1
 *
 */

#include "i2c.h"
#include "i2cdrv.h"
#include "m41t11.h"
#include "uc_mgr.h"
#include "sysfun.h"

/* M41T11 Register list as below:
 * -----+---------------------------------------+------------------------+
 * Addr |                  Data                 | Function/Range         |
 * -----+----+----+----+----+----+----+----+----+   BCD Format           |
 *      | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |                        |
 * -----+----+----+----+----+----+----+----+----+--------------+---------+
 *   0  | ST |  10 Seconds  |      Seconds      | Seconds      |  00-59  |
 *   1  | X  |  10 Minutes  |      Minutes      | Minutes      |  00-59  |
 *   2  | CEB| CB | 10 Hours|      Hours        | Century/Hours|0-1/00-23|
 *   3  | X  | X  | X  | X  | X  |  Day         | Day          |  01-07  |
 *   4  | X  | X  |  10 Date|      Date         | Date         |  01-31  |
 *   5  | X  | X  | X  |10 M|      Month        | Month        |  01-12  |
 *   6  |   10 Years        |      Years        | Year         |  00-99  |
 *   7  | OUT| FT | S  |    Calibration         | Control      |         |
 * -----+----+----+----+----+----+----+----+----+--------------+---------+
 * project that support M41T11:
 * ES4627MB
 * ECS4910_28F
 */ 
static int to_bcd(int value)
{
    return value / 10 * 16 + value % 10;
}

static int from_bcd(int value)
{
    return value / 16 * 10 + value % 16;
}

static int day_of_week(int y, int m, int d)	/* 0-6 ==> Sun-Sat */
{       
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4}; 
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}       


/*
 * Note: the TOD should store the current GMT
 */

static BOOL_T  M41T11_Set(int year,    /* 2001-2099 */
                       int month,   /* 01-12 */
                       int day,     /* 01-31 */
                       int hour,    /* 00-23 */
                       int minute,  /* 00-59 */
                       int second)  /* 00-59 */
{
    UI32_T board_id;
    SYS_HWCFG_RTCRegInfo_T reg_info;
    UC_MGR_Sys_Info_T         uc_sys_info;
    BOOL_T result;
    UI8_T i2c_dev_addr;
    UI8_T buff[7];
    UI8_T datanum=7;
 
    if (year < 2001 || year > 2099)
        return FALSE;

    year = year - 2001 + 1;

    buff[YEAR] =(unsigned char) to_bcd(year);
    buff[MONTH] = (unsigned char)to_bcd(month);
    buff[DAY] = (unsigned char)to_bcd(day);
    buff[DAY_OF_WEEK] =(unsigned char) day_of_week(year, month, day) + 1;
    buff[HOUR] = (unsigned char)to_bcd(hour);
    buff[MINUTE] = (unsigned char)to_bcd(minute);
    buff[SECOND] = (unsigned char)to_bcd(second);

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {   
        printf("\r\n %s(): Get UC System Information Fail.", __FUNCTION__);
        /* severe problem, while loop here
        */
        while (TRUE);
    }
    board_id=uc_sys_info.board_id;

    if(SYS_HWCFG_GetRTCRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s():Failed to get RTC register info.\r\n", __FUNCTION__);
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

    /* write RTC device
     */
    result = I2CDRV_SetI2CInfo(i2c_dev_addr, M41T11_ADDR_DATAADDR, datanum, buff);

    if( result != TRUE)
    {
        SYSFUN_Debug_Printf("%s():I2CDRV_SetI2CInfo fail(%x,%u)\r\n", __FUNCTION__,
            reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr,
            reg_info.info.i2c_with_channel.i2c_reg_info.offset);
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);
        return FALSE;
    }

    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);

    return TRUE;    
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - M41T11_Get
 *---------------------------------------------------------------------------
 * PURPOSE:  GET RTC Time
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is NULL.
 * OUTPUT: year_p  : year value
 *         month_p : month value
 *         day_p   : day value
 *         hour_p  : hour value
 *         minute_p: minute value
 *         second_p: second value
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T  M41T11_Get(I2CDRV_TwsiDataRead_Func_T read_fn_p,
                          I2CDRV_TwsiDataWrite_Func_T write_fn_p,
                          int *year_p,      /* 2001-2099 */
                          int *month_p,     /* 01-12 */
                          int *day_p,       /* 01-31 */
                          int *hour_p,      /* 00-23 */
                          int *minute_p,    /* 00-59 */
                          int *second_p)    /* 00-59 */
{
    UI32_T board_id;
    SYS_HWCFG_RTCRegInfo_T reg_info;
    UC_MGR_Sys_Info_T         uc_sys_info;
    BOOL_T result;
    UI8_T i2c_dev_addr;
    UI32_T y;
    UI8_T buff[7];
    UI8_T datanum=7;

    if((year_p ==NULL) ||(month_p ==NULL) ||(day_p ==NULL) ||(hour_p ==NULL) ||
       (minute_p ==NULL) ||(second_p ==NULL) )
    {
        printf("Set time fail ! \n");
        return FALSE;
    }

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {   
        printf("\r\n %s(): Get UC System Information Fail.", __FUNCTION__);
        /* severe problem, while loop here
        */
        while (TRUE);
    }
    board_id=uc_sys_info.board_id;

    if(read_fn_p == NULL)
    {
        read_fn_p = &I2CDRV_TwsiDataRead;
    }

    if(write_fn_p == NULL)
    {
        write_fn_p = &I2CDRV_TwsiDataWrite;
    }

    if(SYS_HWCFG_GetRTCRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s():Failed to get RTC register info.\r\n", __FUNCTION__);
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
        if(I2CDRV_SetAndLockMuxWithFunPtr(reg_info.info.i2c_with_channel.i2c_mux_index,
            reg_info.info.i2c_with_channel.channel_val, write_fn_p)==FALSE)
        {
            printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

    /* read from RTC device
     */
    result = read_fn_p(i2c_dev_addr, I2C_7BIT_ACCESS_MODE, TRUE,
                       M41T11_ADDR_DATAADDR, FALSE, datanum, buff);

    if( result != TRUE)
    {
        SYSFUN_Debug_Printf("%s():I2CDRV_SetI2CInfo fail(%x,%u)\r\n", __FUNCTION__,
            reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr,
            reg_info.info.i2c_with_channel.i2c_reg_info.offset);
        if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
            I2CDRV_UnLockMuxWithFunPtr(reg_info.info.i2c_with_channel.i2c_mux_index, write_fn_p);
        return FALSE;
    }

    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        I2CDRV_UnLockMuxWithFunPtr(reg_info.info.i2c_with_channel.i2c_mux_index, write_fn_p);

    y = from_bcd( (int)buff[YEAR] );
    
    *year_p =2000 + y ;
    *month_p = from_bcd((int) buff[MONTH] & 0x1f);
    *day_p = from_bcd((int) buff[DAY] & 0x3f);
    *hour_p = from_bcd((int)buff[HOUR] & 0x3f);
    *minute_p = from_bcd( (int)buff[MINUTE] & 0x7f);
    *second_p = from_bcd( (int)buff[SECOND] & 0x7f);

    #if 0
    /* For debug*/
    printf("\r\nmonth/day hour:minute:second year %s-%d\r\n", __FUNCTION__, __LINE__);
    printf("\r\n%d/%d %d:%d:%d %d\r\n", *month_p, *day_p, *hour_p, *minute_p, *second_p, *year_p);
    #endif
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_RTC_SetDateTime
 *---------------------------------------------------------------------------
 * PURPOSE:  set RTC time
 * INPUT: year  : year value.  
 *        month : month value.
 *        day   : day value
 *        hour  : hour value
 *        minute: minute value
 *        second: second value
 * OUTPUT:   
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SYSDRV_RTC_SetDateTime(int year,    /* 2001-2099 */
                       int month,   /* 01-12 */
                       int day,     /* 01-31 */
                       int hour,    /* 00-23 */
                       int minute,  /* 00-59 */
                       int second)  /* 00-59 */
{
    if( M41T11_Set(year,month,day,hour,minute,second)  != TRUE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_RTC_GetDateTime
 *---------------------------------------------------------------------------
 * PURPOSE:  GET RTC Time
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is NULL.
 * OUTPUT: year_p  : year value
 *         month_p : month value
 *         day_p   : day value
 *         hour_p  : hour value
 *         minute_p: minute value
 *         second_p: second value
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SYSDRV_RTC_GetDateTime(I2CDRV_TwsiDataRead_Func_T read_fn_p,
                               I2CDRV_TwsiDataWrite_Func_T write_fn_p,
                               int *year_p,      /* 2001-2099 */
                               int *month_p,     /* 01-12 */
                               int *day_p,       /* 01-31 */
                               int *hour_p,      /* 00-23 */
                               int *minute_p,    /* 00-59 */
                               int *second_p)    /* 00-59 */
{
    if( M41T11_Get(read_fn_p, write_fn_p, year_p, month_p, day_p, hour_p, minute_p, second_p)  != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}
