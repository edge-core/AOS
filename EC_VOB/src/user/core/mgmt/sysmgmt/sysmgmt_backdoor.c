/* Module Name: SYS_MGR_BACKDOOR.C
 * Purpose: This file contains the debugging information of stack topology:
 *
 * Notes:
 *
 * History:
 *    07/18/03       -- S.K Yang, Create
 *
 * Copyright(C)      Accton Corporation, 1999-2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "sys_type.h"
#include "sys_hwcfg.h"
//#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "sys_env_vm.h"
#include "sysdrv.h"
#include "sys_cpnt.h"
//#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#include "sys_mgr.h"
#include "sys_time.h"
#include <string.h>
//#endif /* SYS_CPNT_WATCHDOG_TIMER */
#include "backdoor_mgr.h" 
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSMGMT_BACKDOOR_USE_IPC_IO FALSE
/* for software watchdog log
 */
#define UPLOAD_SW_WATCHDOG_LOG_SCRIPT_FILE "/etc/upload_sw_wtd_log.sh"
#define SW_WATCHDOG_LOG_DIRNAME "/flash/sw_wtd_log"
#define SW_WATCHDOG_LOG_FILENAME_PREFIX "sw_wtd_log"


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#if ( SYSMGMT_BACKDOOR_USE_IPC_IO == FALSE )
#define SYSMGMT_BACKDOOR_GetLine(buf, size) BACKDOOR_MGR_RequestKeyIn((buf), (size))
#define SYSMGMT_BACKDOOR_Printf(fmt_p, ...) BACKDOOR_MGR_Printf((fmt_p), ##__VA_ARGS__)
#else
#define SYSMGMT_BACKDOOR_Printf(fmt_p, ...) printf((fmt_p), ##__VA_ARGS__)
#endif


/* FUNCTION NAME: SYSMGMT_BACKDOOR_GetLine
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */

