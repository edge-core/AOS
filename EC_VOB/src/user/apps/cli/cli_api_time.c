#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_api_time.h"
#include "sys_time.h"

#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif

static int check_month(char *arg);

static UI32_T
parsing_timezone_str(
    char *arg[]
    );

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
static void get_order_str(int order, char *order_str);
static UI32_T check_day(char *day);
static BOOL_T detect_date_input_time(char *arg[], SYS_TIME_DST *begin_dst, SYS_TIME_DST *end_dst);
static BOOL_T detect_recurring_input_time(char *arg[], SYS_TIME_RECURRING_DST *begin_dst, SYS_TIME_RECURRING_DST *end_dst);
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

UI32_T CLI_API_Calendar_Set(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    int  year, month, day, hour, minute, second;
    
    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;
    hour   = atoi(arg[0]);
    
    if(arg[1]==NULL)
        return CLI_ERR_INTERNAL;
    minute = atoi(arg[1]);
    
    if(arg[2]==NULL)
        return CLI_ERR_INTERNAL;
    second = atoi(arg[2]);
    
    if((arg[3]==NULL) && (arg[4]==NULL))
        return CLI_ERR_INTERNAL;
    if(isdigit(arg[3][0])) /*day month*/
    {
        day = atoi(arg[3]);
        if((month = check_month(arg[4])) == 0)
            return CLI_ERR_INTERNAL;
    }
    else                   /*month day */
    {
        if((month = check_month(arg[3])) == 0)
            return CLI_ERR_INTERNAL;
        day = atoi(arg[4]);
    }

    if(arg[5]==NULL)
        return CLI_ERR_INTERNAL;
    year = atoi(arg[5]);
   
    if (!SYS_TIME_SetRealTimeClock(year, month, day, hour, minute, second))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set calendar.\r\n");
#endif
    }
    return CLI_NO_ERROR;
}

static int check_month(char *arg)
{
   switch(arg[0])
   {
   case 'a':
   case 'A':
      switch(arg[1])
      {
      case 'p': /*april*/
      case 'P':
         return 4;
         
      case 'u': /*august*/
      case 'U':
         return 8;
      }
      break;
      
   case 'd':   /*december*/
   case 'D':
      return 12;
      
   case 'f':   /*february*/
   case 'F':
      return 2;
      
   case 'j':
   case 'J':
      switch(arg[1])
      {
      case 'a': /*january*/
      case 'A':
         return 1;
         
      case 'u':
      case 'U':
         switch(arg[2])
         {
         case 'l': /*july*/
         case 'L':
            return 7;
            
         case 'n': /*june*/
         case 'N':
            return 6;
         }
         break;
      }
      break;
   
   case 'm':
   case 'M':
      switch(arg[2])
      {
      case 'r': /*march*/
      case 'R':
         return 3;
         
      case 'y': /*may*/
      case 'Y':
         return 5;
      }
      break;
      
      
   case 'n': /*november*/
   case 'N':
      return 11;
      
   case 'o': /*october*/
   case 'O':
      return 10;
   
   case 's': /*september*/
   case 'S':
      return 9;
   }

   return 0;
}

UI32_T 
CLI_API_Show_Calendar(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  seconds;

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
   SYS_TIME_DST_MODE_T summer_time_mode;
   char month_str[SYS_TIME_ABBREVIATION_STR_LEN+1] = {0};   /* abbreviations of month, e.g. "Jan", "Feb" etc. */
   char weekday_str[SYS_TIME_ABBREVIATION_STR_LEN+1] = {0}; /* abbreviations of week, e.g. "Sun", "Mon" etc. */
   char order_str[5] = {0}; /* max example: "last" + NUL-ending */
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
   char summer_time_zone_name[SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN+1] = {0};
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE) */
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

    char    date_time_str[SYS_TIME_DATE_TIME_STR_LEN+1] = {0};
    SYS_TIME_Timezone_T timezone;

    /* current time information
     */
    SYS_TIME_GetRealTimeBySec(&seconds);
    SYS_TIME_ConvertTime(seconds, date_time_str);
    CLI_LIB_PrintStr_1(" Current Time          : %s\r\n", date_time_str);

