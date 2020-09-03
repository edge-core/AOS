#ifndef SYS_TIME_H
#define SYS_TIME_H

/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_TIME.H
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the system timer
 *          functions
 *
 * Notes: This module will support all the system timer functions includes
 *        1) System Ticks
 *        2) RTC timer setting/getting if the H/W support RTC on the board
 *
 *  History
 *
 *   Jason Hsue     11/19/2001      new created
 *   S.K.Yang       05/25/2002      modified
 *   1.Modified SYS_TIME_GetRealTimeBySec,SYS_TIME_GetRealTimeClock,SYS_TIME_SetRealTimeClock
 *   2.Add software clock for situation of non-RTC machine.
 *
 * ------------------------------------------------------------------------
 * Copyright(C)  ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"

#ifdef EPON
#include "leaf_eltbox.h"
#else
#include "leaf_es3626a.h"
#endif
#include "i2cdrv.h"

#define SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE 10 /* 10 minutes */

#define SYS_TIME_MIN_CONFIGURABLE_YEAR  1970  /* for linux system */
#define SYS_TIME_MAX_CONFIGURABLE_YEAR  2037  /* for linux system */

/*time zone range in minutes
 */
#if (SYS_CPNT_A3COM515_SNTP_MIB == TRUE)
#define SYS_TIME_MAX_TIMEZONE_IN_MINUTES           840
#else
#define SYS_TIME_MAX_TIMEZONE_IN_MINUTES           780
#endif

#define SYS_TIME_MIN_TIMEZONE_IN_MINUTES           (-720)

#define SYS_TIME_MAX_TIMEZONE_HOUR                 (SYS_TIME_MAX_TIMEZONE_IN_MINUTES / 60)
#define SYS_TIME_MIN_TIMEZONE_HOUR                 (SYS_TIME_MIN_TIMEZONE_IN_MINUTES / 60)
#define SYS_TIME_MAX_TIMEZONE_MINUTE               59
#define SYS_TIME_MIN_TIMEZONE_MINUTE               0

#define SYS_TIME_ABBREVIATION_STR_LEN      3  /* the length of month & day's abbreviation,
                                               * Month:   "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                                               * Weekday: "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
                                               */
#define SYS_TIME_DATE_TIME_STR_LEN        20  /* example: "Dec 23 15:10:08 2011" */
#define SYS_TIME_PREDEFINED_STR_LEN       11  /* the longest region: "New Zealand" */

#define SYS_TIME_MAX_TIMEZONE_PREDEFINE_STRING_LEN 70

#define SYS_TIME_DEFAULT_TIMEZONE_ID            VAL_sysTimeZonePredefined_none
#define SYS_TIME_DEFAULT_TIMEZONE_NAME          "UTC"
#define SYS_TIME_DEFAULT_TIMEZONE_OFFSET        0

typedef struct
{
    UI32_T year;
    UI32_T month;
    UI32_T day;
    UI32_T hour;
    UI32_T minute;
    UI32_T second;
}SYS_TIME_DST;

typedef struct
{
    UI32_T year;
    UI32_T month;
    UI32_T week;
    UI32_T wday;   /* days since Sunday */
    UI32_T hour;
    UI32_T minute;
    UI32_T second;
}SYS_TIME_RECURRING_DST;

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
#define SYS_TIME_DEFAULT_DST_TIME_OFFSET         60  /* default Time offset in minute */
#define SYS_TIME_MIN_DST_TIME_OFFSET             1                                /* Min Time offset in minute */
#define SYS_TIME_MAX_DST_TIME_OFFSET             MAX_sysSummerTimeOffset          /* Max Time offset in minute */
#define SYS_TIME_MIN_DST_TIMEZONE_NAME_LEN       MINSIZE_sysSummerTimeZoneName
#define SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN       MAXSIZE_sysSummerTimeZoneName

typedef enum
{
    DAYLIGHT_SAVING_TIME_DISABLE                               = 0,
    DAYLIGHT_SAVING_TIME_ENABLE_AMERICA,
    DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA,
    DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA,
    DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND,
    DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED,
    DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
} SYS_TIME_DST_MODE_T;