#if ( SYSMGMT_BACKDOOR_USE_IPC_IO == TRUE)
int SYSMGMT_BACKDOOR_GetLine(char s[], int lim)
{
    int c, i;

    for (i=0; i<lim-1 && (c=getchar()) != 0 && c!='\n'; ++i)
    {
        s[i] = c;
        printf("%c", c);
    }
    if (c == '\n')
    {
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return i;
} /* End of SYSMGMT_BACKDOOR_GetLine */
#endif  /* SYSMGMT_BACKDOOR_USE_IPC_IO == FALSE */

/* FUNCTION NAME: SYS_BACKDOOR_Main
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void SYS_BACKDOOR_Main (void)
{
#define MAXLINE 255
char line_buffer[MAXLINE];
int  select_value = 0;

    while(1)
    {
        SYSMGMT_BACKDOOR_Printf("\r\nPress <enter> to continue.");
        SYSMGMT_BACKDOOR_GetLine(line_buffer, 255);

        SYSMGMT_BACKDOOR_Printf("\r\n===========================================");
        SYSMGMT_BACKDOOR_Printf("\r\n  SYS_MGR Engineer Menu 2001/10/26  ");
        SYSMGMT_BACKDOOR_Printf("\r\n===========================================");

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        SYSMGMT_BACKDOOR_Printf("\r\n [1] Show all monitored threads");
        SYSMGMT_BACKDOOR_Printf("\r\n [2] Set SW watchdog debug message level(%hu)", SW_WATCHDOG_MGR_GetDebugMsgLevel());
        SYSMGMT_BACKDOOR_Printf("\r\n [3] Set SW watchdog stop reboot status(%hu)", SW_WATCHDOG_MGR_GetStopRebootStatus());
        SYSMGMT_BACKDOOR_Printf("\r\n [4] Register SW watchdog monitored thread");
        SYSMGMT_BACKDOOR_Printf("\r\n [5] Only For TEST: kill one monitored thread");
        SYSMGMT_BACKDOOR_Printf("\r\n [6] Upload sw watchdog log file by TFTP");
#endif
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
        SYSMGMT_BACKDOOR_Printf("\r\n [11] Enable Watch Dog.");
        SYSMGMT_BACKDOOR_Printf("\r\n [12] Disable Watch Dog.");
        SYSMGMT_BACKDOOR_Printf("\r\n [13] Kick Off Watch Dog (Must Enable Watch Dog First).");
        SYSMGMT_BACKDOOR_Printf("\r\n [14] Kick On Watch Dog (Must Enable Watch Dog First).");
        SYSMGMT_BACKDOOR_Printf("\r\n [15] Kick Counter.");
        SYSMGMT_BACKDOOR_Printf("\r\n [16] Watch Dog Test Task Create.");
        SYSMGMT_BACKDOOR_Printf("\r\n [17] Show WDT database. ");
#endif /* SYS_CPNT_WATCHDOG_TIMER */
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        SYSMGMT_BACKDOOR_Printf("\r\n [18] Get temp.");
        SYSMGMT_BACKDOOR_Printf("\r\n [19] Get fan speed.");
#endif
        SYSMGMT_BACKDOOR_Printf("\r\n [20] Enable sys_env_vm debug mode.");
        SYSMGMT_BACKDOOR_Printf("\r\n [21] Disable sys_env_vm debug mode.");
#if ((SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE))
        SYSMGMT_BACKDOOR_Printf("\r\n [22] I2C Channel test.");
        SYSMGMT_BACKDOOR_Printf("\r\n [23] I2C Channel test.(Infinite mode)");
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        SYSMGMT_BACKDOOR_Printf("\r\n [24] Set fan speed.");
#endif
        SYSMGMT_BACKDOOR_Printf("\r\n [25] Set sys_mgr debug level");

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
        {
            UI32_T old_value;
            BOOL_T status;
            SYS_MGR_GetCpuGuardHighWatermark(&old_value);
            SYSMGMT_BACKDOOR_Printf("\r\n [30] CPU guard - Set high watermark.(%lu%%)", (unsigned long)old_value);
            SYS_MGR_GetCpuGuardLowWatermark(&old_value);
            SYSMGMT_BACKDOOR_Printf("\r\n [31] CPU guard - Set low watermark.(%lu%%)", (unsigned long)old_value);
            SYS_MGR_GetCpuGuardMaxThreshold(&old_value);
            SYSMGMT_BACKDOOR_Printf("\r\n [32] CPU guard - Set max threshold.(%lupps)", (unsigned long)old_value);
            SYS_MGR_GetCpuGuardMinThreshold(&old_value);
            SYSMGMT_BACKDOOR_Printf("\r\n [33] CPU guard - Set min threshold.(%lupps)", (unsigned long)old_value);
            SYSMGMT_BACKDOOR_Printf("\r\n [34] CPU guard - Show debug messages.(%c)", SYS_MGR_GetCpuGuardDebugFlag() ? 'Y' : 'N');
            SYS_MGR_GetCpuGuardStatus(&status);
            SYSMGMT_BACKDOOR_Printf("\r\n [35] CPU guard - Turn ON/OFF.(%s)", status ? "ON" : "OFF");
        }
#endif

        SYSMGMT_BACKDOOR_Printf("\r\n [99] Exit Stack Topology Engineer Menu!!");
        SYSMGMT_BACKDOOR_Printf("\r\n Enter Selection: ");

        if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
            SYSMGMT_BACKDOOR_Printf("\r\nSelect value is %d", select_value); /* Debug message */
        }

        switch(select_value)
        {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            case 1:
                SW_WATCHDOG_MGR_DumpMonitoredThread();
                break;
            case 2:
                SYSMGMT_BACKDOOR_Printf("\r\nDebug message level(0-3, 0:No debug msg, 3:Verbose debug msg):");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    select_value = atoi(line_buffer);
                    SW_WATCHDOG_MGR_SetDebugMsgLevel(select_value);
                    SYSMGMT_BACKDOOR_Printf("\r\n");
                }
                break;
            case 3:
                SYSMGMT_BACKDOOR_Printf("\r\nStop Reboot State:(0:Reboot when timeout, 1:No reboot when timeout):");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    select_value = atoi(line_buffer);
                    SW_WATCHDOG_MGR_SetStopRebootStatus((select_value)?TRUE:FALSE);
                    SYSMGMT_BACKDOOR_Printf("\r\n");
                }
                break;
            case 4:
            {
                UI32_T monitor_id=0,thread_id=0,expired_time=0,is_save=0;

                SYSMGMT_BACKDOOR_Printf("\r\nInput the monitor id : ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    monitor_id = atoi(line_buffer);
                }

                SYSMGMT_BACKDOOR_Printf("\r\nInput the thread id : ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    thread_id = atoi(line_buffer);
                }

                SYSMGMT_BACKDOOR_Printf("\r\nInput the expired timer: ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    expired_time = atoi(line_buffer);
                }

                SYSMGMT_BACKDOOR_Printf("\r\n Save [1: yes or 2: no]: ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    is_save = atoi(line_buffer);
                }
                if (is_save == 1)
                {
                    SW_WATCHDOG_MGR_RegisterMonitorThread(monitor_id, thread_id, expired_time);
                }
            }
                break;

            case 5:
            {
                UI32_T thread_id=0,is_kill=0;
                char  system_cmd[10]={0};
                SYSMGMT_BACKDOOR_Printf("\r\nInput the thread id : ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    thread_id = atoi(line_buffer);
                    if (thread_id > 1000)
                    {
                        SYSMGMT_BACKDOOR_Printf("\r\nThread id is out of range");
                        break;
                    }
                }
                SYSMGMT_BACKDOOR_Printf("\r\n Kill thread %lu  [1: yes or 2: no]: ",(unsigned long)thread_id);
				
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    is_kill = atoi(line_buffer);
                }
                if (is_kill == 1)
                {
                    sprintf(system_cmd,"kill %lu",(unsigned long)thread_id);
                    system(system_cmd);
                }
            }
                break;
            case 6:
            {
                struct stat file_stat;
                int    rc;
                char   ip_buffer[28];
                char   sys_buffer[64];
                DIR   *dir;
                struct dirent *dirent_p;
                BOOL_T is_found;

                rc = stat(UPLOAD_SW_WATCHDOG_LOG_SCRIPT_FILE, &file_stat);
                if(rc!=0)
                {
                    SYSMGMT_BACKDOOR_Printf("Do not support yet.\r\n");
                    break;
                }

                /* check whether software watchdog log exists
                 */
                is_found=FALSE;
                dir = opendir(SW_WATCHDOG_LOG_DIRNAME);
                if(dir==NULL)
                {
                    goto SW_WATCHDOG_LOG_DIRECTORY_NOT_EXIST;
                }

                while((dirent_p = readdir(dir))!=NULL)
                {
                    if(!strncmp(dirent_p->d_name, SW_WATCHDOG_LOG_FILENAME_PREFIX, sizeof(SW_WATCHDOG_LOG_FILENAME_PREFIX)-1))
                    {
                        is_found=TRUE;
                        break;
                    }
                }

                closedir(dir);

SW_WATCHDOG_LOG_DIRECTORY_NOT_EXIST:
                /* do upload software watchdog log files if they exist
                 */
                if(is_found==TRUE)
                {
                    SYSMGMT_BACKDOOR_Printf("Please ensure that IP of this device had been configured properly\r\n");
                    SYSMGMT_BACKDOOR_Printf("TFTP server ip: ");
                    SYSMGMT_BACKDOOR_GetLine(ip_buffer, sizeof(ip_buffer)-1);
                    SYSMGMT_BACKDOOR_Printf("\r\n");
                    sprintf(sys_buffer, "%s %s", UPLOAD_SW_WATCHDOG_LOG_SCRIPT_FILE, ip_buffer);
                    system(sys_buffer);
                    SYSMGMT_BACKDOOR_Printf("Done.\r\n");
                }
                else
                    SYSMGMT_BACKDOOR_Printf("No software watchdog log on the device\r\n");

            }
                break;