#if (SYS_CPNT_SYS_TIME == TRUE)
    /* time zone information
     */
    if (TRUE == SYS_TIME_GetTimeZone(&timezone))
    {
        if (SYS_TIME_TIMEZONE_TYPE_TIMEZONE == timezone.type)
        {
            CLI_LIB_PrintStr_4(" Time Zone             : %s, %s%02lu:%02lu\r\n",
                               timezone.timezone.custom.name, 
                               (timezone.timezone.custom.sign == SYS_TIME_TIMEZONE_MINUS) ? "-" : "", 
                               (unsigned long)timezone.timezone.custom.hour, 
                               (unsigned long)timezone.timezone.custom.minute);          
        }
        else
        {
            CLI_LIB_PrintStr_1(" Time Zone             : %s\r\n", timezone.timezone.predefined.name);
        }
    }
#endif /* #if (SYS_CPNT_SYS_TIME == TRUE) */

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    /* summer time information
     */
    if ((TRUE == SYS_TIME_DST_GetMode(&summer_time_mode))
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
        && (TRUE == SYS_TIME_DST_GetZoneName(summer_time_zone_name))
#endif
       )
    {
        UI32_T offset = 0;
        char predefined_name[SYS_TIME_PREDEFINED_STR_LEN+1] = {0};
        SYS_TIME_DST date_begin_dst;
        SYS_TIME_DST date_end_dst;
        SYS_TIME_RECURRING_DST rec_begin_dst;
        SYS_TIME_RECURRING_DST rec_end_dst;
        memset(&date_begin_dst, 0, sizeof(date_begin_dst));
        memset(&date_end_dst, 0, sizeof(date_end_dst));
        memset(&rec_begin_dst, 0, sizeof(rec_begin_dst));
        memset(&rec_end_dst, 0, sizeof(rec_end_dst));

        switch (summer_time_mode)
        {
            case DAYLIGHT_SAVING_TIME_DISABLE:
                /* summer time is not configured
                 */
                CLI_LIB_PrintStr(" Summer Time           : Not configured\r\n");
                break;

            case DAYLIGHT_SAVING_TIME_ENABLE_AMERICA:
            case DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA:
            case DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA:
            case DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND:
                if (TRUE == SYS_TIME_DST_GetPredefinedName(predefined_name))
                {
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
                    CLI_LIB_PrintStr_2(" Summer Time           : %s, %s region\r\n", summer_time_zone_name, predefined_name);
#else
                    CLI_LIB_PrintStr_1(" Summer Time           : %s\r\n", predefined_name);
#endif
                }
                break;

            case DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED:
                /* date mode
                 */
                if ((TRUE == SYS_TIME_DST_GetTimeOffset(&offset)) &&
                    (TRUE == SYS_TIME_DST_GetUserConfigTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED,
                                                            &date_begin_dst,
                                                            &date_end_dst)))
                {
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
                    CLI_LIB_PrintStr_1(" Summer Time           : %s, ", summer_time_zone_name);
#else
                    CLI_LIB_PrintStr(" Summer Time           : ");
#endif

                    CLI_LIB_PrintStr_1("offset %lu minutes\r\n", (unsigned long)offset);

                    SYS_TIME_ConvertMonth(date_begin_dst.month, month_str);
                    CLI_LIB_PrintStr_3("                         %s %lu %lu ", month_str,
                                                                               (unsigned long)date_begin_dst.day,
                                                                               (unsigned long)date_begin_dst.year);

                    CLI_LIB_PrintStr_2("%02lu:%02lu to ", (unsigned long)date_begin_dst.hour,
                                                          (unsigned long)date_begin_dst.minute);

                    SYS_TIME_ConvertMonth(date_end_dst.month, month_str);
                    CLI_LIB_PrintStr_3("%s %lu %lu ", month_str,
                                                      (unsigned long)date_end_dst.day,
                                                      (unsigned long)date_end_dst.year);

                    CLI_LIB_PrintStr_2("%02lu:%02lu\r\n", (unsigned long)date_end_dst.hour,
                                                          (unsigned long)date_end_dst.minute);
                }
                break;

            case DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING:
                /* recurring mode
                 */
                if ((TRUE == SYS_TIME_DST_GetTimeOffset(&offset)) &&
                    (TRUE == SYS_TIME_DST_GetRecurringTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING,
                                                           &rec_begin_dst,
                                                           &rec_end_dst)))
                {
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
                    CLI_LIB_PrintStr_1(" Summer Time           : %s, ", summer_time_zone_name);
#else
                    CLI_LIB_PrintStr(" Summer Time           : ");
#endif

                    CLI_LIB_PrintStr_1("offset %lu minutes\r\n", (unsigned long)offset);

                    SYS_TIME_ConvertMonth(rec_begin_dst.month, month_str);
                    SYS_TIME_ConvertWeekday(rec_begin_dst.wday, weekday_str);
                    get_order_str(rec_begin_dst.week, order_str);
                    CLI_LIB_PrintStr_3("                         %s %s, %s, ", order_str,
                                                                               weekday_str,
                                                                               month_str);

                    CLI_LIB_PrintStr_2("%02lu:%02lu to ", (unsigned long)rec_begin_dst.hour,
                                                          (unsigned long)rec_begin_dst.minute);

                    SYS_TIME_ConvertMonth(rec_end_dst.month, month_str);
                    SYS_TIME_ConvertWeekday(rec_end_dst.wday, weekday_str);
                    get_order_str(rec_end_dst.week, order_str);
                    CLI_LIB_PrintStr_3("%s %s, %s, ", order_str,
                                                      weekday_str,
                                                      month_str);

                    CLI_LIB_PrintStr_2("%02lu:%02lu\r\n", (unsigned long)rec_end_dst.hour,
                                                          (unsigned long)rec_end_dst.minute);
                }

            default:
                break;
        } /* end of switch (summer_time_mode) */

        /* show summer time is in effect or not
         */
        if (TRUE == SYS_TIME_DST_IsInSummerTime())
        {
            CLI_LIB_PrintStr(" Summer Time in Effect : Yes\r\n");
        }
        else
        {
            CLI_LIB_PrintStr(" Summer Time in Effect : No\r\n");
        }
    } /* end of summer time information */
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Clock_SummerTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
    UI32_T arg_offset = 1; /* offset one argument */