typedef struct
{
    SYS_TIME_DST_MODE_T mode; /*daylight saving time mode*/
    SYS_TIME_DST first_day;   /* summer-time date */
    SYS_TIME_DST last_day;    /* summer-time date */
    SYS_TIME_RECURRING_DST start_wday;    /* summer-time recuring, predefined */
    SYS_TIME_RECURRING_DST end_wday;      /* summer-time recuring, predefined */
    UI32_T  offset;
    BOOL_T  is_effect;
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
    char    zone_name[SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN+1];
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE) */
    UI32_T  ydays_till_month[2][12];      /* total days from January to the specified month */
} SYS_TIME_DST_Data_T;

#define SYS_TIME_DEFAULT_DAYLIGHT_SAVING_TIME    DAYLIGHT_SAVING_TIME_DISABLE
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

typedef struct
{
    char  zone_name[MAXSIZE_sysTimeZoneName + 1];
    int   offset; /* minutes */
    UI32_T timezone_offset_id;
} Timezone_T;

typedef struct
{
    UI32_T time;     /* seconds from 1970/01/01 00:00:00 */
    UI32_T sys_tick; /* system ticks of time */
} Software_clock_T;

typedef struct
{
    UI64_T accumulated_sys_up_time_base; /* the accumulated system up time in minute, this value must be the same with the one in header of active data block */
    UI32_T bump_up_count; /* the count of bump up, the real accumulated system up time = accumulated_sys_up_time_base + (SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE*bump_up_count) */
    UI32_T prev_bump_up_sys_up_tick; /* the previous system up tick of doing bump up operation */
    UI32_T cursor; /* offset from the beginning of the data block to the byte to flip one bit from 1 to 0 */
    UI32_T data_block_size; /* size of one data block */
    UI32_T data_block_total_num; /* total number of data block for accumulated system up time */
    UI32_T active_data_block_id; /* block id to do bump up accumulated system up time mark */
    UI32_T active_seq_num; /* seq_num in the active data block */
    UI8_T  write_data; /* the data to be written to the byte pointed by cursor when doing bump up mark */
    UI8_T  flip_bit_mask; /* the value of the bit to be flipped from 1 to 0 in the write_data when doing bump up mark */
} SYS_TIME_AccumulatedSysUpTimeControlBlock_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM

    // this 3 variablies for watchdog
    BOOL_T wd_enable;//= FALSE;
    BOOL_T wd_kick;  //= FALSE;
    BOOL_T wd_kick_counter;//= 0;
    UI8_T  board_id;

    Timezone_T time_zone;
    Software_clock_T software_clock;
    UI32_T provision_complete_ticks;
    UI32_T is_provision_complete;
    UI32_T seq_no;
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    SYS_TIME_DST_Data_T  dst_data;
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */
#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)
    SYS_TIME_AccumulatedSysUpTimeControlBlock_T accumulated_sys_up_time_ctl_blk;
#endif
} SYS_TIME_Shmem_Data_T;

#if (SYS_CPNT_EH == TRUE)
typedef enum
{
    SYS_TIME_None_Fun_NO                       = 0,
    SYS_TIME_SetRealTimeClock_Fun_NO           = 1,
    SYS_TIME_SetRealTimeClockByStr_Fun_NO      = 2,
    SYS_TIME_GetTimeZone_Fun_NO                = 3,
    SYS_TIME_GetTimeZoneByStr_Fun_NO           = 4,
    SYS_TIME_SetTimeZoneByStr_Fun_NO           = 5,
    SYS_TIME_SetTimeZone_Fun_NO                = 6,
    SYS_TIME_GetTimeZoneNameByStr_Fun_NO       = 7,
    SYS_TIME_SetTimeZoneNameByStr_Fun_NO       = 8,
    SYS_TIME_GetSoftwareClockBySec_Fun_NO      = 9,
    SYS_MGR_Init_Fun_NO                        = 10,
    SYS_MGR_SetPasswordThreShold_Fun_NO        = 11,
    SYS_MGR_SetConsoleInActiveTimeOut_Fun_NO   = 12,
    SYS_MGR_SetConsoleSilentTime_Fun_NO        = 13,
    SYS_MGR_SetTelnetPasswordThreshold_Fun_NO  = 14,
    SYS_MGR_SetTelnetInActiveTimeOut_Fun_NO    = 15,
    SYS_MGR_SetUartParity_Fun_NO               = 16,
    SYS_MGR_SetUartDataBits_Fun_NO             = 17,
    SYS_MGR_SetUartStopBits_Fun_NO             = 18,
    SYS_MGR_GetSysInfo_Fun_NO                  = 19
} SYS_MGR_UI_MESSAGE_FUNC_NO_T;
#endif /* SYS_CPNT_EH */