#endif /* #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) */

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            case 11:
                SYS_TIME_EnableWatchDogTimer();
                break;
            case 12:
                SYS_TIME_DisableWatchDogTimer();
                break;
            case 13:
                SYS_TIME_KickWatchDogTimer();
                SYS_TIME_SetKick(FALSE);
                break;
            case 14:
                SYS_TIME_SetKick(TRUE);
                SYS_TIME_KickWatchDogTimer();
                break;
            case 15:
                {
                    UI32_T counter;
                    SYS_TIME_GetKickCounter(&counter);
                    SYSMGMT_BACKDOOR_Printf("\r\n");
                    SYSMGMT_BACKDOOR_Printf("\nWatch Dog Kick   = [%ld]\r\n", (long)counter);

                }
                break;
            case 16:
                {
                    #define WATCH_DOG_TEST_STRING   "Watch Dog Test Task"
                    SYS_MGR_WatchDogExceptionInfo_T wd_own;

                    memset(&wd_own, 0, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
                    memcpy(wd_own.name, WATCH_DOG_TEST_STRING, sizeof(WATCH_DOG_TEST_STRING));
                    wd_own.pStackBase  = 1;   /* points to bottom of stack */
                    wd_own.pStackLimit = 2;   /* points to stack limit */
                    wd_own.pStackEnd   = 3;   /* points to init stack limit */

                    wd_own.priority    = 4;   /* task priority       */
                    wd_own.status      = 5;   /* task status         */
                    wd_own.delay       = 6;   /* delay/timeout ticks */

                    wd_own.stackSize    = 7;  /* size of stack in bytes */
                    wd_own.stackCurrent = 8;  /* current stack usage in bytes */
                    wd_own.stackMargin  = 9;  /*  used as Program Counter */

                    if (SYS_MGR_LogWatchDogExceptionInfo(&wd_own) == TRUE)
                        SYSMGMT_BACKDOOR_Printf("\n\nWatch Dog Log Success ==> Warm Start System\n\n");
                }

                break;
            case 17:
                SYS_MGR_ShowWatchDogTimerLogInfoFromLogFile();
                break;
#endif /* SYS_CPNT_WATCHDOG_TIMER */
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
            case 18:
                {
                    SYS_MGR_SwitchThermalEntry_T switch_thermal_status;
                    SYSMGMT_BACKDOOR_Printf("\r\nInput unit : ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        switch_thermal_status.switch_unit_index = atoi(line_buffer);
                    }
                    SYSMGMT_BACKDOOR_Printf("\r\nInput thermal : ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        switch_thermal_status.switch_thermal_index = atoi(line_buffer);
                    }
                    SYS_MGR_GetThermalStatus(&switch_thermal_status);
                    SYSMGMT_BACKDOOR_Printf("\r\n thermal status temp : %ld", (long)switch_thermal_status.switch_thermal_temp_value);
                }
                break;
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
            case 19:
                {
                    SYS_MGR_SwitchFanEntry_T switch_fan_status;

                    SYSMGMT_BACKDOOR_Printf("\r\nInput unit : ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        switch_fan_status.switch_unit_index = atoi(line_buffer);
                    }
                    SYSMGMT_BACKDOOR_Printf("\r\nInput fan : ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        switch_fan_status.switch_fan_index = atoi(line_buffer);
                    }
                    SYS_MGR_GetFanStatus(&switch_fan_status);
                    SYSMGMT_BACKDOOR_Printf("\r\n Fan status : %ld", (long)switch_fan_status.switch_fan_status);
                    SYSMGMT_BACKDOOR_Printf("\r\n Fan fail counter : %ld", (long)switch_fan_status.switch_fan_fail_counter);
                    SYSMGMT_BACKDOOR_Printf("\r\n Fan Admin speed : %ld", (long)switch_fan_status.switch_fan_speed);
                    SYSMGMT_BACKDOOR_Printf("\r\n Fan Oper speed : %ld", (long)switch_fan_status.switch_fan_oper_speed);
                }
                break;
#endif
            case 20:
                SYS_ENV_VM_SetDebugMode(TRUE);
                SYSMGMT_BACKDOOR_Printf("\r\nEnable sys_env_vm debug mode success");
                break;
            case 21:
                SYS_ENV_VM_SetDebugMode(FALSE);
                SYSMGMT_BACKDOOR_Printf("\r\nDisable sys_env_vm debug mode success");
                break;
#if ((SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE))
            case 22:
            {
                UI32_T polling_interval = 0;
                UI8_T ch;
                UI32_T test_cnt;
                UI32_T fail_cnt;
                UI32_T total_fail;
                UI32_T begin_tick;
                UI32_T end_tick;

                test_cnt=0;
                fail_cnt=0;
                total_fail=0;
                begin_tick=0;
                end_tick=0;

                begin_tick = SYS_TIME_GetSystemTicksBy10ms();

                SYSMGMT_BACKDOOR_Printf("\r\nInput polling interval in msec : ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    polling_interval = atoi(line_buffer);
                }
                SYSMGMT_BACKDOOR_Printf(" \r\nBegin testing I2C channel with interval %ld msec, press Enter to quit.\n\r", (long)polling_interval);
                while (1)
                {
                    fail_cnt = 0;

                    total_fail= fail_cnt+total_fail;
                    test_cnt++;
                    /*to see if quit the for loop*/
                    ch = BACKDOOR_MGR_GetChar();

                    if(ch == '\n')
                    {
                        end_tick = SYS_TIME_GetSystemTicksBy10ms();
                        SYSMGMT_BACKDOOR_Printf("\r\nTesting period: %ld seconds, testing %ld times, Fail %ld times\r\n", (long)(end_tick-begin_tick)/100, (long)test_cnt, (long)total_fail);
                        SYSMGMT_BACKDOOR_Printf("Stop test! \r\n");
                        break;
                    }

                }
            }
            break;
            case 23:
            {
                UI32_T polling_interval = 0;

                SYSMGMT_BACKDOOR_Printf("\r\nInput polling interval in msec : ");
                if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                {
                    polling_interval = atoi(line_buffer);
                }
                SYSMGMT_BACKDOOR_Printf(" \r\nBegin testing I2C channel with interval %ld msec.Infinite mode.\n\r", (long)polling_interval);
//                while (1)
//                {
//                    SYSDRV_TestI2CChannel(polling_interval);
//                }
            }
            break;
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && \
    (((SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) && (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP != 0)) || (SYS_HWCFG_FAN_CONTROLLER_TYPE != SYS_HWCFG_FAN_NONE))
            case 24:
                {
                    UI32_T fan=1, speed=100;
                    char shell_cmd[40];
                    BOOL_T set_result=TRUE;

                    SYSMGMT_BACKDOOR_Printf("\r\nInput fan index: ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        fan = atoi(line_buffer);
                    }
                    SYSMGMT_BACKDOOR_Printf("\r\nInput speed(0~100) : ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        speed = atoi(line_buffer);
                    }
#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
                    if(FALSE == SYSDRV_FAN_SetSpeed(fan, speed))
                        set_result=FALSE;
#else
                    snprintf(shell_cmd, sizeof(shell_cmd), "/initrd/usr/bin/onlp_util -f%d.%d", (int)fan, (int)speed);
                    if (SYSFUN_ExecuteSystemShell(shell_cmd)!=SYSFUN_OK)
                    {
                        set_result=FALSE;
                    }
#endif

                    if (set_result==TRUE)
                        SYSMGMT_BACKDOOR_Printf("\r\n set Fan %ld speed successfully.", (long)fan);
                    else
                        SYSMGMT_BACKDOOR_Printf("\r\n set Fan %ld speed fail.", (long)fan);

                }
                break;
#endif

            case 25:
                while (1)
                {
                    char *dbg_func_str[SYS_MGR_DBG_F_MAX] = {
                        [SYS_MGR_DBG_F_CPU_UTIL] = "cpu_util",
                    };

                    SYS_MGR_Debug_Func_T dbg_func;
                    SYS_MGR_Debug_Level_T dbg_level;

                    int i;

                    for (i = 0; i < SYS_MGR_DBG_F_MAX; i++)
                    {
                        SYSMGMT_BACKDOOR_Printf("\r\n %d: %-8.8s: %d",
                            i,
                            dbg_func_str[i] ? dbg_func_str[i] : "unknown",
                            SYS_MGR_GetDebugLevel(i));
                    }

                    SYSMGMT_BACKDOOR_Printf("\r\n x: exit");
                    SYSMGMT_BACKDOOR_Printf("\r\n select = ");
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) == 0)
                    {
                        continue;
                    }

                    if (tolower(line_buffer[0]) == 'x')
                    {
                        break;
                    }

                    dbg_func = atoi(line_buffer);

                    if (dbg_func < SYS_MGR_DBG_F_MAX)
                    {
                        SYSMGMT_BACKDOOR_Printf("\r\n debug level = ");
                        SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE);
                        dbg_level = atoi(line_buffer);

                        SYS_MGR_SetDebugLevel(dbg_func, dbg_level);
                    }
                }
                break;

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
            case 30:
                {
                    UI32_T old_value = 0;
                    UI32_T new_value = 0;

                    SYS_MGR_GetCpuGuardHighWatermark(&old_value);

                    SYSMGMT_BACKDOOR_Printf("\r\nNew high watermark (%lu): ", (unsigned long)old_value);
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        new_value = atoi(line_buffer);
                    }

                    if (new_value != 0)
                    {
                        SYS_MGR_SetCpuGuardHighWatermark(new_value);
                    }
                }
                break;

            case 31:
                {
                    UI32_T old_value = 0;
                    UI32_T new_value = 0;

                    SYS_MGR_GetCpuGuardLowWatermark(&old_value);

                    SYSMGMT_BACKDOOR_Printf("\r\nNew low watermark (%lu): ", (unsigned long)old_value);
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        new_value = atoi(line_buffer);
                    }

                    if (new_value != 0)
                    {
                        SYS_MGR_SetCpuGuardLowWatermark(new_value);
                    }
                }
                break;

            case 32:
                {
                    UI32_T old_value = 0;
                    UI32_T new_value = 0;

                    SYS_MGR_GetCpuGuardMaxThreshold(&old_value);

                    SYSMGMT_BACKDOOR_Printf("\r\nNew max threshold (%lu): ", (unsigned long)old_value);
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        new_value = atoi(line_buffer);
                    }

                    if (new_value != 0)
                    {
                        SYS_MGR_SetCpuGuardMaxThreshold(new_value);
                    }
                }
                break;

            case 33:
                {
                    UI32_T old_value = 0;
                    UI32_T new_value = 0;

                    SYS_MGR_GetCpuGuardMinThreshold(&old_value);

                    SYSMGMT_BACKDOOR_Printf("\r\nNew min threshold (%lu): " , (unsigned long)old_value);
                    if (SYSMGMT_BACKDOOR_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        new_value = atoi(line_buffer);
                    }

                    if (new_value != 0)
                    {
                        SYS_MGR_SetCpuGuardMinThreshold(new_value);
                    }
                }
                break;

            case 34:
                {
                    SYS_MGR_ToggleCpuGuardDebugFlag();
                }
                break;

            case 35:
                {
                    BOOL_T status;
                    SYS_MGR_GetCpuGuardStatus(&status);
                    SYS_MGR_SetCpuGuardStatus(status ? FALSE : TRUE);
                }
                break;

#endif

            case 99:
                SYSMGMT_BACKDOOR_Printf("\r\n Exit Stack Topology Engineer Menu");
                return;
        }
    }

} /* End of SYS_BACKDOOR_Main */