#else
    UI32_T arg_offset = 0; /* no offset */
#endif
    UI32_T time_offset = 0;
    SYS_TIME_DST_MODE_T daylight_mode = SYS_TIME_DEFAULT_DAYLIGHT_SAVING_TIME;
    SYS_TIME_DST date_begin_dst;
    SYS_TIME_DST date_end_dst;
    SYS_TIME_RECURRING_DST rec_begin_dst;
    SYS_TIME_RECURRING_DST rec_end_dst;
    memset(&date_begin_dst, 0, sizeof(date_begin_dst));
    memset(&date_end_dst, 0, sizeof(date_end_dst));
    memset(&rec_begin_dst, 0, sizeof(rec_begin_dst));
    memset(&rec_end_dst, 0, sizeof(rec_end_dst));

    /* to disable summer time
     */
    if (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CLOCK_SUMMERTIME)
    {
        if (FALSE == SYS_TIME_DST_SetPredefinedMode(DAYLIGHT_SAVING_TIME_DISABLE))
        {
            CLI_LIB_PrintStr("Failed to disable summer time.\r\n");
        }
        return CLI_NO_ERROR;
    }

    switch (arg[0 + arg_offset][0])
    {
        case 'R':
        case 'r':
            daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING;
            if (FALSE == detect_recurring_input_time(arg, &rec_begin_dst, &rec_end_dst))
            {
                return CLI_ERR_INTERNAL;
            }

            if (FALSE == SYS_TIME_DST_SetRecurringTime(daylight_mode,
                                                       &rec_begin_dst,
                                                       &rec_end_dst))
            {
                CLI_LIB_PrintStr("Failed to set summer recurring time.\r\n");
            }
            break;

        case 'D':
        case 'd':
            daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED;
            if (FALSE == detect_date_input_time(arg, &date_begin_dst, &date_end_dst))
            {
                return CLI_ERR_INTERNAL;
            }

            if (FALSE == SYS_TIME_DST_SetUserConfigTime(daylight_mode,
                                                        &date_begin_dst,
                                                        &date_end_dst))
            {
                CLI_LIB_PrintStr("Failed to set summer time.\r\n");
            }
            break;

        case 'P':
        case 'p':
            switch (arg[1 + arg_offset][0])
            {
                case 'E':
                case 'e':
                    daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA;
                    break;

                case 'U':
                case 'u':
                    daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_AMERICA;
                    break;

                case 'A':
                case 'a':
                    daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA;
                    break;

                case 'N':
                case 'n':
                    daylight_mode = DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (FALSE == SYS_TIME_DST_SetPredefinedMode(daylight_mode))
            {
                CLI_LIB_PrintStr("Failed to enable summer time.\r\n");
                return CLI_NO_ERROR;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
    /* set name of daylight saving time zone
     */
    if (FALSE == SYS_TIME_DST_SetZoneName(arg[0]))
    {
        CLI_LIB_PrintStr("Failed to set summer time zone name.\r\n");
        return CLI_NO_ERROR;
    }
#endif

    /* get time offset
     */
    if (NULL != arg[11 + arg_offset])
    {
        /* user-defined time offset
         */
        time_offset = CLI_LIB_AtoUll(arg[11 + arg_offset]);
    }
    else
    {
        /* default time offset
         */
        time_offset = SYS_TIME_DEFAULT_DST_TIME_OFFSET;
    }

    /* set time offset
     */
    if (FALSE == SYS_TIME_DST_SetTimeOffset(time_offset))
    {
        CLI_LIB_PrintStr("Failed to set summer offset.\r\n");
    }
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
static UI32_T check_day(char *day)
{
    switch(day[0])
    {
        case 'M':/*monday*/
        case 'm':
            return 1;
        case 'T':/*Tue,Thu*/
        case 't':
            if (day[1] == 'U' || day[1] == 'u')
            {
                return 2;
            }
            else
            {
                return 4;
            }
        case 'W':/*wed*/
        case 'w':
            return 3;
        case 'F':/*fri*/
        case 'f':
            return 5;
        case 'S':/*Sat, Sun*/
        case 's':
            if (day[1] == 'A' || day[1] == 'a')
            {
                return 6;
            }
            else
            {
                return 0;
            }
        default:
            return 255;
    }
    return 255;
}

static void get_order_str(int order, char *order_str)
{
    switch (order)
    {
        case 1:
            strcpy(order_str, "1st");
            break;

        case 2:
            strcpy(order_str, "2nd");
            break;

        case 3:
            strcpy(order_str, "3rd");
            break;

        case 4:
            strcpy(order_str, "4th");
            break;

        case 5:
            strcpy(order_str, "last");
            break;

        default:
            order_str[0] = '\0';
            break;
    }
}

static BOOL_T detect_date_input_time(char *arg[], SYS_TIME_DST *begin_dst, SYS_TIME_DST *end_dst)
{
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
    UI32_T arg_offset = 1; /* offset one argument */
#else
    UI32_T arg_offset = 0; /* no offset */
#endif

    if (isdigit(arg[1 + arg_offset][0]))
    {
        /* date month year hh mm date month year hh mm offset
         */
        begin_dst->day = CLI_LIB_AtoUll(arg[1 + arg_offset]);
        begin_dst->month = check_month(arg[2 + arg_offset]);
        end_dst->day = CLI_LIB_AtoUll(arg[6 + arg_offset]);
        end_dst->month = check_month(arg[7 + arg_offset]);
    }
    else
    {
        /* month data year hh mm month data year hh mm offset
         */
        begin_dst->month = check_month(arg[1 + arg_offset]);
        begin_dst->day = CLI_LIB_AtoUll(arg[2 + arg_offset]);
        end_dst->month = check_month(arg[6 + arg_offset]);
        end_dst->day = CLI_LIB_AtoUll(arg[7 + arg_offset]);
    }
    begin_dst->year = CLI_LIB_AtoUll(arg[3 + arg_offset]);
    begin_dst->hour = CLI_LIB_AtoUll(arg[4 + arg_offset]);
    begin_dst->minute = CLI_LIB_AtoUll(arg[5 + arg_offset]);
    begin_dst->second = 0;
    end_dst->year = CLI_LIB_AtoUll(arg[8 + arg_offset]);
    end_dst->hour = CLI_LIB_AtoUll(arg[9 + arg_offset]);
    end_dst->minute = CLI_LIB_AtoUll(arg[10 + arg_offset]);
    end_dst->second = 0;

    return TRUE;
}

static BOOL_T detect_recurring_input_time(char *arg[], SYS_TIME_RECURRING_DST *begin_dst, SYS_TIME_RECURRING_DST *end_dst)
{
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
    UI32_T arg_offset = 1; /* offset one argument */
#else
    UI32_T arg_offset = 0; /* no offset */
#endif

    begin_dst->week = CLI_LIB_AtoUll(arg[1 + arg_offset]);
    begin_dst->wday = check_day(arg[2 + arg_offset]);
    begin_dst->month = check_month(arg[3 + arg_offset]);
    begin_dst->hour = CLI_LIB_AtoUll(arg[4 + arg_offset]);
    begin_dst->minute = CLI_LIB_AtoUll(arg[5 + arg_offset]);
    end_dst->week = CLI_LIB_AtoUll(arg[6 + arg_offset]);
    end_dst->wday = check_day(arg[7 + arg_offset]);
    end_dst->month = check_month(arg[8 + arg_offset]);
    end_dst->hour = CLI_LIB_AtoUll(arg[9 + arg_offset]);
    end_dst->minute = CLI_LIB_AtoUll(arg[10 + arg_offset]);

    return TRUE;
}
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

#if (SYS_CPNT_SYS_TIME == TRUE)
UI32_T 
CLI_API_Clock_Timezone(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
    SYS_TIME_TIMEZONE_SIGN_T  sign;
    UI32_T  hour = 0, minute = 0;
    char *p;

    if (NULL != arg[2])
    {
        p = arg[2];

        if ('-' == *p)
        {
            sign = SYS_TIME_TIMEZONE_MINUS;
            ++p;
        }
        else if ('+' == *p)
        {
            sign = SYS_TIME_TIMEZONE_PLUS;
            ++p;
        }
        else
        {
            sign = SYS_TIME_TIMEZONE_PLUS;
        }

        hour = atol(p);
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

    if ((arg[3] != NULL) && (arg[3][0] == 'm' || arg[3][0] == 'M'))
    {
        minute = atol(arg[4]);
    }

    if (((arg[3] != NULL) && (arg[3][0] == 'a' || arg[3][0] == 'A')) ||
        ((arg[5] != NULL) && (arg[5][0] == 'a' || arg[5][0] == 'A')))
    {
        if (SYS_TIME_TIMEZONE_MINUS == sign)
        {
           CLI_LIB_PrintStr("Failed to set clock timezone.\r\n");
           return CLI_NO_ERROR;
        }
    }
    else if (((arg[3] != NULL) && (arg[3][0] == 'b' || arg[3][0] == 'B')) ||
             ((arg[5] != NULL) && (arg[5][0] == 'b' || arg[5][0] == 'B')))
    {
        if (SYS_TIME_TIMEZONE_MINUS == sign)
        {
           CLI_LIB_PrintStr("Failed to set clock timezone.\r\n");
           return CLI_NO_ERROR;
        }

        sign = SYS_TIME_TIMEZONE_MINUS;
    }

    if (SYS_TIME_SetTimeZone(arg[0], sign, hour, minute) != TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set clock timezone.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Clock_Timezone_Predefined(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T timezone_id = 0;

    timezone_id = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W2_CLOCK_TIMEZONEPREDEFINED)?
                   parsing_timezone_str(arg) : SYS_TIME_DEFAULT_TIMEZONE_ID;

    if (0 == timezone_id)
    {
        return CLI_ERR_INTERNAL;
    }

    if (TRUE != SYS_TIME_SetTimezonePredefined(timezone_id))
    {
        CLI_LIB_PrintStr("Failed to set time-zone area\r\n");
    }

    return CLI_NO_ERROR;
}

static UI32_T
parsing_timezone_str(
    char *arg[])
{
    if (arg[0][3] == '-')
    {
        switch (arg[0][4])
        {
            case '1': /* -1xxx */
                switch (arg[0][5])
                {
                    case '0':/* -1000 */
                        return VAL_sysTimeZonePredefined_minus1000Hawaii;

                    case '1':/* -1100 */
                        return VAL_sysTimeZonePredefined_minus1100MidwayIslandSamoa;

                    case '2':/* -1200 */
                        return VAL_sysTimeZonePredefined_minus1200InternationalDateLineWest;

                    default:
                        return 0;
                }
                break;

            case '0': /* -0xxx */
                switch (arg[0][5])
                {
                    case '1': /* -01xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_minus0100Azores;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_minus0100CapeVerdeIs;

                            default:
                                return 0;
                        }

                    case '2': /* -02xx */
                        return VAL_sysTimeZonePredefined_minus0200MidAtlantic;

                    case '3': /* -03xx */
                        switch (arg[0][9])
                        {
                            case 'G':
                            case 'g':
                                return VAL_sysTimeZonePredefined_minus0300Greenland;

                            case 'B':
                            case 'b':
                                if (arg[0][10] == 'R' || arg[0][10] == 'r')
                                {
                                    return VAL_sysTimeZonePredefined_minus0300Brasilia;
                                }
                                else
                                {
                                    return VAL_sysTimeZonePredefined_minus0300BuenosAiresGeorgetown;
                                }

                            case 'N':
                            case 'n':
                                return VAL_sysTimeZonePredefined_minus0330Newfoundland;

                            default:
                                return 0;
                        } /* end of switch (arg[0][9]) */

                    case '4': /* -04xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_minus0400AtlanticTimeCanada;

                            case 'C':
                            case 'c':
                               return VAL_sysTimeZonePredefined_minus0400CaracasLaPaz;

                            case 'S':
                            case 's':
                                return VAL_sysTimeZonePredefined_minus0400Santiago;

                            default:
                                return 0;
                        }

                    case '5': /* -05xx */
                        switch (arg[0][9])
                        {
                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_minus0500BogotaLimaQuito;

                            case 'E':
                            case 'e':
                                return VAL_sysTimeZonePredefined_minus0500EasternTimeUSCanada;

                            case 'I':
                            case 'i':
                                return VAL_sysTimeZonePredefined_minus0500IndianaEast;

                            default:
                                return 0;
                        }
                    case '6': /* -06xx */
                        switch (arg[0][9])
                        {
                            case 'S':
                            case 's':
                                return VAL_sysTimeZonePredefined_minus0600Saskatchewan;

                            case 'C':
                            case 'c':
                                if (arg[0][17] == 'A' || arg[0][17] == 'a')
                                {
                                    return VAL_sysTimeZonePredefined_minus0600CentralAmerica;
                                }
                                else
                                {
                                    return VAL_sysTimeZonePredefined_minus0600CentralTimeUSCanada;
                                }

                            case 'G':
                            case 'g':
                                return VAL_sysTimeZonePredefined_minus0600GuadalajaraMexicoCityMonterrey;

                            default:
                                return 0;
                        } /* end of switch (arg[0][9]) */

                    case '7': /* -07xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_minus0700Arizona;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_minus0700ChihuahuaLaPazMazatlan;

                            case 'M':
                            case 'm':
                                return VAL_sysTimeZonePredefined_minus0700MountainTimeUSCanada;

                            default:
                                return 0;
                        }

                    case '8': /* -08xx */
                        return VAL_sysTimeZonePredefined_minus0800PacificTimeTijuana;

                    case '9': /* -09xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_minus0900Alaska;

                            case 'T':
                            case 't':
                                return VAL_sysTimeZonePredefined_minus0930Taiohae;

                            default:
                                return 0;
                        }

                    default:
                        return 0;
                } /* end of switch (arg[0][5]) */
                break;

            case 'G': /* GMT */
            case 'g':
                return VAL_sysTimeZonePredefined_gmtDublinEdinburghLisbonLondon;
                break;

            case 'C': /* GMT */
            case 'c':
                return VAL_sysTimeZonePredefined_gmtCasablancaMonrovia;
                break;

            default:
                return 0;
        } /* end of switch (arg[0][4]) */
    } /* end of if (arg[0][3] == '-') */
    else
    {
        switch (arg[0][4])
        {
            case '1': /* +1xxx */
                switch (arg[0][5])
                {
                    case '0': /* +10xx */
                        switch (arg[0][9])
                        {
                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_plus1000Brisbane;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_plus1000CanberraMelbourneSydney;

                            case 'G':
                            case 'g':
                                return VAL_sysTimeZonePredefined_plus1000GuamPortMoresby;

                            case 'H':
                            case 'h':
                                return VAL_sysTimeZonePredefined_plus1000Hobart;

                            case 'V':
                            case 'v':
                                return VAL_sysTimeZonePredefined_plus1000Vladivostok;

                            case 'L':
                            case 'l':
                                return VAL_sysTimeZonePredefined_plus1030LordHoweIsland;

                            default:
                                return 0;
                        } /* end of switch (arg[0][9]) */

                    case '1': /* +11xx */
                        switch (arg[0][9])
                        {
                            case 'M':
                            case 'm':
                                return VAL_sysTimeZonePredefined_plus1100MagadanSolomonIsNewCaledonia;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus1130Kingston;
                        }

                    case '2': /* +12xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_plus1200AucklandWellington;

                            case 'F':
                            case 'f':
                                return VAL_sysTimeZonePredefined_plus1200FijiKamchatkaMarshallIs;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_plus1245ChathamIsland;

                            default:
                                return 0;
                        } /* end of switch (arg[0][9]) */

                    case '3': /* +13xx */
                        return VAL_sysTimeZonePredefined_plus1300Nukualofa;

                    case '4': /* +14xx */
                        return VAL_sysTimeZonePredefined_plus1400Kiritimati;

                    default:
                        return 0;
                } /* end of switch (arg[0][5]) */
                break;

            case '0': /* +0xxx */
                switch (arg[0][5])
                {
                    case '1': /* +01xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_plus0100AmsterdamBerlinBernRomeStockholmVienna;

                            case 'B':
                            case 'b':
                                if (arg[0][10] == 'E' || arg[0][10] == 'e')
                                {
                                    return VAL_sysTimeZonePredefined_plus0100BelgradeBratislavaBudapestLjubljanaPrague;
                                }
                                else
                                {
                                    return VAL_sysTimeZonePredefined_plus0100BrusselsCopenhagenMadridParis;
                                }

                            case 'S':
                            case 's':
                                return VAL_sysTimeZonePredefined_plus0100SarajevoSkopjeWarsawZagreb;

                            case 'W':
                            case 'w':
                                return VAL_sysTimeZonePredefined_plus0100WestCentralAfrica;

                            default:
                                return 0;
                        } /* end of switch (arg[0][9]) */

                    case '2': /* +02xx */
                       switch (arg[0][9])
                       {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_plus0200AthensBeirutIstanbulMinsk;

                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_plus0200Bucharest;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_plus0200Cairo;

                            case 'H':
                            case 'h':
                                if (arg[0][10] == 'A' || arg[0][10] == 'a')
                                {
                                    return VAL_sysTimeZonePredefined_plus0200HararePretoria;
                                }
                                else
                                {
                                    return VAL_sysTimeZonePredefined_plus0200HelsinkiKyivRigaSofiaTallinnVilnius;
                                }

                            case 'J':
                            case 'j':
                                return VAL_sysTimeZonePredefined_plus0200Jerusalem;

                            default:
                                return 0;
                        }

                    case '3': /* +03xx */
                        switch (arg[0][9])
                        {
                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_plus0300Baghdad;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus0300KuwaitRiyadh;

                            case 'M':
                            case 'm':
                                return VAL_sysTimeZonePredefined_plus0300MoscowStPetersburgVolgograd;

                            case 'N':
                            case 'n':
                                return VAL_sysTimeZonePredefined_plus0300Nairobi;

                            case 'T':
                            case 't':
                                return VAL_sysTimeZonePredefined_plus0330Tehran;

                            default:
                                return 0;
                        }

                    case '4': /* +04xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_plus0400AbuDhabiMuscat;

                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_plus0400BakuTbilisiYerevan;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus0430Kabul;

                            default:
                                return 0;
                        }

                    case '5': /* +05xx */
                        switch (arg[0][9])
                        {
                            case 'E':
                            case 'e':
                                return VAL_sysTimeZonePredefined_plus0500Ekaterinburg;

                            case 'I':
                            case 'i':
                                return VAL_sysTimeZonePredefined_plus0500IslamabadKarachiTashkent;

                            case 'C':
                            case 'c':
                                return VAL_sysTimeZonePredefined_plus0530ChennaiCalcutaMumbaiNewDelhi;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus0545Kathmandu;

                            default:
                                return 0;
                        }

                    case '6': /* +06xx */
                        switch (arg[0][9])
                        {
                            case 'A':
                            case 'a':
                                if (arg[0][17] == 'L' || arg[0][17] == 'l')
                                {
                                    return VAL_sysTimeZonePredefined_plus0600AlmatyNovosibirsk;
                                }
                                else
                                {
                                    return VAL_sysTimeZonePredefined_plus0600AstanaDhaka;
                                }

                            case 'S':
                            case 's':
                                return VAL_sysTimeZonePredefined_plus0600SriJayawardenepura;

                            case 'R':
                            case 'r':
                                return VAL_sysTimeZonePredefined_plus0630Rangoon;

                            default:
                                return 0;
                        }

                    case '7': /* +07xx */
                        switch (arg[0][9])
                        {
                            case 'B':
                            case 'b':
                               return VAL_sysTimeZonePredefined_plus0700BangkokHanoiJakarta;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus0700Krasnoyarsk;

                            default:
                                return 0;
                        }

                    case '8': /* +08xx */
                        switch (arg[0][9])
                        {
                            case 'B':
                            case 'b':
                                return VAL_sysTimeZonePredefined_plus0800BeijingChongqingHongKongUrumqi;

                            case 'I':
                            case 'i':
                                return VAL_sysTimeZonePredefined_plus0800IrkutskUlaanBataar;

                            case 'K':
                            case 'k':
                                return VAL_sysTimeZonePredefined_plus0800KualaLumpurSingapore;

                            case 'P':
                            case 'p':
                                return VAL_sysTimeZonePredefined_plus0800Perth;

                            case 'T':
                            case 't':
                                return VAL_sysTimeZonePredefined_plus0800Taipei;

                            default:
                                return 0;
                        }
                    case '9': /* +09xx */
                        switch (arg[0][9])
                        {
                            case 'O':
                            case 'o':
                                return VAL_sysTimeZonePredefined_plus0900OsakaSapporoTokyo;

                            case 'S':
                            case 's':
                                return VAL_sysTimeZonePredefined_plus0900Seoul;

                            case 'Y':
                            case 'y':
                                return VAL_sysTimeZonePredefined_plus0900Yakutsk;

                            case 'A':
                            case 'a':
                                return VAL_sysTimeZonePredefined_plus0930Adelaide;

                            case 'D':
                            case 'd':
                                return VAL_sysTimeZonePredefined_plus0930Darwin;

                            default:
                                return 0;
                        }

                    default:
                        return 0;
                } /* end of switch (arg[0][5]) */
                break;

            default:
                return 0;
        } /* end of switch (arg[0][4]) */
    } /* end of else */

    return 0;
}
#endif /* #if (SYS_CPNT_SYS_TIME == TRUE) */