typedef enum
{
    SYS_TIME_TIMEZONE_PLUS,
    SYS_TIME_TIMEZONE_MINUS
} SYS_TIME_TIMEZONE_SIGN_T;

typedef enum
{
    SYS_TIME_TIMEZONE_TYPE_TIMEZONE = 0,
    SYS_TIME_TIMEZONE_TYPE_TIMEZONE_PREDEFINE = 1
}SYS_TIME_TIMEZONE_TYPE_T;

typedef struct
{
    SYS_TIME_TIMEZONE_TYPE_T type;
    union
    {
        struct SYS_TIME_Type_Timezone_S
        {
            char  name[MAXSIZE_sysTimeZoneName + 1];
            UI32_T                   hour;
            UI32_T                   minute;
            SYS_TIME_TIMEZONE_SIGN_T sign;
        }custom;

        struct SYS_TIME_Type_Timezone_Predefined_S
        {
            char  name[SYS_TIME_MAX_TIMEZONE_PREDEFINE_STRING_LEN + 1];
            UI32_T  id;
        }predefined;

    } timezone;
} SYS_TIME_Timezone_T;

typedef struct
{
    UI32_T  id;
    int     offest;
    char    name[SYS_TIME_MAX_TIMEZONE_PREDEFINE_STRING_LEN + 1];
    UI32_T  index;
} SYS_TIME_Timezone_Predefined_T;

#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)
typedef struct
{
    UI32_T magic;
    UI32_T seq_num;
    UI64_T accumulated_sys_up_time_base; /* accumulated system up time base in minute */
    UI32_T reserved[7]; /* for extension in the future, always set as 0 when not used */
    UI32_T checksum;
} SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T;
#endif

/* FUNCTION NAME: SYS_TIME_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T SYS_TIME_InitiateSystemResources(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_Init_InitiateProcessResources
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 *
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is
 *                         NULL.
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This routine will initialize the software clock by copy time
 *           information from hardware clock if RTC exists.
 *        2. If RTC doesn't exists, then set software clock to zero.
 *        3. Software clock always store UTC time in seconds point from
 *           1970/01/01 00:00:00
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_Init_InitiateProcessResources(I2CDRV_TwsiDataRead_Func_T read_fn_p,
                     I2CDRV_TwsiDataWrite_Func_T write_fn_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYS_TIME_Create_InterCSC_Relation(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode
 * INPUT : None
 * OUTPUT: None
 * RETURN: None
 * NOTES : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnterMasterMode(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave
 * INPUT : None
 * OUTPUT: None
 * RETURN: None
 * NOTES : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnterSlaveMode(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode
 * INPUT : None
 * OUTPUT: None
 * RETURN: None
 * NOTES : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SYS_TIME into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SYS_TIME_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function is used to notify provision complete.
 * INPUT: None
 * OUTPUT:
 * RETURN: none
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ProvisionComplete(UI32_T unit);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeClock
 * ---------------------------------------------------------------------
 * PURPOSE  : get real TOD(Time-Of-Day)
 * INPUT    : None
 * OUTPUT   : year, month, day, hour, minute, second
 * RETURN   : None
 * NOTES    : 1.If there is no RTC in the system, then then get time
 *              from S/W clock. (Local time)
 *            2.If there is no RTC in system, then this routine is same
 *              as SYS_TIME_GetSoftwareClock
 * History  :  S.K.Yang     06/10/2002      modified
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeClock(int   *year,      /* 2001-2100 */
                               int   *month,     /* 01-12 */
                               int   *day,       /* 01-31 */
                               int   *hour,      /* 00-23 */
                               int   *minute,    /* 00-59 */
                               int   *second);   /* 00-59 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeBySec
 * ---------------------------------------------------------------------
 * PURPOSE  : get real time in seconds
 * INPUT    : None
 * OUTPUT   : seconds
 * RETURN   : None
 * NOTES    : 1. This API is used for Sys_Log. When the system log try to
 *               log event, he will call this API to get current time by
 *               how many seconds.
 *            2. This API will always based on 1/1/2001 00:00:00 as 0 second
 *               to count how many seconds pass away from that time.
 *            3. If there is no RTC in the system, then then get seconds
 *                from S/W clock (Local Time)
 *            4. If no RTC exists, this rountine behave like SYS_TIME_GetSoftwareClockBySec
 * History  :  S.K.Yang     06/7/2002      modified
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeBySec(UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertSecondsToDateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : convert real time based on the input -- how many seconds pass away
 *            since the time 1/1/1970 00:00:00 to UTC date time
 * INPUT    : None
 * OUTPUT   : year, month, day, hour, minute, second
 * RETURN   : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertSecondsToDateTime(UI32_T seconds, int *year, int *month, int *day,
                                       int *hour, int *minute, int *second);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertSecondsToLocalDateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : convert real time based on the input -- how many seconds pass away
 *            since the time 1/1/1970 00:00:00 to local date time
 * INPUT    : None
 * OUTPUT   : year, month, day, hour, minute, second
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertSecondsToLocalDateTime(UI32_T seconds, int *year, int *month, int *day,
                                            int *hour, int *minute, int *second);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertDateTimeToSeconds
 * ---------------------------------------------------------------------
 * PURPOSE  : convert real time in seconds based on the input
 *            -- Year,month,day,hour,min,second to seconds
 * INPUT    : TOD:Year,month,day,hour,min,second
 * OUTPUT   : Seconds from 1970/01/01 00:00:00 base on input
 * RETURN   : None
 * NOTES    : 1. Symmetry function of SYS_TIME_ConvertDateTimeToSeconds
 * History  : S.K.Yang     06/10/2002      new added
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertDateTimeToSeconds(int year, int month, int day, int hour,
                                       int minute, int second, UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ValidateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : Validates time
 * INPUT    : year, month, day, hour, minute, second
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_ValidateTime(int year, int month, int day, int hour,
                             int minute, int second);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetRealTimeClock
 * ---------------------------------------------------------------------
 * PURPOSE: set real TOD(Time-Of-Day)
 * INPUT    : year, month, day, hour, minute, second
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 1. This API is the only entrance used to set up local time.
 *            2. Set to RTC  if it exists, and set it to Software clock,
 *               so h/w clock is synchronous with s/w clock.
 *            3. When set time to software/hardware clock, this routine will trans-
 *               late local time to UTC format.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_SetRealTimeClock(int   year,      /* 2001-2099 */
                                 int   month,     /* 01-12 */
                                 int   day,       /* 01-31 */
                                 int   hour,      /* 00-23 */
                                 int   minute,    /* 00-59 */
                                 int   second);   /* 00-59 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetRealTimeClockByStr
 * ---------------------------------------------------------------------
 * PURPOSE: set real time clock by string
 * INPUT    : String like below :
 *                   Date-time string :   J|a|n| | |3| |1|5|:| 1| 4| :| 1| 3|  | 1| 9| 8| 8|\0|
 *               Character Position :     0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    :
 *
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_SetRealTimeClockByStr(const char *clock_time_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemUpTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used by CLI and WEB to tell how many days, hours,
 *            minutes, seconds and miliseconds this box has run after
 *            system running.
 * INPUT    : None
 * OUTPUT   : days, hours, minutes, seconds, miliseconos
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetSystemUpTime(UI32_T   *days,
                              UI32_T   *hours,
                              UI32_T   *minutes,
                              UI32_T   *seconds,
                              UI32_T   *miliseconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemUpTimeByTick
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to get system up time by tick.
 * INPUT    : None
 * OUTPUT   : Ticks
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetSystemUpTimeByTick(UI32_T *ticks);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemTicksBy10ms
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used for SNMP or those functions which needs to
 *            get system tick count.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : The most tickSet() value, plus all tickAnnounce() calls since.
 * NOTES    : VxWorks kernel also supports tickGet() to get systemUpTicks,
 *            but for maintainance and generic purpose.  Our core layer
 *            and application layer had better use this API instead of
 *            call tickGet().
 * ---------------------------------------------------------------------
 */
UI32_T SYS_TIME_GetSystemTicksBy10ms(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetSystemTicksBy10ms
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used for SNMP or those functions which needs to
 *            set system tick count.
 * INPUT    : ticks -- new time in ticks
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : VxWorks kernel also supports tickSet() to set systemUpTicks,
 *            but for maintainance and generic purpose.  Our core layer
 *            and application layer had better use this API instead of
 *            call tickSet().
 * ---------------------------------------------------------------------
 */
void SYS_TIME_SetSystemTicksBy10ms(UI32_T ticks);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnableWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to enable watch dog timer.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnableWatchDogTimer(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_DisableWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to disable watch dog timer.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_DisableWatchDogTimer(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_KickWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to kick watch dog timer to prevent that
 *            watch dog timer time out.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_KickWatchDogTimer(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZone
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time zone and name of time zone
 * INPUT    : A pointer point to buffer
 * OUTPUT   : timezone - timezone data.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------
*/
BOOL_T 
SYS_TIME_GetTimeZone(
    SYS_TIME_Timezone_T *timezone
    );

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZoneByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time zone by string
 * INPUT    : A pointer point to buffer
 * OUTPUT   : 1. time offset relative to GMT e.q, +8:00 characters
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_GetTimeZoneByStr(char *time_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZone
 *------------------------------------------------------------------------------
 * PURPOSE  : Set time zone and name of time zone
 * INPUT    : zone_name_p - Time zone name
 *            sign        - Plus or minus.
 *                          SYS_TIME_TIMEZONE_PLUS: UTC plus the time
 *                          SYS_TIME_TIMEZONE_MINUS: UTC minus the time
 *            hour        - Time zone hours
 *            minute      - Time zone minutes
 * OUTPUT   : none
 * RETURN   : TRUE if success,FALSE if time zone out of range -780 < zone <720 (min)
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZone(const char *zone_name_p, SYS_TIME_TIMEZONE_SIGN_T sign, UI32_T hour, UI32_T minute);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZoneNameByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Get name of time zone
 * INPUT    : 1.Time zone name
 * OUTPUT   : none
 * RETURN   : TRUE if success,FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_GetTimeZoneNameByStr(char *name_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZoneNameByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Set  name of time zone
 * INPUT    : A pointer point to buffer
 * OUTPUT   : 1. zone_name
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZoneNameByStr(const char *name_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZoneByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Set time zone by string
 * INPUT    : A pointer point to buffer
 * OUTPUT   :
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZoneByStr(const char *time_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetRunningTimeZone
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the timezone of system
 * INPUT:    None
 * OUTPUT:   In minutes
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: 
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T 
SYS_TIME_GetRunningTimeZone(
    SYS_TIME_Timezone_T *timezone
    );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetDayAndTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the DayAndTime in RFC_2579 for SNMP usage.
 * INPUT:    None.
 * OUTPUT:   day_and_time     - The current local time or the UTC time.
 *           day_and_time_len - The length of the time.
 * RETURN:   TRUE/FALSE
 * NOTE  :   1.Refer to RFC2579
 *             DateAndTime ::= TEXTUAL-CONVENTION
 *              field  octets  contents                  range
 *              -----  ------  --------                  -----
 *                1      1-2   year*                     0..65536
 *                2       3    month                     1..12
 *                3       4    day                       1..31
 *                4       5    hour                      0..23
 *                5       6    minutes                   0..59
 *                6       7    seconds                   0..60
 *                             (use 60 for leap-second)
 *                7       8    deci-seconds              0..9
 *                8       9    direction from UTC        '+' / '-'
 *                9      10    hours from UTC*           0..13
 *               10      11    minutes from UTC          0..59
 *---------------------------------------------------------------------------*/

BOOL_T SYS_TIME_GetDayAndTime(UI8_T *day_and_time, UI32_T *day_and_time_len);

/*----------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetSoftwareClock
 *---------------------------------------------------------------------------
 * PURPOSE:  Get software clock
 * INPUT:    Buffer of year, month, day, hour, minute, second
 * OUTPUT:   year,month,day,hour,minute of local time.
 * RETURN:   TRUE/FALSE
 * NOTE:     1. This routine will give you local time by year, month,day,hour,
 *              minute,second."year" may be 2 if 2002 , 3 if 2003,etc,.
 *           2. This routine provide you local time.
 *           3. The software clock may be setted by SNTP(when SNTP is enabled)
 *               or RTC(when it exists, or setted by user)
 *           4. Even RTC or SNTP doesn't exists, software clock always has a base time
 *              2001/01/01 00:00:00
 *           5. It will be drifted away during polling interval. Or if sntp is not available
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_GetSoftwareClock(int *year, int *month, int *day,
                                 int *hour, int *minute, int *second);

/*----------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetSoftwareClockBySec
 *---------------------------------------------------------------------------
 * PURPOSE:  Get software clock by seconds
 * INPUT:    buffer of seconds
 * OUTPUT:   Local time in seconds from 1970/01/01 00:00:00
 * RETURN:   TRUE/FALSE
 * NOTE:     1. This routine get time in seconds
 *           2. This routine provide you local time
 *           3. It will be drifted away during polling interval. Or if sntp is not available
 *---------------------------------------------------------------------------*/
 BOOL_T  SYS_TIME_GetSoftwareClockBySec(UI32_T *sec);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_ConvertTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Make  time information into format as May 21 10:11:11 2002
 * INPUT    : 1.Time in seconds from 1970/01/01/,00:0:00:00
 *            2.Buffer pointer stored time information , 22 characters buffer needed
 * OUTPUT   : "May 21 10:11:11 2002" format
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL.
 * NOTES    : Return is as the form :
 *            Date-time string :      J|a|n| | |3| |1|5|:| 1| 4| :| 1| 3|  | 1| 9| 8| 8|\0|
 *            Character Position :    0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|
 *            so 22 characters needed
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertTime(UI32_T time, char *date_time_p);

/*-------------------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetRealTimeClockBySeconds
 *-------------------------------------------------------------------------------------------
 * PURPOSE  : Get time information from sntp to calculate system up time
 *            when RTC is not available.
 * INPUT    : 1. seconds : seconds from 1970/01/01 00:00:00
 *            2. systick : system tick number when  time packet is received
 * OUTPUT   : none
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL
 *            never get time from network.
 * NOTES    :1. --|--------------------------------|---------> time coordinate
 *                V                              V
 *              system up time                   sntp packet received
 *                                               and get system tick number on this point
 *           2. This routine is called by SNTP_MGR when sntp is enabled
 *              and received time packet.
 *           3. Once this routine is called, it will update software clock and hardware clock.
 *           4. Called by SNTP_MGR
 *-------------------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetRealTimeClockBySeconds(UI32_T seconds, UI32_T systick);

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetKick
 * ---------------------------------------------------------------------
 * PURPOSE  : This function set kick on/off.
 * INPUT    : setting -- turn on/off kick
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : 1.For debug issue, we need support this function.
 *            2.We need turn on kick after enable watch dog timer.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_SetKick(BOOL_T setting);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetKickCounter
 * ---------------------------------------------------------------------
 * PURPOSE  : This function get kick counter.
 * INPUT    : None.
 * OUTPUT   : counter - kick number.
 * RETURN   : None.
 * NOTES    : 1.For debug issue, we need support this function.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetKickCounter(UI32_T *tcounter);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ClearWatchdogIntStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : This function clear watchdog int status.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    :
 * EPR00057114,sunnyt 2005/07/29
 * in 8248, watchdog timeout is trigger by irq5.
 * We need to clear watchdog irq status
 * So, need to add one function in sys_time to clear irq status
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ClearWatchdogIntStatus();

#endif /* SYS_CPNT_WATCHDOG_TIMER */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_TIME_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SYS_TIME in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_TIME_AttachSystemResources(void);

/**/
void SYS_TIME_GetUtcRealTimeBySec(UI32_T *seconds);

void SYS_TIME_Main(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetDayOfWeek
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get weekday from seconds
 * INPUT    : UI32_T year
 *            UI32_T month
 *            UI32_T day
 * OUTPUT   : UI32_T *wday - days since Sunday
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_GetDayOfWeek(UI32_T second, UI32_T *wday);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_IsTimeModify
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will be used to check sys_time is changed by sntp or admininstrator
 * INPUT:    NONE
 * OUTPUT:   NONE
 * RETURN:   TRUE/FALSE
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_IsTimeModify(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNextProcessDate
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will be used to get next process date which days of month is greater than input date
 * INPUT:    NONE
 * OUTPUT:   NONE
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_GetNextProcessDate(UI32_T year, UI32_T month, UI32_T day, UI32_T *next_year, UI32_T *next_month);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertTicksToDiffTime
 * ---------------------------------------------------------------------
 * PURPOSE  : convert ticks to weeks/days/hours/minutes/seconds for UI
 *            to display delta time.
 * INPUT    : ticks
 * OUTPUT   : weeks, days, hours, minutes, seconds, milliseconds
 * RETURN   : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertTicksToDiffTime(UI32_T ticks, UI32_T *weeks, UI32_T *days,
                                     UI32_T *hours, UI32_T *minutes, UI32_T *seconds,
                                     UI32_T *milliseconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetUTC
 * ---------------------------------------------------------------------
 * PURPOSE  : Get current UTC since 1/1/2001 in seconds.
 * INPUT    : None
 * OUTPUT   : seconds
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetUTC(UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeSecondsFromTOD
 * ---------------------------------------------------------------------
 * PURPOSE  : get real time in seconds based on the input
 *            -- Year,month,day,hour,min,second
 * INPUT    : TOD:Year,month,day,hour,min,second
 * OUTPUT   : Seconds from 2001/01/01 00:00:00 base on input
 * RETURN   : None
 * NOTES    : 1. Symmetry function of  SYS_TIME_ConvertSecondsToDateTime
 * History  :  S.K.Yang     06/10/2002      new added
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeSecondsFromTOD(int year, int month, int day, int hour,
                                        int minute, int second, UI32_T  *seconds);

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST mode
 * INPUT    : mode_p
 * OUTPUT   : mode_p
 *            DAYLIGHT_SAVING_TIME_DISABLE
 *            DAYLIGHT_SAVING_TIME_ENABLE_AMERICA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND,
 *            DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED,
 *            DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
 * RETURN   : none
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_DISABLE, then it means enable.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetMode(SYS_TIME_DST_MODE_T *mode_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningMode
 *---------------------------------------------------------------------------
 * PURPOSE  : Get running DST mode
 * INPUT    : mode_p
 * OUTPUT   : mode_p
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningMode(SYS_TIME_DST_MODE_T *mode_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetPredefinedName
 *------------------------------------------------------------------------------
 * PURPOSE  : get currnet predefined name
 * INPUT    : none
 * OUTPUT   : predefined name
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetPredefinedName(char *predefined_name_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetPredefinedMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST predefined mode
 * INPUT    : mode.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if mode == DAYLIGHT_SAVING_TIME_DISABLE
              then disbale Daylight Saving Time function.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetPredefinedMode(SYS_TIME_DST_MODE_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetUserConfigTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST user config
 * INPUT    : mode, start day and end day of daylight saving time.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED
 *            then return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetUserConfigTime(SYS_TIME_DST_MODE_T mode, const SYS_TIME_DST *begin_dst_p, const SYS_TIME_DST *end_dst_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetUserConfigTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST user config
 * INPUT    : mode, buffer of start day and end day of daylight saving time.
 * OUTPUT   : *begin_dst -- begin dst
 *            *end_dst   -- end dst
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED
              then return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetUserConfigTime(SYS_TIME_DST_MODE_T mode, SYS_TIME_DST *begin_dst_p, SYS_TIME_DST *end_dst_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetRecurringTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST recurring time
 * INPUT    : mode, start day and end day of daylight saving time.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
              will return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetRecurringTime(SYS_TIME_DST_MODE_T mode, const SYS_TIME_RECURRING_DST *begin_dst_p, const SYS_TIME_RECURRING_DST *end_dst_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetRecurringTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST recurring settings
 * INPUT    : mode, bffer of start day and end day of daylight saving time.
 * OUTPUT   : *begin_dst_p -- begin dst
 *            *end_dst_p   -- end dst
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
              will return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetRecurringTime(SYS_TIME_DST_MODE_T mode, SYS_TIME_RECURRING_DST *begin_dst_p, SYS_TIME_RECURRING_DST *end_dst_p);

#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetZoneName
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST zone name
 * INPUT    : zone name
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetZoneName(const char *zone_name_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetZoneName
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST zone name
 * INPUT    : none
 * OUTPUT   : zone name
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetZoneName(char *zone_name_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningZoneName
 *---------------------------------------------------------------------------
 * PURPOSE : This function will get the DST zone name of the system
 * INPUT   : none
 * OUTPUT  : zone name
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE    : none
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningZoneName(char *zone_name_p);
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_SetTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will set the DST time offset
 * INPUT    : time_offset -- time offset in minute
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetTimeOffset(const UI32_T time_offset);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the DST time offset
 * INPUT    : *time_offset_p -- buffer of  time offset in minute
 * OUTPUT   : time_offset_p -- timeoffset
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetTimeOffset(UI32_T *time_offset_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the running DST time offset
 * INPUT    : *time_offset_p -- buffer of  time offset in minute
 * OUTPUT   : time_offset_p -- timeoffset
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *            SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE     : none
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningTimeOffset(UI32_T *time_offset_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_IsInSummerTime
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will check if is in date of configured summer time
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - daylight saving time is in effect
 *            FALSE - daylight saving time is not in effect
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_IsInSummerTime(void);
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_ConvertMonth
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will convert month number to English
 * INPUT    : UI32_T mon
 * OUTPUT   : char month_ar - Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
 * RETURN   : TRUE - month number is in effect
 *            FALSE - month number is out of range
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertMonth(UI32_T mon, char *month_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_ConvertWeekday
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will convert week day number to English
 * INPUT    : UI32_T day
 * OUTPUT   : char weekday_ar - Sun Mon Tue Wed Thu Fri Sat
 * RETURN   : TRUE - day number is in effect
 *            FALSE - day number is out of range
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertWeekday(UI32_T day, char *weekday_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_SetTimezonePredefined
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will set timezone-predefined
 * INPUT    : timezone_id
 * OUTPUT   : none
 * RETURN   : TRUE - success.
 *            FALSE - fail.
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T
SYS_TIME_SetTimezonePredefined(
    UI32_T  timezone_id
    );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNextTimeZonePredefinedData
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the next timezone-predefined data.
 * INPUT    : timezone_predefined->index
 * OUTPUT   : timezone_predefined - next timezone-predefined data.
 * RETURN   : TRUE - success.
 *            FALSE - fail.
 * NOTE     : The index of the timezone-predefined data is from 
 *            0 to _countof(timezone_predefined_id) - 1.
 *            Use 0xffffffff as timezone_predefined->index to get the 
 *            first entry.
 *---------------------------------------------------------------------------*/
BOOL_T 
SYS_TIME_GetNextPredefinedTimeZone(
    SYS_TIME_Timezone_Predefined_T *timezone_predefined
    );

#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_AccumulatedUpTime_Get
 * ------------------------------------------------------------------------
 * FUNCTION : This function gets the accumulated system up time(in minute).
 * INPUT    : None.
 * OUTPUT   : accumulated_time  -  The accumulated system up time(in minute)
 * RETURN   : TRUE  -  The accumulated system up time is output successfully.
 *            FALSE -  Error to get accumulated system up time.
 * NOTE     : The accumulated system up time will be kept counting whenever
 *            the system is up.
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_AccumulatedUpTime_Get(UI64_T *accumulated_time_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_BumpUpAccumulatedSysUpTime
 * ------------------------------------------------------------------------
 * FUNCTION : This function bumps up the statistic of the accumulated systime
 *            up time.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : 1. The accumulated system up time will be kept counting whenever
 *               the system is up.
 *            2. The accumulated system up time will be increased by
 *               SYS_TIME_ACCUMULATED_SYS_UP_TIME_ONE_TIME_SLICE_IN_TICK for
 *               each call to this function.
 *            3. This function should be called by a thread periodically to
 *               update the accumulated system up time.
 * ------------------------------------------------------------------------
 */
void SYS_TIME_BumpUpAccumulatedSysUpTime(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_InitAccumulatedSysUpTimeContrlBlock
 * ------------------------------------------------------------------------
 * FUNCTION : Initialize the fields of the control block for accumulated
 *            system up time.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : This function should be called by SYSRSC_MGR.
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_InitAccumulatedSysUpTimeContrlBlock(void);
#endif /* end of #if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE) */

#endif /* End of SYS_TIME_H */

