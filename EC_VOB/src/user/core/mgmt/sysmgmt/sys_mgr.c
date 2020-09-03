/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_MGR.C
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the all system
 *          wide parameters.
 *
 * Note: 1. For SYS_MGMT_Getxxx routine, caller need to provide the storing
 *          Buffer for display string type.
 *          Caller will also get the string length information as well as
 *          byte array charaters.
 *       2. For SYS_MGMT_Setxxx routine, caller need to provide the length
 *          information for the display string input
 *       3. The naming constant defined in this package shall be reused by
 *          all the BNBU L2/L3 switch projects.
 *       4. This package shall be reusable for all all the BNBU L2/L3 switch projects.
 *
 *
 *
 *  History
 *
 *   Jason Hsue     10/30/2001      new created
 *   Jason Hsue     08/21/2002
 *      Added a ifdef root_swdrv for FTTH project in reload, since FTTH agent board
 *      doesn't need switch shut down at all.
 *   Jason Hsue     10/25/2002
 *      Add prompt string get/set/getrunning 3 APIs
 *   Charlie Chen   09/26/2005      add cursor_of_5sec_entry and cursor_of_minute_entry
 *                                  in SYS_MGR_CpuRecord_T.
 *   Charlie Chen   09/26/2005      add function SYS_MGR_CpuUsage_Callback for
 *                                  new SYSFUN cpu usage callback.
 *   Charlie Chen   09/26/2005      add static variable "whole_tasks_stat"
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "leaf_sys.h"
#include "sysfun.h"
#include "sys_mgr.h"
#include "uc_mgr.h"
#include "stktplg_om.h"
#include "stktplg_pom.h"

//#include "ioLib.h"
//#include "sioLib.h"       /* This is defined in VxWorks   need use for UART : SIO_BAUD_SET*/

/*#include "cacheLib.h"*/   /* This is defined in VxWorks   */
/*#include "rebootLib.h"*/
#include "swctrl.h"
#include "sysdrv.h"
#include "sys_env_vm.h"
#include "leaf_es3626a.h"
#include "l_cvrt.h"

#include "backdoor_mgr.h"

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#include "stkctrl_pmgr.h"
#endif

/*Build2 no this componment*/
//#include "cfgdb_mgr.h"

#if (SYS_CPNT_DBSYNC_TXT ==TRUE)
#include "dbsync_txt_mgr.h"
#endif

#if (SYS_CPNT_3COM_PASSWORD_RECOVERY == TRUE)
#include "userauth.h"
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
#include "l_mm.h"
#include "l_ipcmem.h"
#include "sys_module.h"
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
#include "led_pmgr.h"
#endif

/* UI message */
#include "sys_module.h"
//#include "syslog_type.h"
//#include "eh_type.h"
//#include "eh_mgr.h"

#include "sys_time.h"

/* add by S.K.Yang for autobaudrate */
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
/*it is no use*/
//#include "cli_mgr.h"
/* #include "sysSerial.h" */
#include "sys_dflt.h"
#endif

#include "sysmgmt_backdoor.h"
#include "sys_bld.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
#include "trap_mgr.h"
#include "snmp_pmgr.h"
#endif /* #if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
#include "sys_reload_mgr.h"
#endif

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
#include "dev_swdrv_pmgr.h"
#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#include "fs.h"
#include "fs_type.h"
#endif

#if (SYS_CPNT_CONFIG_AGENT == TRUE)
#include "config_agent.h"
#endif

#define SYS_MGR_DBG_Printf(func, lvl, fmt, ...) do { \
        if (sys_mgr_func_dbg_lvl[(func)] >= (lvl)) \
            BACKDOOR_MGR_Printf("[%s]%s:%d:" fmt, sys_mgr_dbg_lvl_str[(lvl)], __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)


typedef struct {
    char sys_oid[SYS_ADPT_MAX_OID_STRING_LEN + 1];
    char private_mib_root[SYS_ADPT_MAX_OID_STRING_LEN + 1];
    char sys_descr[MAXSIZE_sysDescr + 1];
    char product_name[MAXSIZE_swProdName + 1];
    char model_name[SYS_ADPT_MAX_MODEL_NAME_SIZE + 1];
} SYS_MGR_BoardInfo_T;

typedef struct {
    char product_manufacturer[MAXSIZE_swProdManufacturer + 1];
    char product_description[MAXSIZE_swProdDescription + 1];

    SYS_MGR_BoardInfo_T board_info[8];
} SYS_MGR_SwitchInfo_T;

#define sysmgr_board_count (sizeof(sysmgr_swinfo.board_info)/sizeof(sysmgr_swinfo.board_info[0]))

static SYS_MGR_SwitchInfo_T sysmgr_swinfo;

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#define SYS_MGR_WATCHDOG_TIMER_FILE                       "$watch_dog_log"
#define SYS_MGR_WD_TIMER_EXC_INFO_LENGTH                  76 /* Size of SYS_MGR_WatchDog_Exc_Info_S    */
#define SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH               8  /* Size of SYS_MGR_WDLogfileHeader_S      */
#define SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE           SYS_ADPT_MAX_NBR_OF_WATCH_DOG_LOG_INFO
#define SYS_MGR_WD_LOGFILE_SIZE                           ((SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE * SYS_MGR_WD_TIMER_EXC_INFO_LENGTH) + SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH)
#define SYS_MGR_MAX_CAPABILITY_WDTIMER_MESSAGE            (SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE * SYS_MGR_WD_TIMER_EXC_INFO_LENGTH)

typedef struct SYS_MGR_WDLogfileHeader_S
{
    UI32_T  count;
    UI32_T  sequence_no;
} SYS_MGR_WDLogfileHeader_T;

typedef struct SYS_MGR_WDLogfileDb_S
{
    SYS_MGR_WDLogfileHeader_T  header;
    UI8_T                      message[SYS_MGR_MAX_CAPABILITY_WDTIMER_MESSAGE];
}  SYS_MGR_WDLogfileDb_T;

static BOOL_T SYS_MGR_LogExcInfoToLogFile(void);
static BOOL_T SYS_MGR_CreateWDLogfileInFileSystem(void);
static BOOL_T SYS_MGR_ExistWDLogfileInFileSystem(void);
static void SYS_MGR_DisplayLogInfo(SYS_MGR_WatchDogExceptionInfo_T *show_info);
static SYS_MGR_WatchDogExceptionInfo_T local_tcb;
static SYS_MGR_WDLogfileHeader_T   local_header;
#endif /* SYS_CPNT_WATCHDOG_TIMER */

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
typedef struct
{
    UI32_T rising_threshold;
    UI32_T falling_threshold;
    UI32_T alarm_start_time;
    UI32_T alarm_end_time;
    UI32_T alarm_start_tick;    /* used for calculation of alarm duration time */
    UI32_T alarm_end_tick;      /* used for calculation of alarm duration time */
    BOOL_T alarm_status;
} SYS_MGR_UtilizationAlarm_T;

static BOOL_T sys_mgr_debug_alarm = FALSE;
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
#define SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY      12
#define SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY       5

typedef struct
{
    UI32_T record_tick;
    UI32_T cpu_usage;
    UI32_T cpu_usage_float;
} SYS_MGR_CpuUtilizationRecord_T;

static SYS_MGR_CpuUtilizationRecord_T       sys_mgr_cpu_util_5sec_record[SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY];
static UI32_T sys_mgr_cpu_util_5sec_record_valid_count;
static UI32_T current_5sec_index = 0;
static SYS_MGR_UtilizationAlarm_T           sys_mgr_cpu_alarm;

static SYS_MGR_CpuUtilizationRecord_T       sys_mgr_cpu_util_record[SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY];
static UI32_T sys_mgr_cpu_util_record_valid_count;
static UI32_T current_index = 0;

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
typedef struct
{
    UI32_T cpu_rate;
    UI32_T trap_event;
} SYS_MGR_CpuGuardStatus_T;

typedef struct
{
    UI32_T watermark_high;
    UI32_T watermark_low;
    UI32_T threshold_max;
    UI32_T threshold_min;
    BOOL_T status;
    BOOL_T trap_status;
} SYS_MGR_CpuGuardConfig_T;

typedef struct
{
    SYS_MGR_CpuGuardStatus_T status;
    SYS_MGR_CpuGuardConfig_T config;
    BOOL_T                     debug_flag;
} SYS_MGR_CpuGuard_T;

static SYS_MGR_CpuGuard_T cpu_guard;
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
#define SYS_MGR_UNNAMED_TASK_CPU_UTIL_ENTRY__TASK_NAME          "SYSTEM"
#define SYS_MGR_UNNAMED_TASK_CPU_UTIL_ENTRY__TASK_ID            0

typedef struct
{
    SYS_MGR_CpuUtilizationRecord_T cpu_util_record[SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY];
    UI32_T cpu_util_record_valid_count;
    UI32_T current_5sec_index;
    UI32_T task_id;
    UI32_T aggregate_task_id;
    UI32_T task_user_ticks;
    UI32_T task_sys_ticks;
    char task_name[SYS_MGR_CPU_UTIL_TASK_NAME_MAX_LEN+1];

    /* internal used by SYS_MGR_UpdateTaskCpuUtil
     */
    UI32_T delta_task_user_ticks;
    UI32_T delta_task_sys_ticks;
    UI32_T delta_update_timestamp;
    UI32_T create_timestamp;
} SYS_MGR_TaskCpuUtilEntry_T;

static SYS_MGR_TaskCpuUtilEntry_T sys_mgr_task_cpu_util_entry_buf[SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM];
static L_PT_Descriptor_T sys_mgr_task_cpu_util_entry_buf_desc;
static L_SORT_LST_List_T sys_mgr_task_cpu_util_entry_id_lst;
static L_SORT_LST_List_T sys_mgr_task_cpu_util_entry_name_lst;

static int SYS_MGR_CompareTaskCpuUtilEntryById(void *e1, void *e2);
static int SYS_MGR_CompareTaskCpuUtilEntryByName(void *e1, void *e2);
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_CreateTaskCpuUtilEntry(UI32_T task_id);
static void SYS_MGR_DestroyCpuUtilEntry(SYS_MGR_TaskCpuUtilEntry_T *entry_p);
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_GetTaskCpuUtilEntryPtrById(UI32_T task_id, BOOL_T get_next);
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_GetTaskCpuUtilEntryPtrByName(char *task_name, BOOL_T get_next);
static void SYS_MGR_UpdateTaskCpuUtil(UI32_T delta_idle_ticks);
static BOOL_T SYS_MGR_FillTaskCpuUtilInfo(
    SYS_MGR_TaskCpuUtilEntry_T *entry_p,
    SYS_MGR_TaskCpuUtilInfo_T *cpu_util_info_p);
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE) */
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
typedef struct
{
    MEM_SIZE_T free_bytes;
    MEM_SIZE_T free_blocks;
    MEM_SIZE_T free_max_block_size;
    MEM_SIZE_T used_bytes;
    MEM_SIZE_T used_blocks;
} SYS_MGR_MemoryUtilizationRecord_T;

static SYS_MGR_MemoryUtilizationRecord_T    sys_mgr_mem_util_record;
static SYS_MGR_UtilizationAlarm_T           sys_mgr_mem_alarm;
#endif

#define SYS_MGR_NO_BOARD_ID                               0xffffffff


#define  SYS_MGR_ENTER_CRITICAL_SECTION(sem_id)  SYSFUN_ENTER_CRITICAL_SECTION(sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define  SYS_MGR_LEAVE_CRITICAL_SECTION(sem_id)  SYSFUN_LEAVE_CRITICAL_SECTION(sem_id)
/* macro functions for cpu usage index manipulation
 */
#define GET_NEXT_IDX(cursor, size_of_array) (cursor+1)%(size_of_array)
#define GET_PREV_IDX(cursor, size_of_array) (cursor+size_of_array-1)%(size_of_array)
#define GET_NEXT_SYSMGR_CPUREC_5SEC_IDX(idx) GET_NEXT_IDX(idx, SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY)
#define GET_PREV_SYSMGR_CPUREC_5SEC_IDX(idx) GET_PREV_IDX(idx, SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY)
#define GET_NEXT_SYSMGR_CPUREC_MIN_IDX(idx) GET_NEXT_IDX(idx, SYS_MGR_MAX_NBR_OF_ONE_MINUTE_ENTRY)
#define GET_PREV_SYSMGR_CPUREC_MIN_IDX(idx) GET_PREV_IDX(idx, SYS_MGR_MAX_NBR_OF_ONE_MINUTE_ENTRY)

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* callback list of upper layer component when fan status changed
 */
static SYS_TYPE_CallBack_T *FanStatusChanged_callbacklist;
static SYS_TYPE_CallBack_T *FanSpeedChanged_callbacklist;

/* callback upper layer component when fan status changed
 */
static void SYS_MGR_Notify_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status);


/* register to SYSDRV when fan status changed
 */
BOOL_T  SYS_MGR_FanSpeedChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T speed);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* callback list of upper layer component when fan status changed
 */
static SYS_TYPE_CallBack_T *ThermalStatusChanged_callbacklist;

/* callback upper layer component when thermal status changed
 */
/* comment out the code because no upper layer CSC to notify now
static void SYS_MGR_Notify_ThermalStatusChanged(UI32_T unit, UI32_T thermal_idx, I32_T temperature);
*/

/* register to SYSDRV when fan status changed
 */
BOOL_T  SYS_MGR_ThermalStatusChanged_CallBack(UI32_T unit, UI32_T thermal, I32_T status);
/*static void   SYS_MGR_BackdoorCallBack(void);*/
/*static int SYS_MGR_GetLine(char   s[], int lim);*/
#endif
/*static SYS_TYPE_CallBack_T *XFPModuleStatusChanged_callbacklist;*/
/*static void   SYS_MGR_XFPModuleStatusChanged_CallBack(UI32_T unit, UI32_T port, BOOL_T status);*/

static UI8_T sys_mgr_func_dbg_lvl[SYS_MGR_DBG_F_MAX];
static char *sys_mgr_dbg_lvl_str[SYS_MGR_DBG_L_MAX] = {
    [SYS_MGR_DBG_L_CRIT]    = "CRIT",
    [SYS_MGR_DBG_L_ERR]     = "ERR",
    [SYS_MGR_DBG_L_WARN]    = "WARN",
    [SYS_MGR_DBG_L_VERB]    = "VERB",
    [SYS_MGR_DBG_L_TRACE]   = "TRACE",
};

static UI32_T                   sys_mgr_sem_id;

static UI32_T                   sys_mgr_num_of_unit;
static SYS_MGR_Console_T        sys_mgr_console_cfg;
static SYS_MGR_Telnet_T         sys_mgr_telnet_cfg;
static SYS_MGR_Uart_Cfg_T       sys_mgr_uartAdmin_cfg; /* Modify name from XX_uart_cfg to XX_uartAdmin_cfg by S.K.Yang 2003.1.30. */

/* Remove VxWork dependent Constant - Frederick 01/08/2003 21:24 */
/*static int                      sys_mgr_sio_arg;*/

static UI8_T                    sys_mgr_prompt[SYS_ADPT_MAX_PROMPT_STRING_LEN + 1];

static BOOL_T                   provision_complete;


#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
static BOOL_T sys_mgr_autobaudrate_switch = FALSE;
#endif
static SYS_MGR_Uart_BaudRate_T baud_rate_set[SYS_MGR_NUMBER_OF_SUPPORT_BAUDRATE] =  {   SYS_MGR_UART_BAUDRATE_1200,
                                                                                    SYS_MGR_UART_BAUDRATE_2400,
                                                                                    SYS_MGR_UART_BAUDRATE_4800,
                                                                                    SYS_MGR_UART_BAUDRATE_9600,
                                                                                    SYS_MGR_UART_BAUDRATE_19200,
                                                                                    SYS_MGR_UART_BAUDRATE_38400,
                                                                                    SYS_MGR_UART_BAUDRATE_57600,
                                                                                    SYS_MGR_UART_BAUDRATE_115200
                                                                                };
/* add by S.K.Yang for autobaudrate */



/* Local data structures */
static UI32_T UART_Handle;
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
static char RXBuff[255];
#endif
/*   add for BUILD2 end  */

/* callback list of upper layer component when fan status changed
 */
static SYS_TYPE_CallBack_T *PowerStatusChanged_callbacklist;

SYSFUN_DECLARE_CSC                    /* declare variables used for transition mode  */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 *
 * INPUT: None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_Init(void)
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    UC_MGR_Sys_Info_T sys_info;
#endif
//  PowerStatusChanged_callbacklist = 0;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
//  FanStatusChanged_callbacklist = 0;
//  FanSpeedChanged_callbacklist = 0;
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
//  ThermalStatusChanged_callbacklist = 0;
#endif
//  XFPModuleStatusChanged_callbacklist = 0;


#ifndef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, need check detail, 2009-06-12, 16:15:23 */
      UART_Handle = SYSFUN_OpenUART(SYSFUN_UART_CHANNEL1);
#endif /* ES3526MA_POE_7LF_LN */

    /* Get Baudrate from sys_info set by loader */
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)

    /* kinghong added: We should set the baudrate at least one to ioctl, otherwise,
     * the operbaudrate will be 0.
     * Here we get the baudrate from UC, which is detect by loader, and setto ioctl
     */
    UC_MGR_GetSysInfo(&sys_info);

    /* kinghong: The baudrate get from Loader detect may not too accurately,
     * according to Jasonh suggestion, we use a +/- 10 % baudrate to setto ioctl.
     */
    if ((sys_info.baudrate <= SYS_MGR_UART_BAUDRATE_9600 * 1.1) && (sys_info.baudrate >= SYS_MGR_UART_BAUDRATE_9600 * 0.9))
    {
         sys_info.baudrate = SYS_MGR_UART_BAUDRATE_9600;
    }
    else if ((sys_info.baudrate <= SYS_MGR_UART_BAUDRATE_19200 * 1.1) && (sys_info.baudrate >= SYS_MGR_UART_BAUDRATE_19200 * 0.9))
    {
         sys_info.baudrate = SYS_MGR_UART_BAUDRATE_19200;
    }
    else if ((sys_info.baudrate <= SYS_MGR_UART_BAUDRATE_38400 * 1.1) && (sys_info.baudrate >= SYS_MGR_UART_BAUDRATE_38400 * 0.9))
    {
         sys_info.baudrate = SYS_MGR_UART_BAUDRATE_38400;
    }
    else if ((sys_info.baudrate <= SYS_MGR_UART_BAUDRATE_57600 * 1.1) && (sys_info.baudrate >= SYS_MGR_UART_BAUDRATE_57600 * 0.9))
    {
         sys_info.baudrate = SYS_MGR_UART_BAUDRATE_57600;
    }
    else if ((sys_info.baudrate <= SYS_MGR_UART_BAUDRATE_115200 * 1.1) && (sys_info.baudrate >= SYS_MGR_UART_BAUDRATE_115200 * 0.9))
    {
         sys_info.baudrate = SYS_MGR_UART_BAUDRATE_115200;
    }
    else
    {
        /* Should not happen, print out errmsg
         */
        printf(" Baud rate %ld detected from loader not valid, not setting to ioctl.\n\r", sys_info.baudrate);
        return TRUE;
    }

    SYSFUN_SetUartBaudRate(UART_Handle , sys_info.baudrate);
#endif  /* #if (SYS_CPNT_AUTOBAUDRATE == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    /* reset cpu util records
     */
    sys_mgr_cpu_util_5sec_record_valid_count = 0;

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
    sys_mgr_task_cpu_util_entry_buf_desc.buffer = (void *)sys_mgr_task_cpu_util_entry_buf;
    sys_mgr_task_cpu_util_entry_buf_desc.buffer_len = sizeof(sys_mgr_task_cpu_util_entry_buf);
    sys_mgr_task_cpu_util_entry_buf_desc.partition_len = sizeof(*sys_mgr_task_cpu_util_entry_buf);

    if (!L_PT_Create(&sys_mgr_task_cpu_util_entry_buf_desc))
    {
        return FALSE;
    }

    if (!L_SORT_LST_Create(
            &sys_mgr_task_cpu_util_entry_id_lst,
            sizeof(sys_mgr_task_cpu_util_entry_buf)/sizeof(*sys_mgr_task_cpu_util_entry_buf),
            sizeof(void *),
            SYS_MGR_CompareTaskCpuUtilEntryById))
    {
        return FALSE;
    }

    if (!L_SORT_LST_Create(
            &sys_mgr_task_cpu_util_entry_name_lst,
            sizeof(sys_mgr_task_cpu_util_entry_buf)/sizeof(*sys_mgr_task_cpu_util_entry_buf),
            sizeof(void *),
            SYS_MGR_CompareTaskCpuUtilEntryByName))
    {
        return FALSE;
    }
#endif
#endif

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &sys_mgr_sem_id) != SYSFUN_OK)
    {
        printf("SYS_MGR_Init : Can't get semaphore.\n");
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYS_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sys_mgr", SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY, SYS_BACKDOOR_Main);
//  SYSDRV_Register_PowerStatusChanged_CallBack(SYS_MGR_PowerStatusChanged_CallBack);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
//  SYSDRV_Register_FanStatusChanged_CallBack(SYS_MGR_FanStatusChanged_CallBack);
//  SYSDRV_Register_FanSpeedChanged_CallBack(SYS_MGR_FanSpeedChanged_CallBack);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
//  SYSDRV_Register_ThermalStatusChanged_CallBack(SYS_MGR_ThermalStatusChanged_CallBack);
//  BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sysmgr", SYS_MGR_BackdoorCallBack);
#endif
//  SYSDRV_Register_XFPModuleStatusChanged_CallBack(SYS_MGR_XFPModuleStatusChanged_CallBack);

    /* Register a backdoor debug function
     */
//  BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sys_mgr", SYS_BACKDOOR_Main);

} /* end of SYS_MGR_Create_InterCSC_Relation */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDefaultPromptString
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get the default value of the Prompt String
 *          from SYS_DFLT
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : *prompt_string -- default prompt string with max length 32
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * ---------------------------------------------------------------------
 */
static BOOL_T SYS_MGR_GetDefaultPromptString(UI8_T *prompt_string)
{
    UI32_T board_id=0;
    UI32_T unit_id=0;

    if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
    {
        printf("\r\n%s:STKTPLG_POM_GetMyUnitID return false=%lu", __FUNCTION__, (unsigned long)unit_id);
        return FALSE;
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(prompt_string, 0, SYS_ADPT_MAX_PROMPT_STRING_LEN+1);
    switch (board_id)
    {
        case 0:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID0, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        case 1:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID1, SYS_ADPT_MAX_PROMPT_STRING_LEN);
             break;
        case 2:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID2, SYS_ADPT_MAX_PROMPT_STRING_LEN);
             break;
        case 3:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID3, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        case 4:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID4, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        case 5:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID5, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        case 6:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID6, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        case 7:
            strncpy((char *)prompt_string, SYS_DFLT_SYSMGR_PROMPT_DEF_FOR_BOARD_ID7, SYS_ADPT_MAX_PROMPT_STRING_LEN);
            break;
        default:
            printf("%s:invalid board id=%lu\n", __FUNCTION__, (unsigned long)board_id);
            return FALSE;
            break;
    }
    return TRUE;
}

// file: bsd-strlcpy.c

#ifndef HAVE_STRLCPY
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
static size_t strlcpy(char* dst, const char* src, size_t siz)
{
    register char* d = dst;
    register const char* s = src;
    register size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0'; /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1); /* count does not include NUL */
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to SYS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        SYS_MGR_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
static void SYS_MGR_SetConfigSettingToDefault()
{
    sys_mgr_console_cfg.password_threshold = SYS_DFLT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD;
    sys_mgr_console_cfg.exec_timeout       = SYS_DFLT_SYSMGR_CONSOLE_EXEC_TIMEOUT;
    sys_mgr_console_cfg.login_timeout      = SYS_DFLT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT;
    sys_mgr_console_cfg.silent_time        = SYS_DFLT_SYSMGR_CONSOLE_SILENT_TIME;

    sys_mgr_telnet_cfg.password_threshold  = SYS_DFLT_SYSMGR_TELNET_PASSWORD_THRESHOLD;
    sys_mgr_telnet_cfg.exec_timeout        = SYS_DFLT_SYSMGR_TELNET_EXEC_TIMEOUT;
    sys_mgr_telnet_cfg.login_timeout       = SYS_DFLT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT;
    sys_mgr_telnet_cfg.silent_time         = SYS_DFLT_SYSMGR_TELNET_SILENT_TIME;

    sys_mgr_uartAdmin_cfg.baudrate         = SYS_MGR_UART_ADMIN_BAUDRATE_DEF;
    sys_mgr_uartAdmin_cfg.parity           = SYS_MGR_UART_PARITY_DEF;
    sys_mgr_uartAdmin_cfg.data_length      = SYS_MGR_UART_DATA_LENGTH_DEF;
    sys_mgr_uartAdmin_cfg.stop_bits        = SYS_MGR_UART_STOP_BITS_DEF;

    SYS_MGR_GetDefaultPromptString(sys_mgr_prompt);

/* Remove VxWork dependent Constant - Frederick 01/08/2003 21:24*/
    /* data length 8, no parity, stop bit 1
     */
/*    sys_mgr_sio_arg = (CS8 & ~PARENB & ~STOPB); */

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    /* kinghong comment: Note: The function call is useless currently since this API
     * will only work when CSC enter master mode, but the current calling time
     * is only in EnterTransitionMode.
     * We leave this API calls here for future use when CSC is in mastermode.
     */
    SYS_MGR_SetUartBaudrate(SYS_DFLT_UART_ADMIN_BAUDRATE);
#endif

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    cpu_guard.config.watermark_high = SYS_DFLT_CPU_UTILIZATION_WATERMARK_HIGH;
    cpu_guard.config.watermark_low  = SYS_DFLT_CPU_UTILIZATION_WATERMARK_LOW;
    cpu_guard.config.threshold_max  = SYS_DFLT_CPU_GUARD_THRESHOLD_MAX;
    cpu_guard.config.threshold_min  = SYS_DFLT_CPU_GUARD_THRESHOLD_MIN;
    cpu_guard.config.status         = SYS_DFLT_CPU_GUARD_STATUS;
    cpu_guard.config.trap_status    = SYS_DFLT_CPU_GUARD_TRAP_STATUS;
    cpu_guard.status.cpu_rate       = SYS_DFLT_CPU_GUARD_THRESHOLD_MAX;
    cpu_guard.debug_flag            = FALSE;
    DEV_SWDRV_PMGR_SetCpuPortRate(cpu_guard.status.cpu_rate);
#endif

{
    UI32_T bid;

    strlcpy(sysmgr_swinfo.product_manufacturer, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(sysmgr_swinfo.product_manufacturer));
    strlcpy(sysmgr_swinfo.product_description, SYS_ADPT_PRODUCT_DESCRIPTION, sizeof(sysmgr_swinfo.product_description));

    for (bid = 0; bid < sysmgr_board_count; bid++) {
        SYS_MGR_BoardInfo_T* binfo = &sysmgr_swinfo.board_info[bid];

        const char* sys_oid;
        const char* private_mib_root;
        const char* sys_descr;
        const char* prod_name;
        const char* model_name;

        switch (bid) {
        case 0:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID0;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID0;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID0;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID0;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID0;
            break;

        case 1:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID1;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID1;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID1;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID1;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID1;
            break;

        case 2:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID2;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID2;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID2;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID2;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID2;
            break;

        case 3:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID3;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID3;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID3;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID3;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID3;
            break;

        case 4:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID4;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID4;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID4;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID4;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID4;
            break;

        case 5:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID5;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID5;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID5;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID5;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID5;
            break;

        case 6:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID6;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID6;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID6;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID6;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID6;
            break;

        case 7:
            sys_oid = SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID7;
            private_mib_root = SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID7;
            sys_descr = SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID7;
            prod_name = SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID7;
            model_name = SYS_ADPT_MODEL_NAME_FOR_BOARD_ID7;
            break;

        default:
			/*It shouldn't run here!*/
			return;
        }

        strlcpy(binfo->sys_oid, sys_oid, sizeof(binfo->sys_oid));
        strlcpy(binfo->private_mib_root, private_mib_root, sizeof(binfo->private_mib_root));
        strlcpy(binfo->sys_descr, sys_descr, sizeof(binfo->sys_descr));
        strlcpy(binfo->product_name, prod_name, sizeof(binfo->product_name));
        strlcpy(binfo->model_name, model_name, sizeof(binfo->model_name));
    }
} // end of init sysmgr_swinfo ...

}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_EnterTransitionMode(void)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    sys_mgr_cpu_alarm.alarm_status = FALSE;
    sys_mgr_cpu_alarm.rising_threshold = SYS_DFLT_SYSMGR_CPU_UTILIZATION_RAISING_THRESHOLD;
    sys_mgr_cpu_alarm.falling_threshold = SYS_DFLT_SYSMGR_CPU_UTILIZATION_FALLING_THRESHOLD;
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    sys_mgr_mem_alarm.alarm_status = FALSE;
    sys_mgr_mem_alarm.rising_threshold = SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_RAISING_THRESHOLD;
    sys_mgr_mem_alarm.falling_threshold = SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_FALLING_THRESHOLD;
#endif

    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();

    provision_complete = FALSE;

    /* Ted marked off 08/04/04,
     * Because Loader has done autobaudrate
     * If we reset the baudrate to default
     * it might cause the baudrate not matching to the value Loader detected
     * This will cause console show characters which is not able to recognize
     * If we don't reset baudrate here,the value will use what loader has detected.
     */
    /*ioctl (consoleFd, SIO_BAUD_SET, (int)SYS_DFLT_UART_OPER_BAUDRATE);*/
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SYS_MGR into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SYS_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_EnterMasterMode(void)
{
    /* kinghong comment out, transition mode already do this*/
    /*SYS_MGR_SetConfigSettingToDefault();*/

    SYS_ENV_VM_SetDatabaseToDefault();

    STKTPLG_POM_GetNumberOfUnit(&sys_mgr_num_of_unit);

    SYS_MGR_SetConfigSettingToDefault();
    /* set mgr in master mode */

    SYS_MGR_LoadConfig();

    SYSFUN_ENTER_MASTER_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

#if (SYS_CPNT_CONFIG_AGENT == TRUE)
static void SYS_MGR_LoadConfig_ProductManufacturer(const json_t* json, const char*key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    str = config_agent_json_get_string(json, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_ProductDescription(const json_t* json, const char* key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    str = config_agent_json_get_string(json, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardSysOid(const json_t* board_info, const char* key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;
    options.pattern = "^1\\.3\\.6\\.1\\.4\\.1\\.(\\d+\\.)*\\d+$";

    str = config_agent_json_get_string(board_info, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardPrivateMibRoot(const json_t* board_info, const char* key, char *storage, UI32_T storage_size)
{
    const UI16_T SYS_ADPT_PRIVATEMIB_OID_ARRAY[] = { SYS_ADPT_PRIVATEMIB_OID };
    const UI32_T SYS_ADPT_PRIVATEMIB_OID_ARRAY_LEN = (sizeof(SYS_ADPT_PRIVATEMIB_OID_ARRAY) / sizeof(SYS_ADPT_PRIVATEMIB_OID_ARRAY[0]));
    const UI32_T MAX_AVAILABLE_LEN = SYS_ADPT_PRIVATEMIB_OID_ARRAY_LEN - 6; // 6 means [1, 3, 6, 1, 4, 1]

    const char pattern_tpl[] = "^1\\.3\\.6\\.1\\.4\\.1\\.(\\d+\\.){0,%u}\\d+$";
    char pattern[ sizeof(pattern_tpl) + 20 ];

    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    // "^1\\.3\\.6\\.1\\.4\\.1\\.(\\d+\\.){0,4}\\d+$";
    //                                       ^
    snprintf(pattern, sizeof(pattern), pattern_tpl, MAX_AVAILABLE_LEN - 1);

    options.pattern = pattern;

    str = config_agent_json_get_string(board_info, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardSysDescription(const json_t* board_info, const char*key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    str = config_agent_json_get_string(board_info, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardProductName(const json_t* board_info, const char* key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    str = config_agent_json_get_string(board_info, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardModelName(const json_t* board_info, const char* key, char *storage, UI32_T storage_size)
{
    config_agent_get_string_options_t options;
    const char* str;

    memcpy(&options, config_agent_json_default_get_string_options(), sizeof(options));

    options.max_length = storage_size - 1;

    str = config_agent_json_get_string(board_info, key, &options);

    if (str)
    {
        strlcpy(storage, str, storage_size);
    }
}

static void SYS_MGR_LoadConfig_BoardInfos(const json_t* json, const char* key)
{
    config_agent_get_array_options_t options;
    const json_t* board_infos;

    memcpy(&options, config_agent_json_default_get_array_options(), sizeof(options));
    options.max_items = sysmgr_board_count;

    board_infos = config_agent_json_get_array(json, key, &options);

    if (board_infos)
    {
        UI32_T bid;
        json_t* binfo;

        json_array_foreach(board_infos, bid, binfo)
        {
            if (sysmgr_board_count <= bid) {
                break;
            }

            if (json_is_object(binfo))
            {
                SYS_MGR_LoadConfig_BoardSysOid(binfo, "sysOid",
                    sysmgr_swinfo.board_info[bid].sys_oid,
                    sizeof(sysmgr_swinfo.board_info[bid].sys_oid));

                SYS_MGR_LoadConfig_BoardPrivateMibRoot(binfo, "privateMibRoot",
                    sysmgr_swinfo.board_info[bid].private_mib_root,
                    sizeof(sysmgr_swinfo.board_info[bid].private_mib_root));

                SYS_MGR_LoadConfig_BoardSysDescription(binfo, "sysDescr",
                    sysmgr_swinfo.board_info[bid].sys_descr,
                    sizeof(sysmgr_swinfo.board_info[bid].sys_descr));

                SYS_MGR_LoadConfig_BoardProductName(binfo, "productName",
                    sysmgr_swinfo.board_info[bid].product_name,
                    sizeof(sysmgr_swinfo.board_info[bid].product_name));

                SYS_MGR_LoadConfig_BoardModelName(binfo, "modelName",
                    sysmgr_swinfo.board_info[bid].model_name,
                    sizeof(sysmgr_swinfo.board_info[bid].model_name));
            }
        }
    }
}
#endif // SYS_CPNT_CONFIG_AGENT

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_LoadConfig
 * ---------------------------------------------------------------------
 * PURPOSE: This function will load config from file
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_LoadConfig(void)
{
#if (SYS_CPNT_CONFIG_AGENT == TRUE)
    config_agent_doc_ptr_t doc;

    doc = config_agent_read_document("sysmgmt", NULL);

    if (doc)
    {
        SYS_MGR_LoadConfig_ProductManufacturer(doc->json, "productManufacturer",
            sysmgr_swinfo.product_manufacturer,
            sizeof(sysmgr_swinfo.product_manufacturer));

        SYS_MGR_LoadConfig_ProductDescription(doc->json, "productDescription",
            sysmgr_swinfo.product_description,
            sizeof(sysmgr_swinfo.product_description));

        SYS_MGR_LoadConfig_BoardInfos(doc->json, "boardInfos");

        config_agent_doc_free(doc);
    }
#endif // SYS_CPNT_CONFIG_AGENT

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetConsoleCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of console setting
 * INPUT    : None
 * OUTPUT   : console setting(by record)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetConsoleCfg(SYS_MGR_Console_T *consoleCfg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        *consoleCfg = sys_mgr_console_cfg;
        return (TRUE);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningPasswordThreshold(UI32_T *passwordThreshold)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *passwordThreshold = sys_mgr_console_cfg.password_threshold;

        if (SYS_DFLT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD != *passwordThreshold)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetPasswordThreshold(UI32_T passwordThreshold)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((passwordThreshold != SYSMGMT_TYPE_PASSWORD_THRESHOLD_DISABLED) &&
        ((passwordThreshold < SYS_ADPT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD_MIN) ||
         (passwordThreshold > SYS_ADPT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD_MAX)))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetPasswordThreShold_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Password threshold (1  - 120)");
        return FALSE;
    }

    sys_mgr_console_cfg.password_threshold = passwordThreshold;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningConsoleExecTimeOut(UI32_T *time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *time_out_value = sys_mgr_console_cfg.exec_timeout;

        if (SYS_DFLT_SYSMGR_CONSOLE_EXEC_TIMEOUT != *time_out_value)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT:   time_out_value
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetConsoleExecTimeOut(UI32_T time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((time_out_value != SYSMGMT_TYPE_EXEC_TIMEOUT_DISABLED) &&
        ((time_out_value < SYS_ADPT_SYSMGR_CONSOLE_EXEC_TIMEOUT_MIN) ||
         (time_out_value > SYS_ADPT_SYSMGR_CONSOLE_EXEC_TIMEOUT_MAX)))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetConsoleExecTimeOut_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Console timeout (1-65535)");
        return FALSE;
    }

    sys_mgr_console_cfg.exec_timeout = time_out_value;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningConsoleSilentTime(UI32_T *silent_time)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *silent_time = sys_mgr_console_cfg.silent_time;

        if (SYS_DFLT_SYSMGR_CONSOLE_SILENT_TIME != *silent_time)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetConsoleSilentTime(UI32_T silent_time)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((silent_time != SYSMGMT_TYPE_SILENT_TIME_DISABLED) &&
        ((silent_time < SYS_ADPT_SYSMGR_CONSOLE_SILENT_TIME_MIN) ||
         (silent_time > SYS_ADPT_SYSMGR_CONSOLE_SILENT_TIME_MAX)))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetConsoleSilentTime_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Console silent time (1-65535)");
        return FALSE;
    }

    sys_mgr_console_cfg.silent_time = silent_time;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningTelnetSilentTime(UI32_T *silent_time)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *silent_time = sys_mgr_telnet_cfg.silent_time;

        if (SYS_DFLT_SYSMGR_TELNET_SILENT_TIME != *silent_time)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetTelnetSilentTime(UI32_T silent_time)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((silent_time != SYSMGMT_TYPE_SILENT_TIME_DISABLED) &&
        ((silent_time < SYS_ADPT_SYSMGR_TELNET_SILENT_TIME_MIN) ||
         (silent_time > SYS_ADPT_SYSMGR_TELNET_SILENT_TIME_MAX)))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetConsoleSilentTime_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Console silent time (1-65535)");
        return FALSE;
    }

    sys_mgr_telnet_cfg.silent_time = silent_time;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetTelnetCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of Telnet setting
 * INPUT    : None
 * OUTPUT   : console setting(by record)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetTelnetCfg(SYS_MGR_Telnet_T *telnetCfg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        *telnetCfg = sys_mgr_telnet_cfg;
        return (TRUE);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningTelnetPasswordThreshold(UI32_T *passwordThreshold)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *passwordThreshold = sys_mgr_telnet_cfg.password_threshold;

        if (SYS_DFLT_SYSMGR_TELNET_PASSWORD_THRESHOLD != *passwordThreshold)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Telnet password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetTelnetPasswordThreshold(UI32_T passwordThreshold)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((passwordThreshold != SYSMGMT_TYPE_PASSWORD_THRESHOLD_DISABLED) &&
        ((passwordThreshold < SYS_ADPT_SYSMGR_TELNET_PASSWORD_THRESHOLD_MIN) ||
         (passwordThreshold > SYS_ADPT_SYSMGR_TELNET_PASSWORD_THRESHOLD_MAX)))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetPasswordThreShold_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Password threshold (1  - 120)");
        return FALSE;
    }

    sys_mgr_telnet_cfg.password_threshold = passwordThreshold;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningTelnetExecTimeOut(UI32_T *time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *time_out_value = sys_mgr_telnet_cfg.exec_timeout;

        if (SYS_DFLT_SYSMGR_TELNET_EXEC_TIMEOUT != *time_out_value)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetTelnetExecTimeOut(UI32_T time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((time_out_value < SYS_ADPT_SYSMGR_TELNET_EXEC_TIMEOUT_MIN) ||
        (time_out_value > SYS_ADPT_SYSMGR_TELNET_EXEC_TIMEOUT_MAX))
    {
        //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetTelnetExecTimeOut_Fun_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Telnet timeout (1  - 65535)");
        return FALSE;
    }

    sys_mgr_telnet_cfg.exec_timeout = time_out_value;
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the parameters of local console UART.
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetUartParameters(SYS_MGR_Uart_Cfg_T *uart_cfg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        *uart_cfg = sys_mgr_uartAdmin_cfg;

        return (TRUE);
    }
}

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetUartOperBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the operating baudrate of local console UART .
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetUartOperBaudrate(SYS_MGR_Uart_BaudRate_T *uart_operbaudrate)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        SYSFUN_GetUartBaudRate(UART_Handle,(UI32_T*) uart_operbaudrate);
        return (TRUE);
    }
}
#endif


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function register to cli for autobaudrate when provision complete
 * INPUT: None
 * OUTPUT:
 * RETURN: none
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_ProvisionComplete(void)
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    SYS_MGR_Uart_Cfg_T uart_cfg;
#endif

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    else
    {
        /* kinghong: We check if admin baud is auto, if auto, need to
         * regist callback function to CLI to active the autobaud mechanism
         */
        if (TRUE == SYS_MGR_GetUartParameters(&uart_cfg))
        {
            if ( SYS_MGR_UART_BAUDRATE_AUTO == uart_cfg.baudrate )
            {
                if ( FALSE == sys_mgr_autobaudrate_switch)
                {
                    /* Turn on autobaudrate switch*/
                    sys_mgr_autobaudrate_switch = TRUE;
                }
            }
        }
        return;
    }
#endif

    provision_complete = TRUE;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default uart setting is successfully retrieved.
 *          For UART parameters, any one is changed will be thought
 *          that the setting is different.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *        4. We have a rule that getrunning should be getrunning by field,
 *           but for CLI there is a command "terminal" which can set
 *           all the uart config by one command, so we can getrunning
 *           by record for this uart parameter.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningUartParameters(SYS_MGR_Uart_RunningCfg_T *uart_cfg)
{
    UI32_T  return_val = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
   /* SYS_MGR_Uart_BaudRate_T oper_baud;*/

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
      uart_cfg->baudrate    = sys_mgr_uartAdmin_cfg.baudrate;
        uart_cfg->parity      = sys_mgr_uartAdmin_cfg.parity;
        uart_cfg->data_length = sys_mgr_uartAdmin_cfg.data_length;
        uart_cfg->stop_bits   = sys_mgr_uartAdmin_cfg.stop_bits;

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
        /* kinghong comment out, we check only the admin baudrate and not checking
         * oper baudrate
         */
    #if 0
        SYS_MGR_GetUartOperBaudrate(&oper_baud);
        if (SYS_MGR_UART_ADMIN_BAUDRATE_DEF != sys_mgr_uartAdmin_cfg.baudrate
        || SYS_MGR_UART_OPER_BAUDRATE_DEF != oper_baud)
    #endif
        if (SYS_MGR_UART_ADMIN_BAUDRATE_DEF != sys_mgr_uartAdmin_cfg.baudrate)
#else
        if (SYS_MGR_UART_OPER_BAUDRATE_DEF != sys_mgr_uartAdmin_cfg.baudrate)
#endif
        {
            uart_cfg->baurdrate_changed = TRUE;
            return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            uart_cfg->baurdrate_changed = FALSE;


        if (SYS_MGR_UART_PARITY_DEF != sys_mgr_uartAdmin_cfg.parity)
        {
            uart_cfg->parity_changed = TRUE;
            return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            uart_cfg->parity_changed = FALSE;

        if (SYS_MGR_UART_DATA_LENGTH_DEF != sys_mgr_uartAdmin_cfg.data_length)
        {
            uart_cfg->data_length_changed = TRUE;
            return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            uart_cfg->data_length_changed = FALSE;

        if (SYS_MGR_UART_DATA_LENGTH_DEF != sys_mgr_uartAdmin_cfg.data_length)
        {
            uart_cfg->data_length_changed = TRUE;
            return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            uart_cfg->data_length_changed = FALSE;

        if (SYS_MGR_UART_STOP_BITS_DEF != sys_mgr_uartAdmin_cfg.stop_bits)
        {
            uart_cfg->stop_bits_changed = TRUE;
            return_val = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
            uart_cfg->stop_bits_changed = FALSE;

        return return_val;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set baudrate of serial port
 * INPUT    : SYS_MGR_Uart_BaudRate_T Baudrate,0 for autobaudrate
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 0 for autobaudrate  S.K.Yang 2003.1.30 added  for autobaud
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartBaudrate(SYS_MGR_Uart_BaudRate_T baudrate)
{
    UI8_T index = 0;
    UI8_T found = FALSE;

    /* Check if baudrate exceeds range.
     * Zhong Qiyao, 2004.08.06
     */
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    /* Check autobaud
     */
    if (SYS_MGR_UART_BAUDRATE_AUTO == baudrate)
    {
        found = TRUE;
    }
    else
    {
#endif

        /* Check fixed baud
         */
        while (index < SYS_MGR_NUMBER_OF_SUPPORT_BAUDRATE)
        {
            /* baudrate is valid */
            if (baudrate == baud_rate_set[index])
            {
                found = TRUE;
                break;
            }
            index++;
        }

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    }
#endif

    /* If invalid baud rate, quit.
     */
    if (FALSE == found)
    {
        return (FALSE);
    }

    /* Check operating mode: master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* Data is valid. Set to system.
     */
    sys_mgr_uartAdmin_cfg.baudrate = baudrate;

/* Add for autobaudrate S.K.Yang 2002.1.30 */
/* Don't write such baud to UART register */
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    if (SYS_MGR_UART_BAUDRATE_AUTO == baudrate )
    {
        /* Notify CLI to detect baudrate automatically before entering a session */
        if ( FALSE == sys_mgr_autobaudrate_switch )
        {
            /* Turn on autobaudrate switch*/
            sys_mgr_autobaudrate_switch = TRUE;
        }
        return (TRUE);
    }
    else
    {
        /* Check if autobaudrate is on, if on , turn it off */
        if (TRUE == sys_mgr_autobaudrate_switch )
        {
            sys_mgr_autobaudrate_switch = FALSE;
        }
    }
#endif

    SYSFUN_SetUartBaudRate(UART_Handle,(UI32_T) baudrate);
    SYSFUN_Sleep(1);

    return (TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartParity
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Parity of serial port
 * INPUT    : SYS_MGR_Uart_Parity_T Parity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartParity(SYS_MGR_Uart_Parity_T parity)
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    SYS_MGR_Uart_BaudRate_T oper_baud;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        if ((parity != SYS_MGR_UART_PARITY_NONE) &&
            (parity != SYS_MGR_UART_PARITY_EVEN) &&
            (parity != SYS_MGR_UART_PARITY_ODD))
        {
            //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetUartParity_Fun_NO, EH_TYPE_MSG_INVALID_VALUE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE  | SYSLOG_LEVEL_DEBUG), "parity");
            return (FALSE);
        }
        sys_mgr_uartAdmin_cfg.parity = parity;


#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
        SYS_MGR_GetUartOperBaudrate(&oper_baud);
#endif

        SYSFUN_SetUartCfg(UART_Handle,
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
                            oper_baud,
#else
                            sys_mgr_uartAdmin_cfg.baudrate,
#endif
                            sys_mgr_uartAdmin_cfg.parity,
                            sys_mgr_uartAdmin_cfg.data_length,
                            sys_mgr_uartAdmin_cfg.stop_bits);

        SYSFUN_Sleep(1);
        return (TRUE);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartDataBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set DataLength of serial port
 * INPUT    : SYS_MGR_Uart_Data_Length_T DataLength
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartDataBits(SYS_MGR_Uart_Data_Length_T data_length)
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    SYS_MGR_Uart_BaudRate_T oper_baud;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        if ((data_length != SYS_MGR_UART_DATA_LENGTH_5_BITS) &&
            (data_length != SYS_MGR_UART_DATA_LENGTH_6_BITS) &&
            (data_length != SYS_MGR_UART_DATA_LENGTH_7_BITS) &&
            (data_length != SYS_MGR_UART_DATA_LENGTH_8_BITS))
        {
            //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetUartDataBits_Fun_NO, EH_TYPE_MSG_INVALID_VALUE,  (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "data length");
            return (FALSE);
        }

        sys_mgr_uartAdmin_cfg.data_length = data_length;

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
        SYS_MGR_GetUartOperBaudrate(&oper_baud);
#endif
        SYSFUN_SetUartCfg(UART_Handle,
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
                            oper_baud,
#else
                            sys_mgr_uartAdmin_cfg.baudrate,
#endif
                            sys_mgr_uartAdmin_cfg.parity,
                            sys_mgr_uartAdmin_cfg.data_length,
                            sys_mgr_uartAdmin_cfg.stop_bits);


        SYSFUN_Sleep(1);
        return (TRUE);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartStopBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set StopBits of serial port
 * INPUT    : SYS_MGR_Uart_Stop_Bits_T stop_bits
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartStopBits(SYS_MGR_Uart_Stop_Bits_T stop_bits)
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    SYS_MGR_Uart_BaudRate_T oper_baud;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        if ((stop_bits != SYS_MGR_UART_STOP_BITS_1_BITS) &&
            (stop_bits != SYS_MGR_UART_STOP_BITS_2_BITS))
        {
            //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_SetUartStopBits_Fun_NO, EH_TYPE_MSG_INVALID_VALUE,  (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "stop bits");
            return (FALSE);
        }

        sys_mgr_uartAdmin_cfg.stop_bits = stop_bits;

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
        SYS_MGR_GetUartOperBaudrate(&oper_baud);
#endif

        SYSFUN_SetUartCfg(UART_Handle,
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
                            oper_baud,
#else
                            sys_mgr_uartAdmin_cfg.baudrate,
#endif
                            sys_mgr_uartAdmin_cfg.parity,
                            sys_mgr_uartAdmin_cfg.data_length,
                            sys_mgr_uartAdmin_cfg.stop_bits);

        SYSFUN_Sleep(1);
        return (TRUE);
    }
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysInfo
 * ---------------------------------------------------------------------
 * PURPOSE: get system info for specified unit
 * INPUT    : unit      - key to specifiy a unique uit in the stack
 * OUTPUT   : sys_info  - all the system info of specified unit
 * RETURN   : TRUE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSysInfo(UI32_T unit, SYS_MGR_Info_T *sys_info)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        if (unit > sys_mgr_num_of_unit)
        {
            //EH_PMGR_Handle_Exception1(SYS_MODULE_SYSMGMT, SYS_MGR_GetSysInfo_Fun_NO,  EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "(0~sys_mgr_num_of_unit)");
            return FALSE;
        }

        STKTPLG_POM_GetSysInfo(unit, sys_info);
        return TRUE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_RestartSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will restart system by loader to decompress
 *            the run time image and load the code to DRAM from flash.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
 /*this function need to modify*/
void SYS_MGR_RestartSystem(void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return ;
    }
    else
    {
#ifdef CPU_82XX
        UI8_T   *addr = (UI8_T *)SYS_HWCFG_CPLD_BASE;

        *addr = 1;
        SYSFUN_Sleep(1);
        *addr = 0;
        SYSFUN_Sleep(1);
        *addr = 1;
        SYSFUN_Sleep(1);
        *addr = 0;
#else
#endif
        return ;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_RestartSystemFromPOST
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will restart system from POST loader.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_RestartSystemFromPOST(void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return ;
    }
    else
    {
#ifdef CPU_82XX
    UI8_T   *addr = (UI8_T *)SYS_HWCFG_CPLD_BASE;

    *addr = 1;
    SYSFUN_Sleep(1);
    *addr = 0;
    SYSFUN_Sleep(1);
    *addr = 1;
    SYSFUN_Sleep(1);
    *addr = 0;
#else
#endif
        return ;
    }
}


/*under some condition kill some task will make system hang up,
 *so comment out calling this rutine
 */
/*extern void    SYSFUN_KillAllTasksExceptMeAndSystem(void);
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : *prompt_string -- prompt string with max length 32
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetPromptString(UI8_T *prompt_string)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        memcpy(prompt_string, sys_mgr_prompt, SYS_ADPT_MAX_PROMPT_STRING_LEN);
        prompt_string[SYS_ADPT_MAX_PROMPT_STRING_LEN] = 0;
    }
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetPromptString(UI8_T *prompt_string)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        memcpy(sys_mgr_prompt, prompt_string, SYS_ADPT_MAX_PROMPT_STRING_LEN);
        sys_mgr_prompt[SYS_ADPT_MAX_PROMPT_STRING_LEN] = 0;
    }
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get running system prompt string
 * INPUT    : *prompt_string -- prompt string address
 * OUTPUT   : *prompt_string -- prompt string with max length 32
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningPromptString(UI8_T *prompt_string)
{
    UI8_T dflt_prompt_string[SYS_ADPT_MAX_PROMPT_STRING_LEN+1];
    if (SYS_MGR_GetPromptString(prompt_string) == FALSE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (!SYS_MGR_GetDefaultPromptString(dflt_prompt_string))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    else if (strcmp((char *)prompt_string, (char *)dflt_prompt_string) == 0)
    {
        /* If sys_mgr_prompt is same as default one, return NO_CHANGE
         * Note that prompt_string is the same as sys_mgr_prompt right now.
         */
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_LogWatchDogExceptionInfo
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set watch dog timer exception information
 *          to SYS_MGR_WATCHDOG_TIMER_FILE
 * INPUT    : SYS_MGR_WatchDogExceptionInfo_T *wd_own
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : Call by NMI exception 0x500 handler rtcInt() in usrconfig.c
 *            and backdoor only!!
 * ---------------------------------------------------------------------
 */
 /*this function need to modify*/
BOOL_T SYS_MGR_LogWatchDogExceptionInfo(SYS_MGR_WatchDogExceptionInfo_T *wd_own)
{
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
    /* Note: When we use backdoor turn off kick, it will get in exception.
     *       We need turn on kick in exception handler again.
     *       [Debug Issue]
     */
    SYS_TIME_SetKick(TRUE);

    memset(&local_tcb, 0, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
    SYS_TIME_KickWatchDogTimer();
    memcpy(&local_tcb, wd_own, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
    SYS_TIME_KickWatchDogTimer();

    if (SYS_MGR_ExistWDLogfileInFileSystem() == FALSE)
    {
        SYS_TIME_KickWatchDogTimer();
        printf("\r\nWatch Dog Log file not exist");
        if (SYS_MGR_CreateWDLogfileInFileSystem() == FALSE)
        {
            printf("\r\nCreate Watch Dog Log file Error!!");
            return FALSE;
        }
    }

    SYS_TIME_KickWatchDogTimer();
    if (FALSE == SYS_MGR_LogExcInfoToLogFile())
    {
        printf("\r\nLog Watch Dog Log file Error!!\n");
        return FALSE;
    }
    STKCTRL_PMGR_ColdStartSystem();
#endif /* SYS_CPNT_WATCHDOG_TIMER */
    return TRUE;
} /* End of SYS_MGR_LogWatchDogExceptionInfo */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ShowWatchDogTimerLogInfoFromLogFile
 * ---------------------------------------------------------------------
 * PURPOSE: Show All Information Get from Watch Dog Log File
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE - success.
 *          FALSE- false.
 * NOTES:   This function will show watch dog timer exceprion
 *          information by itself.
 * ---------------------------------------------------------------------
 */
  /*this function need to modify*/
BOOL_T SYS_MGR_ShowWatchDogTimerLogInfoFromLogFile(void)
{
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
    UI32_T  read_count = 0;
    UI32_T  result;
    SYS_MGR_WDLogfileDb_T   prepare_db;

    if (SYS_MGR_ExistWDLogfileInFileSystem()    == FALSE)
    {
        printf("\r\nWatch Dog Timer Log File Doesn't Exist !!!");
        printf("\r\n");
        return FALSE;
    }

    memset(&prepare_db, 0, sizeof(SYS_MGR_WDLogfileDb_T));

    if ((result=FS_ReadFile(DUMMY_DRIVE, (UI8_T*)SYS_MGR_WATCHDOG_TIMER_FILE, (UI8_T*)&prepare_db,
        (SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH + (SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE * SYS_MGR_WD_TIMER_EXC_INFO_LENGTH)),
        &read_count)) == FS_RETURN_OK || result == FS_RETURN_FILE_TRUNCATED)
    {
        printf("\r\nWatch Dog Exception Happened");
        printf("\r\n===Exception Information====");

        if (prepare_db.header.count == SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE &&
           prepare_db.header.sequence_no != SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE)
        {
            UI32_T i, j = 1;
            for(i=prepare_db.header.sequence_no; i<prepare_db.header.count; i++)
            {
                printf("\r\nSequence Number     : %ld",j++);
                SYS_MGR_DisplayLogInfo((SYS_MGR_WatchDogExceptionInfo_T*)(prepare_db.message + (SYS_MGR_WD_TIMER_EXC_INFO_LENGTH * i)));
            }

            for(i=0; i<prepare_db.header.sequence_no; i++)
            {
                printf("\r\nSequence Number     : %ld",j++);
                SYS_MGR_DisplayLogInfo((SYS_MGR_WatchDogExceptionInfo_T*)(prepare_db.message + (SYS_MGR_WD_TIMER_EXC_INFO_LENGTH * i)));
            }

        }
        else
        {
            UI32_T i;
            for(i=0; i<prepare_db.header.count; i++)
            {
                printf("\r\nSequence Number     : %ld",i + 1);
                SYS_MGR_DisplayLogInfo((SYS_MGR_WatchDogExceptionInfo_T*)(prepare_db.message + (SYS_MGR_WD_TIMER_EXC_INFO_LENGTH * i)));
            }
        }
        return TRUE;
    }
    printf("\r\nWatch Dog Read Back Error!!");
    printf("\r\n");
#endif /* SYS_CPNT_WATCHDOG_TIMER */
    return FALSE;

} /* End of SYS_MGR_ShowWatchDogTimerLogInfoFromLogFile */

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_BaudRateCheck
 * ---------------------------------------------------------------------
 * PURPOSE  : To detect correct baudrate by passive detection and passive detection
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   : TRUE on Detecting ENTERKEY, FALSE on None key input or entering-key
 *            is not ENTERKEY
 *
 *
 * NOTES    : Use as an terminal conpoment, only recognition ENTER key
 * ---------------------------------------------------------------------
 */
#define CONSOLE_USER_INPUT_BUF_SIZE_WHEN_POLL   32
#define UART_BAUDRATE_SETTING_GAP               10
BOOL_T SYS_MGR_BaudRateCheck()
{
#define UART_GETLINE_TIMEOUT_STEP   5
#define UART_GETTOKEN_TIMEOUT       200
#define UART_TOKEN_NUMBER           7
#define CONSOLE_IDLE_TIME           10
#define ENTER_KEY                   0x0D


    SYS_MGR_Uart_BaudRate_T origin_baudrate = 0, *p , sugg_baudrate ;
    int index = 0,ch = 0;

    BOOL_T is_key_detected;
    UI8_T user_input_buffer_length = 0;
    UI8_T user_input_length_when_poll = 0;
    UI8_T user_input_when_poll[CONSOLE_USER_INPUT_BUF_SIZE_WHEN_POLL] = {0};
    UI8_T user_input_buffer[CONSOLE_USER_INPUT_BUF_SIZE_WHEN_POLL] = {0};

    UI32_T delta_idle_time = 0, idle_time_point = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }


    /* Get current Baudrate */
    p = &origin_baudrate;
    SYSFUN_GetUartBaudRate(UART_Handle, (UI32_T*) p);

    idle_time_point = SYSFUN_GetSysTick();

    while (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {

        if ((ch = SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff)) ==    -1)
        {
            delta_idle_time += (SYSFUN_GetSysTick() - idle_time_point);

            /* Count idle tick */
            idle_time_point = SYSFUN_GetSysTick();

            if ( CONSOLE_IDLE_TIME <= delta_idle_time)
            {
                /* Begin to poll */
                if (SYS_MGR_PollBaudRate(&is_key_detected,user_input_when_poll , &user_input_length_when_poll) == TRUE)
                {
                    if (is_key_detected == TRUE)
                    {
                        for (index = 0 ; index < user_input_length_when_poll ; index++)
                        {
                            if (user_input_when_poll[index] == 0xA || user_input_when_poll[index] == 0xD)
                            {
                                /* Clear queue when return */
                                while (SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff) !=    -1);
                                return TRUE;
                            }
                        }
                    }
                }
                else
                /* No connected or baudrate is wrong */
                {
                    p = &origin_baudrate;
                    SYSFUN_GetUartBaudRate(UART_Handle,(UI32_T*) p);

                    /* Change to max. baudrate */
                    if (is_key_detected == TRUE)
                    {
                        SYS_MGR_GuessBaudrate((UI8_T*) user_input_when_poll , user_input_length_when_poll , origin_baudrate , &sugg_baudrate);
                    }
                    else
                    {

                        if ( origin_baudrate != SYS_MGR_INTERMEDIATE_BAUDRATE_FOR_AUTO_DETECTION)
                        {
                        SYSFUN_SetUartBaudRate (UART_Handle,(UI32_T)SYS_MGR_INTERMEDIATE_BAUDRATE_FOR_AUTO_DETECTION);
                        SYSFUN_Sleep(UART_BAUDRATE_SETTING_GAP);
                    }
                }
                }
                /* Clear queue when return */
                while (SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff) !=    -1);
                return FALSE;
                /* end of poll*/
            }
            /*end of exceeds condition*/
            else
            {
                SYSFUN_Sleep(100);
            }

        }
        /* else of input buffer is not empty*/
        else
        {
            /* Set idle to zero, because we have already received data */
            delta_idle_time = 0;
            /* Get all chars in buf */
            user_input_buffer_length = 0;
            do
            {
                user_input_buffer[user_input_buffer_length] = (UI8_T)ch;
                user_input_buffer_length ++;
            }
            while ((ch = SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff))    != -1 && user_input_buffer_length <= CONSOLE_USER_INPUT_BUF_SIZE_WHEN_POLL);


            if (SYS_MGR_PollBaudRate(&is_key_detected,user_input_when_poll , &user_input_length_when_poll) == TRUE)
            {
                if (is_key_detected == TRUE)
                {
                    for (index = 0 ; index < user_input_length_when_poll ; index++)
                    {
                        if (user_input_buffer[index] == 0xA || user_input_buffer[index] == 0xD )
                        {
                            /* Clear queue when return */
                            while (SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff) !=    -1);
                            return TRUE;
                        }
                    }
                }
                else
                {

                    for (index = 0 ; index < user_input_buffer_length ; index++)
                    {
                        if (user_input_buffer[index] == 0xA || user_input_buffer[index] == 0xD )
                        {
                            /* Clear queue when return */
                            while (SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff) !=    -1);
                            return TRUE;
                        }
                    }
                }

            }
            else
            {
                /* Get current Baudrate */
                p = &origin_baudrate;
                SYSFUN_GetUartBaudRate(UART_Handle,(UI32_T*) p);

                /* Guess an baudrate, using input key */
                if (is_key_detected == FALSE)
                {
                    SYS_MGR_GuessBaudrate((UI8_T*) user_input_buffer,user_input_buffer_length,origin_baudrate,&sugg_baudrate);

                }
                else
                {
                    SYS_MGR_GuessBaudrate((UI8_T*) user_input_when_poll , user_input_length_when_poll , origin_baudrate , &sugg_baudrate);
                }
            }
            /* Count idle tick */
            /*idle_time_point = SYSFUN_GetSysTick();*/
            /* Clear queue when return */
            while (SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff) !=    -1);
            return FALSE;
        }

    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_PollBaudRate
 * ---------------------------------------------------------------------
 * PURPOSE  : To poll baudrate by sending consequence
 * INPUT    : none
 * OUTPUT   : 1. if any key enter when poll process.
 *               Yes, output is TRUE, No, output is FALSE
 *            2. User input key array when poll interval.
 *            3. length of user_input
 * RETURN   : TRUE if baudrate is ok, FALSE if baudrate is not ok
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_PollBaudRate(BOOL_T *is_key_detected, UI8_T *user_input , UI8_T *user_input_length)
{

    I8_T  t=0,check_index=0;
    int num_buffer_empty = 0;
    UI8_T cs_back = 0;
    //I8_T control_seq[] = {0x1B,0x5B,0x35,0x6E};
    UI8_T terminal_ok[] = {0x1B,0x5B,0x30,0x6E};
    int charfrombuf;
    UI8_T ch = 0 , n = 0;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    //  write(consoleFd,control_seq,4);
    SYSFUN_Sleep(5);

    *is_key_detected = FALSE;
    /*tick_start = SysTimer_GetTick();  */
    while (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if ( (charfrombuf =SYSFUN_UARTPollRxBuff(UART_Handle,255, RXBuff)) != -1 )/* buffer is not empty.*/
        {
            ch = (UI8_T)charfrombuf;
            cs_back = ch ;
            if (cs_back == terminal_ok[n])
            {
                n += 1;
                check_index ++;
            }
            else
            {
                user_input[t] = cs_back;
                (*user_input_length)++;
                t += 1;
            }
        }
        else /* Buffer is empty */
        {
            num_buffer_empty +=1;
        }
        if (n == 4 ||  t > CONSOLE_USER_INPUT_BUF_SIZE_WHEN_POLL  || num_buffer_empty >= 4)
        {
            break;
        }
        SYSFUN_Sleep(10);
    }

    /* user send some char to our console */
    /*if ( t >= 1)
    {
        *is_key_detected = TRUE;
    }
    */
    *is_key_detected = FALSE;
    (*user_input_length) = 0;
    if (check_index == 4)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GuessBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE  : Guess an baudrate when there are input characters but baudrate is wrong
 * INPUT    : 1. input chars 2. input chars length 3. current baudrate
 * OUTPUT   : 1. An suggestion baudrate
 *
 * RETURN   :  1. True : Can exactly guess an baudrate
 *                False: The suggestion baudrate may be wrong
 * NOTES    : It will setting baudrate to system
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GuessBaudrate(UI8_T* ch,UI8_T ch_length, SYS_MGR_Uart_BaudRate_T current_baudrate, SYS_MGR_Uart_BaudRate_T *sugg_baudrate)
{

    UI32_T suggest_baud = 0;
    BOOL_T status = TRUE;
    UI8_T index;

    if ( ch_length > 1)
    {
        for (index = 0 ; index < ch_length - 1 && suggest_baud == 0 ; index++)
        {
            switch (ch[index])
            {
                case 0xe6:
                    if (ch[index+1] == 0x80)
                    {
                        suggest_baud = current_baudrate/2;
                    }
                    break;
                case 0x1c:
                    suggest_baud = current_baudrate/3;
                    break;
                case 0x78:
                    if (ch[index+1] == 0xFC)
                    {
                        suggest_baud = current_baudrate/4;
                    }
                    break;
                case 0x80:
                    if (ch[index+1] == 0x80)
                    {
                        suggest_baud = current_baudrate/8;
                    }
                    break;

                case 0xe0:
                    if (ch[index+1] == 0xe0)
                    {
                        suggest_baud = current_baudrate/6;
                    }
                    break;

                case 0x0:
                    suggest_baud = SYS_DFLT_UART_OPER_BAUDRATE;
                    status = FALSE;
                    break;
            }
            if (ch[index] >= 0xf0) /* max baudrate */
            {
                suggest_baud = SYS_MGR_MAX_BAUDRATE;
                status = FALSE;
            }
        }
    }
    /* only one character */
    else
    {
        if (0x0 == ch[0]) /* intermediate baudrate */
        {
            suggest_baud = SYS_DFLT_UART_OPER_BAUDRATE;
        }
        else if (ch[0] >= 0xf0) /* max baudrate */
        {
            suggest_baud = SYS_MGR_MAX_BAUDRATE;
        }
        else /* it is a key but not recognition, change to default oper-baudrate*/
        {
            suggest_baud = SYS_DFLT_UART_OPER_BAUDRATE;
        }
        status = FALSE;
    }
    /* If no baudrate are suggested , then return */


    if (suggest_baud == 0)
    {
        suggest_baud = SYS_DFLT_UART_OPER_BAUDRATE;
        SYSFUN_SetUartBaudRate(UART_Handle,suggest_baud);
        *sugg_baudrate = suggest_baud;
        status = FALSE;
    }
    /* Clear queue when return
    while (sysSerialPollRxBuff() != -1);
    */
    SYSFUN_SetUartBaudRate(UART_Handle,suggest_baud);
    SYSFUN_Sleep(UART_BAUDRATE_SETTING_GAP);
    *sugg_baudrate = suggest_baud;

    if ( status == TRUE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Get_Autobaudrate_Switch
 * ---------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   :
 *
 *
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_Get_Autobaudrate_Switch()
{
    return sys_mgr_autobaudrate_switch;
}


#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
static void SYS_MGR_DisplayLogInfo(SYS_MGR_WatchDogExceptionInfo_T *show_info)
{
    SYS_MGR_WatchDogExceptionInfo_T wd_own;

    memcpy(&wd_own, show_info, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
    printf("\r\nLink Register       : 0x%lX",wd_own.reg_lr);
    printf("\r\nSRR0 Register       : 0x%lX",wd_own.reg_srr0);
    printf("\r\nSRR1 Register       : 0x%lX",wd_own.reg_srr1);
    printf("\r\nCondition Register  : 0x%lX",wd_own.reg_cr);
    printf("\r\nR1   Register       : 0x%lX",wd_own.reg_r1);
    printf("\r\n");
    printf("\r\nTask Name                    : %s",wd_own.name);
    printf("\r\ntask priority                : 0x%lX",wd_own.priority);
    printf("\r\ntask status                  : 0x%lX",wd_own.status);
    printf("\r\ndelay/timeout ticks          : 0x%lX",wd_own.delay);
    printf("\r\npoints to bottom of stack    : 0x%lX",wd_own.pStackBase);
    printf("\r\npoints to stack limit        : 0x%lX",wd_own.pStackLimit);
    printf("\r\npoints to init stack limit   : 0x%lX",wd_own.pStackEnd);
    printf("\r\nsize of stack in bytes       : 0x%lX",wd_own.stackSize);
    printf("\r\ncurrent stack usage in bytes : 0x%lX",wd_own.stackCurrent);
    printf("\r\ncurrent stack margin in bytes: 0x%lX",wd_own.stackMargin);
    printf("\r\n");

} /* End of SYS_MGR_DisplayLogInfo */

/* FUNCTION NAME: SYS_MGR_LogExcInfoToLogFile
 * PURPOSE: Log Exception Information to file system if exist any entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE - success.
 *          FALSE- false.
 * NOTES:   This function will be active in a exception handler.
 */
static BOOL_T SYS_MGR_LogExcInfoToLogFile(void)
{
    SYS_MGR_WDLogfileDb_T   prepare_db;
    UI32_T  read_count = 0;
    UI32_T  result;

    SYS_TIME_KickWatchDogTimer();
    if (local_header.count >    SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE ||
       local_header.sequence_no > SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE)
    {
        printf("\r\n[Log count]    = %ld ", local_header.count);
        printf("\r\n[Log sequence] = %ld ", local_header.sequence_no);
        printf("\r\nOut of log range!!");
        return FALSE;
    }

    SYS_TIME_KickWatchDogTimer();
    memset(&prepare_db, 0, sizeof(SYS_MGR_WDLogfileDb_T));

    SYS_TIME_KickWatchDogTimer();
    if (local_header.count >    0)
    {
        SYS_TIME_KickWatchDogTimer();
        if ((result=FS_ReadFile(DUMMY_DRIVE, (UI8_T*)SYS_MGR_WATCHDOG_TIMER_FILE, (UI8_T*)&prepare_db,
            (SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH + (SYS_MGR_WD_TIMER_EXC_INFO_LENGTH * local_header.count)),
            &read_count)) == FS_RETURN_OK || result == FS_RETURN_FILE_TRUNCATED)
        {
            SYS_TIME_KickWatchDogTimer();
            /* Read All information from file system SUCCESS */
        }
        else
        {
            printf("\r\nRead All Information Back from Flash Error!!");
            return FALSE;
        }
        SYS_TIME_KickWatchDogTimer();
    }

    SYS_TIME_KickWatchDogTimer();
    if (local_header.count == SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE)
    {
        prepare_db.header.count = SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE;
        if (prepare_db.header.sequence_no ==    SYS_MGR_MAX_ENTRIES_IN_WDTIMER_LOG_FILE)
            prepare_db.header.sequence_no = 1;
        else
            prepare_db.header.sequence_no = local_header.sequence_no + 1;
    }
    else
    {
        prepare_db.header.count       = local_header.count + 1;
        prepare_db.header.sequence_no = local_header.sequence_no + 1;
    }

    SYS_TIME_KickWatchDogTimer();
    memcpy((prepare_db.message + (SYS_MGR_WD_TIMER_EXC_INFO_LENGTH * (prepare_db.header.sequence_no - 1))),
           &local_tcb, sizeof(SYS_MGR_WatchDogExceptionInfo_T));
    SYS_TIME_KickWatchDogTimer();
    if (FS_WriteFile(DUMMY_DRIVE, (UI8_T*)SYS_MGR_WATCHDOG_TIMER_FILE, (UI8_T*)"WatchDog", FS_FILE_TYPE_PRIVATE, (UI8_T *) &prepare_db,
        (SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH + (prepare_db.header.count * SYS_MGR_WD_TIMER_EXC_INFO_LENGTH)),
         SYS_MGR_WD_LOGFILE_SIZE) != FS_RETURN_OK)
    {
        SYS_TIME_KickWatchDogTimer();
        printf("\r\nWrite All Information Back to Flash Error!!");
        return FALSE;
    }
    return TRUE;

} /* End of SYS_MGR_LogExcInfoToLogFile */

/* FUNCTION NAME: SYS_MGR_ExistWDLogfileInFileSystem
 * PURPOSE: This function is check the syslog file exist in file system or not??
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- exist.
 *          FALSE   -- not exist.
 * NOTES:
 *
 */
static BOOL_T SYS_MGR_ExistWDLogfileInFileSystem(void)
{
    SYS_MGR_WDLogfileHeader_T  header;
    UI32_T  read_count = 0;
    UI32_T  result;

    SYS_TIME_KickWatchDogTimer();
    if ((result=FS_ReadFile(DUMMY_DRIVE, (UI8_T*)SYS_MGR_WATCHDOG_TIMER_FILE, (UI8_T*)&header,
         SYS_MGR_WD_TIMER_FILE_HEADER_LENGTH, &read_count)) == FS_RETURN_OK || result == FS_RETURN_FILE_TRUNCATED)
    {
        SYS_TIME_KickWatchDogTimer();
        local_header.count       = header.count;
        local_header.sequence_no = header.sequence_no;
        return TRUE;
    }
    return FALSE;
} /* End of SYS_MGR_ExistWDLogfileInFileSystem */

/* FUNCTION NAME: SYS_MGR_CreateWDLogfileInFileSystem
 * PURPOSE: Create syslog file in file system.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- exist.
 *          FALSE   -- not exist.
 * NOTES:
 *
 */
static BOOL_T SYS_MGR_CreateWDLogfileInFileSystem(void)
{
    SYS_MGR_WDLogfileHeader_T  header;

    SYS_TIME_KickWatchDogTimer();
    memset((UI8_T *) &header, 0, sizeof(SYS_MGR_WDLogfileHeader_T));
    SYS_TIME_KickWatchDogTimer();
    header.count       = local_header.count = 0;
    header.sequence_no = local_header.sequence_no = 0;
    SYS_TIME_KickWatchDogTimer();

    if (FS_WriteFile(DUMMY_DRIVE, (UI8_T*)SYS_MGR_WATCHDOG_TIMER_FILE, (UI8_T*)"WatchDog",
        FS_FILE_TYPE_PRIVATE, (UI8_T *) &header, sizeof(SYS_MGR_WDLogfileHeader_T),
        SYS_MGR_WD_LOGFILE_SIZE) != FS_RETURN_OK)
    {
        return FALSE;
    }

    SYS_TIME_KickWatchDogTimer();
    return TRUE;
} /* End of SYS_MGR_CreateWDLogfileInFileSystem */
#endif /* SYS_CPNT_WATCHDOG_TIMER */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific unit.
 * INPUT  : power_status->sw_unit_index   --- Which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetPowerStatus(SYS_MGR_PowerStatus_T *power_status)
{
    UI32_T main_power_status;
    UI32_T redundant_power_status;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (power_status == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(power_status->sw_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    /* main power
     */
    if (FALSE == SYS_ENV_VM_GetPowerStatus(power_status->sw_unit_index,
                                           1,
                                           &main_power_status))
    {

        return FALSE;
    }

    /* redundant power
     */
#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2)
        if(!STKTPLG_OM_IsSupportRPU(power_status->sw_unit_index))
        {
            redundant_power_status =     VAL_swIndivPowerStatus_notPresent;
        }
        else if (FALSE == SYS_ENV_VM_GetPowerStatus(power_status->sw_unit_index,
                                           2,
                                           &redundant_power_status))
        {
            return FALSE;
        }
#else
        redundant_power_status =    VAL_swIndivPowerStatus_notPresent;
#endif


    if (main_power_status      == VAL_swIndivPowerStatus_green &&
        redundant_power_status == VAL_swIndivPowerStatus_green  )
    {
        power_status->sw_power_status = VAL_swPowerStatus_internalAndRedundantPower;
    }
    else if (main_power_status == VAL_swIndivPowerStatus_green)
    {
        power_status->sw_power_status = VAL_swPowerStatus_internalPower;
    }
    else
    {
        power_status->sw_power_status = VAL_swPowerStatus_redundantPower;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some unit.
 * INPUT  : power_status->sw_unit_index   --- Next to which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextPowerStatus(SYS_MGR_PowerStatus_T *power_status)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (power_status == 0)
    {
        return FALSE;
    }

    if (power_status->sw_unit_index == 0)
    {
        power_status->sw_unit_index = 1;
    }
    else
    {
        power_status->sw_unit_index ++;
    }

    if (FALSE == STKTPLG_POM_UnitExist(power_status->sw_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    if (FALSE == SYS_MGR_GetPowerStatus(power_status))
    {
        return FALSE;
    }
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Which unit.
 *          indiv_power->sw_indiv_power_index      --- Which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (indiv_power == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(indiv_power->sw_indiv_power_unit_index))
    {
        return FALSE;
    }

    if (indiv_power->sw_indiv_power_index == 0 ||
        indiv_power->sw_indiv_power_index > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return FALSE;
    }
#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT > 1)
    if (indiv_power->sw_indiv_power_index > 1 && !STKTPLG_OM_IsSupportRPU(indiv_power->sw_indiv_power_unit_index))
    {
        return FALSE;
    }
#endif

    /* action
     */
    if (SYS_ENV_VM_GetPowerStatus(indiv_power->sw_indiv_power_unit_index,
                                 indiv_power->sw_indiv_power_index,
                                 &(indiv_power->sw_indiv_power_status))==FALSE)
    {
        return FALSE;
    }

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
    if (SYS_ENV_VM_GetPowerType(indiv_power->sw_indiv_power_unit_index,
                               indiv_power->sw_indiv_power_index,
                               &(indiv_power->sw_indiv_power_type))==FALSE)
    {
        return FALSE;
    }
#endif

    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Next to which unit.
 *          indiv_power->sw_indiv_power_index      --- Next to which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (indiv_power == 0)
    {
        return FALSE;
    }

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT > 1)
    if (indiv_power->sw_indiv_power_unit_index == 0)
    {
        indiv_power->sw_indiv_power_unit_index = 1;
        indiv_power->sw_indiv_power_index      = 1;
    }
    else if (indiv_power->sw_indiv_power_index==1 && STKTPLG_OM_IsSupportRPU(indiv_power->sw_indiv_power_unit_index)==TRUE)
    {
        indiv_power->sw_indiv_power_index++;
    }
    else
    {
        indiv_power->sw_indiv_power_unit_index ++;
        indiv_power->sw_indiv_power_index = 1;
    }
#else
    if (indiv_power->sw_indiv_power_unit_index == 0)
    {
        indiv_power->sw_indiv_power_unit_index = 1;
        indiv_power->sw_indiv_power_index      = 1;
    }
    else
    {
        indiv_power->sw_indiv_power_unit_index ++;
        indiv_power->sw_indiv_power_index = 1;
    }
#endif

    if (FALSE == STKTPLG_POM_UnitExist(indiv_power->sw_indiv_power_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    if (SYS_ENV_VM_GetPowerStatus(indiv_power->sw_indiv_power_unit_index,
                                 indiv_power->sw_indiv_power_index,
                                 &(indiv_power->sw_indiv_power_status))==FALSE)
    {
        return FALSE;
    }

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
    if (SYS_ENV_VM_GetPowerType(indiv_power->sw_indiv_power_unit_index,
                               indiv_power->sw_indiv_power_index,
                               &(indiv_power->sw_indiv_power_type))==FALSE)
    {
        return FALSE;
    }
#endif

    return TRUE;
}

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 *          sw_alarm->sw_alarm_input_index       --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name        --- description of alarm input
 *          sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetAlarmInputStatus(sw_alarm->sw_alarm_unit_index,
              &(sw_alarm->sw_alarm_status));

    ret_val |= SYS_ENV_VM_GetAlarmInputName(sw_alarm->sw_alarm_unit_index,
              sw_alarm->sw_alarm_input_index, sw_alarm->sw_alarm_input_name);

    return ret_val;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of next alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name  --- description of alarm input
 *          sw_alarm->sw_alarm_status      --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : Used for SNMP.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return FALSE;
    }
    if(sw_alarm->sw_alarm_input_index == SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
    {
        sw_alarm->sw_alarm_input_index = 1;
        sw_alarm->sw_alarm_unit_index++;
    }
    else
        sw_alarm->sw_alarm_input_index++;

    if(sw_alarm->sw_alarm_unit_index == 0)
        sw_alarm->sw_alarm_unit_index++;

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetAlarmInputStatus(sw_alarm->sw_alarm_unit_index,
              &(sw_alarm->sw_alarm_status));

    ret_val &= SYS_ENV_VM_GetAlarmInputName(sw_alarm->sw_alarm_unit_index,
              sw_alarm->sw_alarm_input_index, sw_alarm->sw_alarm_input_name);
    return ret_val;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwAlarmInputStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 * OUTPUT : sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSwAlarmInputStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetAlarmInputStatus(sw_alarm->sw_alarm_unit_index,
              &(sw_alarm->sw_alarm_status));

    return ret_val;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 *          sw_alarm->sw_alarm_unit_input_index  --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name        --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetAlarmInputName(sw_alarm->sw_alarm_unit_index,
              sw_alarm->sw_alarm_input_index, sw_alarm->sw_alarm_input_name);

    return ret_val;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_SetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to set name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 *          sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_SetAlarmInputName(sw_alarm->sw_alarm_unit_index,
              sw_alarm->sw_alarm_input_index, sw_alarm->sw_alarm_input_name);

    return ret_val;
}
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_MGR_GetRunningAlarmInputName
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get name of alarm input if it's different
 *          from default value
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT:  sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    int ret;
    BOOL_T ret_val;
    char alarm_input_buf[MAXSIZE_swAlarmInputName+1];

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* semantic check
     */
    if (sw_alarm == 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetAlarmInputName(sw_alarm->sw_alarm_unit_index,
              sw_alarm->sw_alarm_input_index, sw_alarm->sw_alarm_input_name);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (ret_val != TRUE)
    {
        ret =  SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        sprintf(alarm_input_buf, "ALARM_IN%ld", sw_alarm->sw_alarm_input_index);
        if(strcmp(sw_alarm->sw_alarm_input_name, alarm_input_buf) == 0)
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    return ret;
}

#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetMajorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get major alarm output status
 * INPUT  : sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT : sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * RETURN : TRUE if get successfully
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMajorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetMajorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status));
    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetMinorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get minor alarm output status
 * INPUT  : sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT : sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN : TRUE if get successfully
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMinorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetMinorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status));
    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get major and minor alarm output status
 * INPUT  : sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT : sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *          sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN : TRUE if get successfully
 * NOTES  : Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetMajorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status));

    ret_val = (ret_val && SYS_ENV_VM_GetMinorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status_2)));
    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get next major and minor alarm output status
 * INPUT  : sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT : sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *          sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN : TRUE if get successfully
 * NOTES  : Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (sw_alarm == NULL)
    {
        return FALSE;
    }

    sw_alarm->sw_alarm_unit_index++;

    if (FALSE == STKTPLG_POM_UnitExist(sw_alarm->sw_alarm_unit_index))
    {
        return FALSE;
    }

    /* action
     */
    ret_val = SYS_ENV_VM_GetMajorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status));

    ret_val = (ret_val && SYS_ENV_VM_GetMinorAlarmOutputStatus(sw_alarm->sw_alarm_unit_index,
                                                   &(sw_alarm->sw_alarm_status_2)));
    return ret_val;
}
#endif/* #if (SYS_CPNT_ALARM_DETECT == TRUE) */

#if (SYS_CPNT_POWER_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_PowerStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Power status changed callback function, register to sysdrv
 * INPUT   : unit   -- which unit
 *           power  -- which power
 *           status -- VAL_swIndivPowerStatus_notPresent
 *                     VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void    SYS_MGR_PowerStatusChanged_CallBack(UI32_T unit, UI32_T power, UI32_T status)
{
    UI32_T original_status;
    BOOL_T power_status_changed;

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        return;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return;
    }

    if (status != VAL_swIndivPowerStatus_green    &&
        status != VAL_swIndivPowerStatus_red      &&
        status != VAL_swIndivPowerStatus_notPresent )
    {
        return;
    }

    /* action
     */
    if (FALSE == SYS_ENV_VM_GetPowerStatus(unit, power, &original_status))
    {
        return;
    }
    else
    {
        if (original_status == status)
        {
            return;
        }
    }
    /* call sys_env to save database and Send trap*/
    SYS_ENV_VM_SetPowerStatus(unit, power, status);

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
    /* All of the system led are controlled by onlp_platmgrd when
     * SYS_CPNT_SYSDRV_USE_ONLP is TRUE
     */
    LED_PMGR_PowerStatusChanged(unit, power, status);
#endif

    return;
}
#endif

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_PowerTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Power type changed callback function, notified by sysdrv
 * INPUT   : unit   -- which unit
 *           power  -- which power
 *           type   -- SYS_HWCFG_COMMON_POWER_DC_N48_MODULE_TYPE
 *                     SYS_HWCFG_COMMON_POWER_DC_P24_MODULE_TYPE
 *                     SYS_HWCFG_COMMON_POWER_AC_MODULE_TYPE
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_MGR_PowerTypeChanged_CallBack(UI32_T unit, UI32_T power, UI32_T type)
{
    UI32_T org_sysmgr_type;
    UI32_T new_sysmgr_type;

    /* semantic check
     */
    if (FALSE == STKTPLG_OM_UnitExist(unit))
    {
        return;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return;
    }

    /* convert SYS_HWCFG_COMMON_POWER_XXX_MODULE_TYPE to
     * VAL_swIndivPowerType_XX
     */
    switch (type)
    {
        case SYS_HWCFG_COMMON_POWER_DC_N48_MODULE_TYPE:
            new_sysmgr_type=VAL_swIndivPowerType_DC_N48;
            break;
        case SYS_HWCFG_COMMON_POWER_DC_P24_MODULE_TYPE:
            new_sysmgr_type=VAL_swIndivPowerType_DC_P24;
            break;
        case SYS_HWCFG_COMMON_POWER_AC_MODULE_TYPE:
            new_sysmgr_type=VAL_swIndivPowerType_AC;
            break;
        default:
            printf("%s(%d): Unknown power type(%lu)=%lu\r\n", __FUNCTION__, __LINE__,
                (unsigned long)power, (unsigned long)type);
            return;
    }

    if (FALSE == SYS_ENV_VM_GetPowerType(unit, power, &org_sysmgr_type))
    {
        return;
    }
    else
    {
        if (new_sysmgr_type == org_sysmgr_type)
        {
            return;
        }
    }

    /* call sys_env to save database
     */
    SYS_ENV_VM_SetPowerType(unit, power, new_sysmgr_type);
    return;
}

#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_AlarmInputStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Alarm input status changed callback function,  register to sysdrv
 * INPUT   : unit               -- which unit
 *           status             -- VAL_alarmInputType_alarmInputType_1
 *                                 VAL_alarmInputType_alarmInputType_2
 *                                 VAL_alarmInputType_alarmInputType_3
 *                                 VAL_alarmInputType_alarmInputType_4
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_MGR_AlarmInputStatusChanged_CallBack(UI32_T unit, UI32_T status)
{
    UI32_T original_status;

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        return;
    }

    if ( status & ~(SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK))
    {
        return;
    }

    /* action
     */
    if (FALSE == SYS_ENV_VM_GetAlarmInputStatus(unit, &original_status))
    {
        return;
    }
    else
    {
        if (original_status == status)
        {
            return;
        }
    }

    /* call sys_env to save database and Send trap*/
    SYS_ENV_VM_SetAlarmInputStatus(unit, status);
    return;
}
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_MajorAlarmOutputStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Alarm output status changed callback function, register to sysdrv
 * INPUT   : unit               -- which unit
 *           status             -- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_MGR_MajorAlarmOutputStatusChanged_CallBack(UI32_T unit, UI32_T status)
{
    /* call sys_env to save database and Send trap if required
     * semantic check had been done in SYS_ENV_VM_SetMajorAlarmOutputStatus()
     */
    SYS_ENV_VM_SetMajorAlarmOutputStatus(unit, status);
#if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV==TRUE)
    LED_PMGR_SetMajorAlarmOutputLed(unit, status);
#endif

    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_MinorAlarmOutputStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Alarm output status changed callback function, register to sysdrv
 * INPUT   : unit               -- which unit
 *           status             -- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE       : None
 * -------------------------------------------------------------------------*/
void SYS_MGR_MinorAlarmOutputStatusChanged_CallBack(UI32_T unit, UI32_T status)
{
    /* call sys_env to save database and Send trap if required
     * semantic check had been done in SYS_ENV_VM_SetMinorAlarmOutputStatus()
     */
    SYS_ENV_VM_SetMinorAlarmOutputStatus(unit, status);
#if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV==TRUE)
    LED_PMGR_SetMinorAlarmOutputLed(unit, status);
#endif
    return;
}
#endif /* #if (SYS_CPNT_ALARM_DETECT == TRUE) */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Notify_FanStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Use the IPC message to call LED mgr to set the status of fan LED.
 * INPUT    : unit   -- unit id
 *            fan    -- fan id
 *            status -- fan status(VAL_switchFanStatus_ok/VAL_switchFanStatus_failure)
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : Base on the architecture of linux platform, the SYS_MGR shall not use callback
 *            function to notify LED_MGR.
 * ---------------------------------------------------------------------
 */
static void SYS_MGR_Notify_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status)
{
    LED_PMGR_SetFanFailLED (unit, fan, status);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_FanStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- call back function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchFanStatus_ok/VAL_switchFanStatus_failure.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_FanStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T fan, UI32_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(FanStatusChanged_callbacklist);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_FanStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Fan status changed sys callback message handler function
 * INPUT   : unit   -- which unit
 *           port   -- which fan
 *           status -- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  SYS_MGR_FanStatusChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T status)
{
    UI32_T original_status;

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    if (status != VAL_switchFanStatus_ok && status != VAL_switchFanStatus_failure)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    if (FALSE == SYS_ENV_VM_GetFanStatus(unit, fan, &original_status))
    {
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
        return FALSE;
    }
    else
    {
        if (original_status == status)
        {
            SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
            return FALSE;
        }
    }

    SYS_ENV_VM_SetFanStatus(unit, fan, status);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    SYS_MGR_Notify_FanStatusChanged(unit, fan, status);
    return TRUE;

}

void SYS_MGR_Register_FanSpeedChanged_CallBack(void (*fun)(UI32_T unit, UI32_T  fan, UI32_T speed))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(FanSpeedChanged_callbacklist);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_FanSpeedChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Fan speed changed callback function, callback from sysdrv
 * INPUT   : unit   -- which unit
 *           fan    -- which fan
 *           speed  -- fan speed
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  SYS_MGR_FanSpeedChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T speed)
{
    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    SYS_ENV_VM_SetFanSpeed(unit, fan, speed);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return TRUE;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_SimulateFanSpeedChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Simulate Fan status changed callback function for backdoor use only
 * INPUT   : unit   -- which unit
 *           fan    -- which fan
 *           speed  -- speed
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_MGR_SimulateFanSpeedChanged(UI32_T unit, UI32_T fan, UI32_T speed)
{
    SYS_MGR_FanSpeedChanged_CallBack(unit, fan, speed);
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    BOOL_T ret_val;
    UI32_T fan_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return FALSE;
    }

    /* semantic check
     */
    if (switch_fan_status == 0)
    {

        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(switch_fan_status->switch_unit_index))
    {

        return FALSE;
    }


    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetFanNum(switch_fan_status->switch_unit_index,&fan_number))
    {

        return FALSE;
    }

    if (switch_fan_status->switch_fan_index == 0 ||
        switch_fan_status->switch_fan_index > fan_number)
    {

        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);

    ret_val = SYS_ENV_VM_GetFanStatus(switch_fan_status->switch_unit_index,
                                      switch_fan_status->switch_fan_index,
                                      &(switch_fan_status->switch_fan_status));

    ret_val &= SYS_ENV_VM_GetFanFailCounter(switch_fan_status->switch_unit_index,
                                      switch_fan_status->switch_fan_index,
                                      &(switch_fan_status->switch_fan_fail_counter));

    ret_val &= SYS_ENV_VM_GetFanSpeed(switch_fan_status->switch_unit_index,
                                     switch_fan_status->switch_fan_index,
                                     &(switch_fan_status->switch_fan_oper_speed));
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure

 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    BOOL_T ret_val;
    UI32_T unit_id;
    UI32_T fan_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (switch_fan_status == 0)
    {
        return FALSE;
    }

    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetFanNum(switch_fan_status->switch_unit_index,&fan_number))
    {
        return FALSE;
    }

#ifdef BLANC

    if (switch_fan_status->switch_fan_index != fan_number)
    {
        unit_id = switch_fan_status->switch_unit_index;

        if (0 == unit_id)
        {
            if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
            {
                return FALSE;
            }
        }

        switch_fan_status->switch_fan_index++;
        switch_fan_status->switch_unit_index = unit_id;
    }
    else
    {
        unit_id = switch_fan_status->switch_unit_index;
        if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
        {
                return FALSE;
            }

        switch_fan_status->switch_fan_index  = 1;
        switch_fan_status->switch_unit_index = unit_id;
    }
#else
    if (switch_fan_status->switch_unit_index == 0)
    {
        switch_fan_status->switch_unit_index = 1;
        switch_fan_status->switch_fan_index  = 1;
    }
    else if (switch_fan_status->switch_fan_index == fan_number)
    {
        switch_fan_status->switch_unit_index ++;
        switch_fan_status->switch_fan_index = 1;
    }
    else
    {
        switch_fan_status->switch_fan_index ++;
    }

    if (FALSE == STKTPLG_POM_UnitExist(switch_fan_status->switch_unit_index))
    {

        return FALSE;
    }
#endif

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);

    ret_val = SYS_ENV_VM_GetFanStatus(switch_fan_status->switch_unit_index,
                                      switch_fan_status->switch_fan_index,
                                      &(switch_fan_status->switch_fan_status));
    ret_val &= SYS_ENV_VM_GetFanFailCounter(switch_fan_status->switch_unit_index,
                                      switch_fan_status->switch_fan_index,
                                      &(switch_fan_status->switch_fan_fail_counter));

    ret_val &= SYS_ENV_VM_GetFanSpeed(switch_fan_status->switch_unit_index,
                                     switch_fan_status->switch_fan_index,
                                     &(switch_fan_status->switch_fan_oper_speed));

    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    BOOL_T ret_val;
    UI32_T fan_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (switch_fan_status == 0)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(switch_fan_status->switch_unit_index))
    {
        return FALSE;
    }
    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetFanNum(switch_fan_status->switch_unit_index,&fan_number))
    {
        return FALSE;
    }

    if (switch_fan_status->switch_fan_index == 0 ||
        switch_fan_status->switch_fan_index > fan_number)
    {

        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetFanSpeed(switch_fan_status->switch_unit_index,
                                     switch_fan_status->switch_fan_index,
                                     &(switch_fan_status->switch_fan_oper_speed));
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    BOOL_T ret_val;
    UI32_T unit_id;
    UI32_T fan_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (switch_fan_status == 0)
    {
        return FALSE;
    }

    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetFanNum(switch_fan_status->switch_unit_index,&fan_number))
    {
        return FALSE;
    }

    if (switch_fan_status->switch_fan_index != fan_number)
    {
        unit_id = switch_fan_status->switch_unit_index;

        if (0 == unit_id)
        {
            if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
            {
                return FALSE;
            }
        }

        switch_fan_status->switch_fan_index++;
        switch_fan_status->switch_unit_index = unit_id;
    }
    else
    {
        unit_id = switch_fan_status->switch_unit_index;
        if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
        {
                return FALSE;
        }

        switch_fan_status->switch_fan_index  = 1;
        switch_fan_status->switch_unit_index = unit_id;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetFanSpeed(switch_fan_status->switch_unit_index,
                                     switch_fan_status->switch_fan_index,
                                     &(switch_fan_status->switch_fan_oper_speed));
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To set exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 *            switch_fan_status->switch_fan_status --- The status of the fan
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) && \
    (((SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) && (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP != 0)) || \
     (SYS_HWCFG_FAN_CONTROLLER_TYPE != SYS_HWCFG_FAN_NONE) \
    )
    return SYSDRV_FAN_SetSpeed(switch_fan_status->switch_fan_index, switch_fan_status->switch_fan_speed);
#else
    return TRUE;
#endif

}

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_MGR_SetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to set force fan speed full
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetFanSpeedForceFull(BOOL_T mode)
{
    BOOL_T ret_val;
    BOOL_T mode_tmp;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetFanSpeedForceFull(&mode_tmp);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    if (mode == mode_tmp)
    {
        return TRUE;
    }
    else
    {
        ret_val = SYSDRV_SetFanSpeedForceFull(mode);
        if (ret_val != TRUE)
        {
            return FALSE;
        }
        else
        {
            SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
            ret_val = SYS_ENV_VM_SetFanSpeedForceFull(mode);
            SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
        }
    }

    return ret_val;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_MGR_GetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetFanSpeedForceFull(BOOL_T *mode)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (mode == 0)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetFanSpeedForceFull(mode);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_MGR_GetRunningFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get running force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode  --- TRUE/FALSE.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningFanSpeedForceFull(BOOL_T *mode)
{
    int ret;
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* semantic check
     */
    if (mode == 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetFanSpeedForceFull(mode);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (ret_val != TRUE)
        ret =  SYS_TYPE_GET_RUNNING_CFG_FAIL;
    else if (*mode != SYS_DFLT_SYSMGMT_FAN_SPEED_FORCE_FULL)
        ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return ret;
}
#endif
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
#if 0 /* comment out the code because no upper layer CSC to notify now */
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_Notify_ThermalStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Notify for thermal status change.
 * INPUT    : unit        -  unit id
 *            thermal_idx -  thermal sensor index
 *            temperature -  temperature got from the specified thermal sensor
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : Thermal status is a generic name for all of the information
 *            which might have in a thermal sensor. For now, thermal status
 *            only contains temperature data.
 * ---------------------------------------------------------------------
 */
static void SYS_MGR_Notify_ThermalStatusChanged(UI32_T unit, UI32_T thermal_idx, I32_T temperature)
{
    SYS_CALLBACK_MGR_ThermalStatusChanged_CallBack(SYS_MODULE_SYSMGMT,unit, thermal_idx, temperature);
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_ThermalStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- call back function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchFanStatus_ok/VAL_switchFanStatus_failure.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_ThermalStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T thermal, UI32_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(ThermalStatusChanged_callbacklist);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_ThermalStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Thermal status changed callback function
 * INPUT   : unit        -- which unit
 *           thermal_idx -- which thermal index (starts from 1)
 *           temperature -- thermal temperature
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Thermal status is a generic name for all of the information
 *           which might have in a thermal sensor. For now, thermal status
 *           only contains temperature data.
 * -------------------------------------------------------------------------*/
BOOL_T  SYS_MGR_ThermalStatusChanged_CallBack(UI32_T unit, UI32_T thermal_idx, I32_T temperature)
{
    BOOL_T abnormal_status_changed, is_abnormal;

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        return FALSE;
    }

    if (thermal_idx == 0 || thermal_idx > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    SYS_ENV_VM_SetThermalStatus(unit, thermal_idx, temperature, &abnormal_status_changed, &is_abnormal);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    #if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) /* led is controlled by ONLP when SYS_CPNT_SYSDRV_USE_ONLP is TRUE */
    if (abnormal_status_changed==TRUE)
    {
        LED_PMGR_ThermalStatusChanged(unit, thermal_idx, is_abnormal);
    }
    #endif

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the thermal status such as temperature from the specified
 *            thermal sensor index.
 * INPUT    : switch_thermal_entry_p->switch_unit_index --- Which unit
 *            switch_thermal_entry_p->switch_thermal_index  --- Which thermal
 * OUTPUT   : switch_thermal_entry_p->switch_thermal_temp_value --- The thermal
 *            temperature
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */

BOOL_T SYS_MGR_GetThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status)
{
    BOOL_T ret_val;
    UI32_T thermal_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (switch_thermal_status == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(switch_thermal_status->switch_unit_index))
    {
        return FALSE;
    }

    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetThermalNum(switch_thermal_status->switch_unit_index,&thermal_number))
    {
        return FALSE;
    }

    if (switch_thermal_status->switch_thermal_index == 0 ||
        switch_thermal_status->switch_thermal_index > thermal_number)
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetThermalStatus(switch_thermal_status->switch_unit_index,
                                          switch_thermal_status->switch_thermal_index,
                                          &(switch_thermal_status->switch_thermal_temp_value));
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get thermal status such as temperature from the thermal sensor
 *            index which is next to the specified thermal sensor index.
 * INPUT    : switch_thermal_entry_p->switch_unit_index --- Which unit
 *            switch_thermal_entry_p->switch_thermal_index  --- Which thermal
 * OUTPUT   : switch_thermal_entry_p->switch_thermal_temp_value --- The thermal
 *            temperature
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status)
{
    BOOL_T ret_val;
    UI32_T unit_id;
    UI32_T thermal_number;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (switch_thermal_status == NULL)
    {
        return FALSE;
    }

    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetThermalNum(switch_thermal_status->switch_unit_index,&thermal_number))
    {
        return FALSE;
    }

    if (switch_thermal_status->switch_thermal_index != thermal_number)
    {
        unit_id = switch_thermal_status->switch_unit_index;

        if (0 == unit_id)
        {
            if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
            {
                return FALSE;
            }
        }

        switch_thermal_status->switch_thermal_index++;
        switch_thermal_status->switch_unit_index = unit_id;
    }
    else
    {
        unit_id = switch_thermal_status->switch_unit_index;

            if (FALSE == STKTPLG_POM_GetNextUnit(&unit_id))
            {
                return FALSE;
            }

        switch_thermal_status->switch_thermal_index = 1;
        switch_thermal_status->switch_unit_index    = unit_id;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetThermalStatus(switch_thermal_status->switch_unit_index,
                                          switch_thermal_status->switch_thermal_index,
                                          &(switch_thermal_status->switch_thermal_temp_value));
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDefaultSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the default switch thermal action entry for rising_threshold,falling_threshold,action.
 * INPUT    : *entry --- output buffer of default entry pointer
 * OUTPUT   : *entry --- default entry pointer
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 *            Don't care key index,just default return rising_threshold,falling_threshold,action.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetDefaultSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (entry == NULL)
    {
        return FALSE;
    }

    /* action
     */

    switch (entry->thermal_index)
    {
    case 1:
        entry->rising_threshold = SYS_ADPT_THERMAL_0_THRESHOLD_UP;
        entry->falling_threshold = SYS_ADPT_THERMAL_0_THRESHOLD_DOWN;
        break;
        #if (SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT >= 2)
    case 2:
        entry->rising_threshold = SYS_ADPT_THERMAL_1_THRESHOLD_UP;
        entry->falling_threshold = SYS_ADPT_THERMAL_1_THRESHOLD_DOWN;
        break;
        #endif
    default:
        entry->rising_threshold = SYS_ADPT_THERMAL_0_THRESHOLD_UP;
        entry->falling_threshold = SYS_ADPT_THERMAL_0_THRESHOLD_DOWN;
        entry->action = L_CVRT_SNMP_BIT_VALUE_32(SYS_DFLT_THERMAL_ACTION);
        return FALSE;
        break;
    }
    entry->action = L_CVRT_SNMP_BIT_VALUE_32(SYS_DFLT_THERMAL_ACTION);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    BOOL_T ret_val;
    SYS_ENV_VM_SwitchThermalActionEntry_T action_entry;
    UI32_T thermal_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (entry == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(entry->unit_index))
    {
        return FALSE;
    }

    /*
    Note:
     Replace max number with real number for max number is not equal to real number sometime.
     Modified by Jason Yang at 2004-10-13.
    */
    if (FALSE == SYSDRV_GetThermalNum(entry->unit_index,&thermal_number))
    {
        return FALSE;
    }

    if (entry->thermal_index == 0 ||
        entry->thermal_index > thermal_number/*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT*/)
    {
        return FALSE;
    }

    if (entry->action_index == 0 ||
        entry->action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
    {
        return FALSE;
    }
    /* action
     */
    action_entry.unit_index = entry->unit_index;
    action_entry.thermal_index = entry->thermal_index;
    action_entry.action_index = entry->action_index;
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetSwitchThermalActionEntry(&action_entry);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (ret_val == TRUE)
    {
        entry->unit_index = action_entry.unit_index;
        entry->thermal_index = action_entry.thermal_index;
        entry->action_index = action_entry.action_index;
        entry->rising_threshold = action_entry.rising_threshold;
        entry->falling_threshold = action_entry.falling_threshold;
        entry->action = action_entry.action;
        entry->status = action_entry.status;
    }

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get next switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetNextSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    BOOL_T ret_val;
    SYS_ENV_VM_SwitchThermalActionEntry_T action_entry;
    UI32_T thermal_number;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (entry == NULL)
    {
        return FALSE;
    }

    if ((entry->unit_index == 0)    &&
       (entry->thermal_index == 0) &&
       (entry->action_index == 0))
    {
        /* get first entry, don't check */
    }
    else
    {
        if (FALSE == STKTPLG_POM_UnitExist(entry->unit_index))
        {
            return FALSE;
        }
        /*
        Note:
         Replace max number with real number for max number is not equal to real number sometime.
         Modified by Jason Yang at 2004-10-13.
        */
        if (FALSE == SYSDRV_GetThermalNum(entry->unit_index,&thermal_number))
        {
            return FALSE;
        }

        if (entry->thermal_index == 0 ||
            entry->thermal_index >= thermal_number/*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT*/)
        {
            return FALSE;
        }

        if (entry->action_index == 0 ||
            entry->action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
        {
            return FALSE;
        }
    }

    /* action
     */
    action_entry.unit_index = entry->unit_index;
    action_entry.thermal_index = entry->thermal_index;
    action_entry.action_index = entry->action_index;
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_GetNextSwitchThermalActionEntry(&action_entry);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (ret_val == TRUE)
    {
        entry->unit_index = action_entry.unit_index;
        entry->thermal_index = action_entry.thermal_index;
        entry->action_index = action_entry.action_index;
        entry->rising_threshold = action_entry.rising_threshold;
        entry->falling_threshold = action_entry.falling_threshold;
        entry->action = action_entry.action;
        entry->status = action_entry.status;
    }

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T action_entry;
    BOOL_T new_flag;
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (entry == NULL)
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_POM_UnitExist(entry->unit_index))
    {
        return FALSE;
    }

    if ((entry->thermal_index == 0) ||
        (entry->thermal_index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if ((entry->action_index == 0) ||
        (entry->action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if ((entry->action != 0)    &&
       (entry->action != L_CVRT_SNMP_BIT_VALUE_32(VAL_switchThermalActionAction_trap)))
    {
        return FALSE;
    }

    if ((entry->status != VAL_switchThermalActionStatus_valid) &&
       (entry->status != VAL_switchThermalActionStatus_invalid))
    {
        return FALSE;
    }
    /* action
     */
    new_flag = FALSE;
    action_entry.unit_index = entry->unit_index;
    action_entry.thermal_index = entry->thermal_index;
    action_entry.action_index = entry->action_index;
    if (SYS_ENV_VM_GetSwitchThermalActionEntry(&action_entry) ==    TRUE)
    {
        if ((action_entry.unit_index    == entry->unit_index) &&
           (action_entry.thermal_index == entry->thermal_index) &&
           (action_entry.action_index == entry->action_index) &&
           (action_entry.rising_threshold == entry->rising_threshold) &&
           (action_entry.falling_threshold == entry->falling_threshold) &&
           (action_entry.action == entry->action) &&
           (action_entry.status == entry->status))
        {
            return TRUE;
        }
        else
        {
            new_flag = FALSE;
        }
    }
    else
    {
        new_flag = TRUE;
    }

    if (new_flag    == TRUE)
    {
        /* create new entry */
        SYS_ENV_VM_SwitchThermalActionEntry_T new_entry;

        new_entry.unit_index = entry->unit_index;
        new_entry.thermal_index = entry->thermal_index;
        new_entry.action_index = entry->action_index;
        new_entry.rising_threshold = entry->rising_threshold;
        new_entry.falling_threshold = entry->falling_threshold;
        new_entry.action = entry->action;
        new_entry.status = entry->status;
        new_entry.is_default_entry = FALSE;
        SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
        ret_val = SYS_ENV_VM_SetSwitchThermalActionEntry(&new_entry);
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    }
    else
    {
        if (action_entry.is_default_entry ==    TRUE)
        {
            /* default entry cannot be modified */
            return FALSE;
        }

        /* replace exist entry */
        SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
        ret_val = SYS_ENV_VM_SetSwitchThermalActionEntry(&action_entry);
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    }

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action rising threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            rising_threshold -- rising threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T action_index, UI32_T rising_threshold)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit_index))
    {
        return FALSE;
    }

    if ((thermal_index == 0) ||
        (thermal_index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if (action_index == 0 ||
        action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if ((rising_threshold < SYS_ADPT_MIN_THERMAL_ACTION_RISING_THRESHOLD) ||
        (rising_threshold > SYS_ADPT_MAX_THERMAL_ACTION_RISING_THRESHOLD))
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_SetSwitchThermalActionRisingThreshold(unit_index, thermal_index,
                                                               action_index, rising_threshold);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action falling threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            falling_threshold -- falling threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T action_index, UI32_T falling_threshold)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit_index))
    {
        return FALSE;
    }

    if ((thermal_index == 0) ||
        (thermal_index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if (action_index == 0 ||
        action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if ((falling_threshold < SYS_ADPT_MIN_THERMAL_ACTION_FALLING_THRESHOLD) ||
        (falling_threshold > SYS_ADPT_MAX_THERMAL_ACTION_FALLING_THRESHOLD))
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_SetSwitchThermalActionFallingThreshold(unit_index, thermal_index,
                                                                action_index, falling_threshold);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry action.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            action -- entry action
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index, UI32_T action_index, UI32_T action)
{
    BOOL_T ret_val;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit_index))
    {
        return FALSE;
    }

    if ((thermal_index == 0) ||
        (thermal_index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if (action_index == 0 ||
        action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if ((action != 0) &&
        (action != L_CVRT_SNMP_BIT_VALUE_32(VAL_switchThermalActionAction_trap)))
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_SetSwitchThermalActionAction(unit_index, thermal_index, action_index, action);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret_val;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry status.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            status -- entry status
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index, UI32_T action_index, UI32_T status)
{
    BOOL_T ret_val;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    /* semantic check
     */
    if (FALSE == STKTPLG_POM_UnitExist(unit_index))
    {
        return FALSE;
    }

    if ((thermal_index == 0) ||
        (thermal_index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        return FALSE;
    }

    if (action_index == 0 ||
        action_index > SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if ((status != VAL_switchThermalActionStatus_valid) &&
        (status > VAL_switchThermalActionStatus_invalid))
    {
        return FALSE;
    }

    /* action
     */
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    ret_val = SYS_ENV_VM_SetSwitchThermalActionStatus(unit_index, thermal_index, action_index, status);
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);


    return ret_val;
}
#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE)*/

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set console login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */

BOOL_T SYS_MGR_SetConsoleLoginTimeOut(UI32_T time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((time_out_value < SYS_ADPT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT_MIN) ||
        (time_out_value > SYS_ADPT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT_MAX))
    {
        /* UIMSG_MGR_SetErrorCode(UIMSG_TYPE_SYS_MGR_OUT_OF_THRESHOLD); */
        return FALSE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    sys_mgr_console_cfg.login_timeout = time_out_value;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set telnet login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetTelnetLoginTimeOut(UI32_T time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((time_out_value < SYS_ADPT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT_MIN) ||
        (time_out_value > SYS_ADPT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT_MAX))
    {
        /* UIMSG_MGR_SetErrorCode(UIMSG_TYPE_SYS_MGR_OUT_OF_THRESHOLD); */
        return FALSE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    sys_mgr_telnet_cfg.login_timeout = time_out_value;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value   - nactive time out
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningConsoleLoginTimeOut(UI32_T *time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    *time_out_value = sys_mgr_console_cfg.login_timeout;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (SYS_DFLT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT != *time_out_value)
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningTelnetLoginTimeOut(UI32_T *time_out_value)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    *time_out_value = sys_mgr_telnet_cfg.login_timeout;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    if (SYS_DFLT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT != *time_out_value)
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysDescr
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system description.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- system description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_sysDescr+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSysDescr(UI32_T unit_id, UI8_T *sys_descrption)
{
    UI32_T board_id;

    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {
            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(sys_descrption, 0, MAXSIZE_sysDescr + 1);
    if (board_id < sysmgr_board_count)
    {
        strncpy((char*)sys_descrption, sysmgr_swinfo.board_info[board_id].sys_descr, MAXSIZE_sysDescr);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysObjectID
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system object ID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_oid -- system OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetSysObjectID(UI32_T unit_id, UI8_T *sys_oid)
{
    UI32_T board_id;

    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {
            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(sys_oid, 0, SYS_ADPT_MAX_OID_STRING_LEN + 1);
    if (board_id < sysmgr_board_count)
    {
        strncpy((char*)sys_oid, sysmgr_swinfo.board_info[board_id].sys_oid, SYS_ADPT_MAX_OID_STRING_LEN);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetProductName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- product name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdName+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetProductName(UI32_T unit_id, UI8_T *product_name)
{
    UI32_T board_id;

    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {
            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(product_name, 0, MAXSIZE_swProdName + 1);
    if (board_id < sysmgr_board_count)
    {
        strncpy((char*)product_name, sysmgr_swinfo.board_info[board_id].product_name, MAXSIZE_swProdName);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDeviceName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get device name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : device_name    -- device name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_DEVICE_NAME_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetDeviceName(UI32_T unit_id, UI8_T *device_name)
{
    UI32_T board_id;


    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {

            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(device_name, 0, SYS_ADPT_DEVICE_NAME_STRING_LEN+1);
    switch (board_id)
    {
    case 0:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID0,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 1:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID1,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 2:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID2,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 3:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID3,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 4:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID4,SYS_ADPT_DEVICE_NAME_STRING_LEN);
         break;

    case 5:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID5,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 6:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID6,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    case 7:
        strncpy((char *)device_name, SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID7,SYS_ADPT_DEVICE_NAME_STRING_LEN);
        break;

    default:

        return FALSE;
    }


    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPrivateMibRoot
 * ---------------------------------------------------------------------
 * PURPOSE  : Get MIB root OID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : mib_root -- MIB root OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetPrivateMibRoot(UI32_T unit_id, UI8_T *mib_root)
{
    UI32_T board_id;


    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {

            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(mib_root, 0, SYS_ADPT_MAX_OID_STRING_LEN + 1);
    if (board_id < sysmgr_board_count)
    {
        strncpy((char*)mib_root, sysmgr_swinfo.board_info[board_id].private_mib_root, SYS_ADPT_MAX_OID_STRING_LEN);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void SYS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* no port database, do nothing */
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void SYS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* no port database, do nothing */
    return;
}
#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_XFPModuleStatusChanged_Callback
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- call back function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchFanStatus_ok/VAL_switchFanStatus_failure.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_XFPModuleStatusChanged_Callback(void (*fun)(UI32_T unit, UI32_T port, BOOL_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(XFPModuleStatusChanged_callbacklist);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Notify_XFPModuleStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- call back function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchFanStatus_ok/VAL_switchFanStatus_failure.
 * ---------------------------------------------------------------------
 */
static void SYS_MGR_Notify_XFPModuleStatusChanged(UI32_T unit, UI32_T port, BOOL_T status)
{
    SYS_CALLBACK_MGR_XFPModuleStatusChanged_CallBack(SYS_MODULE_SYSMGMT,unit, port, status);

//  SYS_TYPE_CallBack_T  *fun_list;

//  for(fun_list=XFPModuleStatusChanged_callbacklist; fun_list; fun_list=fun_list->next)
//      fun_list->func(unit, port, status);
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_XFPModuleStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: XFP module  status changed callback function, register to sysdrv
 * INPUT   : unit    -- which unit
 *           thermal -- which port
 *           status  -- insert or remove
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SYS_MGR_XFPModuleStatusChanged_CallBack(UI32_T unit, UI32_T port, BOOL_T status)
{
    SYS_MGR_Notify_XFPModuleStatusChanged(unit, port, status);
    return;
}
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_PowerStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a power status is changed
 *            the registered function will be called.
 * INPUT    : fun -- callback function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_swIndivPowerStatus_notPresent
 *            VAL_swIndivPowerStatus_green
 *            VAL_swIndivPowerStatus_red
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_PowerStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T power, UI32_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PowerStatusChanged_callbacklist);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetModelName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get model name by board ID
 * INPUT    : unit_id     -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                           means local unit.
 * OUTPUT   : model_name  -- model name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_MODEL_NAME_SIZE + 1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetModelName(UI32_T unit_id, UI8_T *model_name)
{
    UI32_T board_id;

    if (SYS_VAL_LOCAL_UNIT_ID == unit_id)
    {
        if (FALSE == STKTPLG_POM_GetMyUnitID(&unit_id))
        {
            return FALSE;
        }
    }

    if (TRUE != STKTPLG_POM_GetUnitBoardID(unit_id, &board_id))
    {
        return FALSE;
    }

    memset(model_name, 0, SYS_ADPT_MAX_MODEL_NAME_SIZE + 1);
    if (board_id < sysmgr_board_count)
    {
        strncpy((char*)model_name, sysmgr_swinfo.board_info[board_id].model_name, SYS_ADPT_MAX_MODEL_NAME_SIZE);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetProductManufacturer
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product manufacturer.
 * INPUT    : none.
 * OUTPUT   : prod_manufacturer -- product manufacturer.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdManufacturer+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetProductManufacturer(char* prod_manufacturer)
{
    strncpy(prod_manufacturer, sysmgr_swinfo.product_manufacturer, MAXSIZE_swProdManufacturer);
    prod_manufacturer[MAXSIZE_swProdManufacturer] = '\0';
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetProductDescription
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product description.
 * INPUT    : none.
 * OUTPUT   : prod_descrption -- product description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdDescription+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetProductDescription(char* prod_descrption)
{
    strncpy(prod_descrption, sysmgr_swinfo.product_description, MAXSIZE_swProdDescription);
    prod_descrption[MAXSIZE_swProdDescription] = '\0';
    return TRUE;
}

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_CpuUtilizationMonitoringProcess
 * ---------------------------------------------------------------------
 * PURPOSE  : Update CPU utilization info.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_CpuUtilizationMonitoringProcess(void)
{
    static UI32_T accumulated_busy_ticks = 0;
    static UI32_T accumulated_idle_ticks = 0;

    SYS_MGR_CpuUtilizationRecord_T tmp_cpu_util_record;
    UI32_T cpu_util;
    UI32_T busy_ticks, idle_ticks, delta_busy_ticks, delta_idle_ticks, delta_during_ticks;
    UI32_T precise_usage;
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
    TRAP_EVENT_TrapData_T data;
#endif

    /* get current CPU usage
     */
    if (SYSFUN_GetCpuUsage(&busy_ticks, &idle_ticks) != SYSFUN_OK)
    {
        return;
    }

    /* at the first time, just get current accumulated ticks,
     * no need to update cpu usage record.
     */
    if (accumulated_busy_ticks == 0 && accumulated_idle_ticks == 0)
    {
        accumulated_busy_ticks = busy_ticks;
        accumulated_idle_ticks = idle_ticks;
        return;
    }

    delta_busy_ticks = busy_ticks - accumulated_busy_ticks;
    delta_idle_ticks = idle_ticks - accumulated_idle_ticks;
    delta_during_ticks = delta_busy_ticks + delta_idle_ticks;

    if (delta_during_ticks == 0)
    {
        return;
    }

    precise_usage = 100000 * delta_busy_ticks / delta_during_ticks;

    tmp_cpu_util_record.cpu_usage = precise_usage / 1000;
    tmp_cpu_util_record.cpu_usage_float = precise_usage % 1000;
    tmp_cpu_util_record.record_tick = SYSFUN_GetSysTick();

    accumulated_busy_ticks = busy_ticks;
    accumulated_idle_ticks = idle_ticks;

    /* check threshold
     */
    cpu_util = tmp_cpu_util_record.cpu_usage;

    if (sys_mgr_cpu_alarm.alarm_status)
    {
        if (cpu_util <= sys_mgr_cpu_alarm.falling_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_CPU_FALLING_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_cpu_alarm.alarm_end_time);
            sys_mgr_cpu_alarm.alarm_end_tick = SYS_TIME_GetSystemTicksBy10ms();
            sys_mgr_cpu_alarm.alarm_status = FALSE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n", __FUNCTION__, __LINE__, (unsigned long)sys_mgr_cpu_alarm.alarm_end_time, (unsigned long)cpu_util, (unsigned long)sys_mgr_cpu_alarm.falling_threshold);
            }
        }
    }
    else
    {
        if (cpu_util >= sys_mgr_cpu_alarm.rising_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_CPU_RAISE_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_cpu_alarm.alarm_start_time);
            sys_mgr_cpu_alarm.alarm_start_tick = SYS_TIME_GetSystemTicksBy10ms();
            sys_mgr_cpu_alarm.alarm_status = TRUE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n", __FUNCTION__, __LINE__, (unsigned long)sys_mgr_cpu_alarm.alarm_start_time, (unsigned long)cpu_util, (unsigned long)sys_mgr_cpu_alarm.rising_threshold);
            }
        }
    }

    /* update CPU usage record
     */
    if (sys_mgr_cpu_util_5sec_record_valid_count == 0)
        current_5sec_index = 0;
    else
        current_5sec_index = (current_5sec_index + 1) % SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY;

    sys_mgr_cpu_util_5sec_record[current_5sec_index] = tmp_cpu_util_record;

    if (sys_mgr_cpu_util_5sec_record_valid_count < SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY)
        sys_mgr_cpu_util_5sec_record_valid_count++;

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
    SYS_MGR_UpdateTaskCpuUtil(delta_idle_ticks);
#endif
}
#else
/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ComputeCpuUtilization
 * -----------------------------------------------------------------------------
 * PURPOSE  : Compute CPU utilization.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
static void SYS_MGR_ComputeCpuUtilization(void)
{
    static UI32_T accumulated_busy_ticks = 0;
    static UI32_T accumulated_idle_ticks = 0;

    SYS_MGR_CpuUtilizationRecord_T tmp_cpu_util_record;
    UI32_T busy_ticks, idle_ticks, delta_busy_ticks,
           delta_idle_ticks, delta_during_ticks;
    UI32_T precise_usage;

    /* get current CPU usage
     */
    if (SYSFUN_GetCpuUsage(&busy_ticks, &idle_ticks) != SYSFUN_OK)
    {
        return;
    }

    /* at the first time, just get current accumulated ticks,
     * no need to update cpu usage record.
     */
    if (accumulated_busy_ticks == 0 && accumulated_idle_ticks == 0)
    {
        accumulated_busy_ticks = busy_ticks;
        accumulated_idle_ticks = idle_ticks;
        return;
    }

    delta_busy_ticks = busy_ticks - accumulated_busy_ticks;
    delta_idle_ticks = idle_ticks - accumulated_idle_ticks;
    delta_during_ticks = delta_busy_ticks + delta_idle_ticks;

    if (delta_during_ticks == 0)
    {
        return;
    }

    precise_usage = 100000 * delta_busy_ticks / delta_during_ticks;

    tmp_cpu_util_record.cpu_usage = precise_usage / 1000;
    tmp_cpu_util_record.cpu_usage_float = precise_usage % 1000;
    tmp_cpu_util_record.record_tick = SYSFUN_GetSysTick();

    accumulated_busy_ticks = busy_ticks;
    accumulated_idle_ticks = idle_ticks;

    /* update CPU usage record
     */
    if (sys_mgr_cpu_util_record_valid_count == 0)
    {
        current_index = 0;
    }
    else
    {
        current_index = (current_index + 1) % SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY;
    }

    sys_mgr_cpu_util_record[current_index] = tmp_cpu_util_record;

    if (sys_mgr_cpu_util_record_valid_count < SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY)
    {
        sys_mgr_cpu_util_record_valid_count++;
    }

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
    SYS_MGR_UpdateTaskCpuUtil(delta_idle_ticks);
#endif

}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ComputeCpu5secUtilization
 * -----------------------------------------------------------------------------
 * PURPOSE  : Compute CPU utilization in 5 seconds.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
static void SYS_MGR_ComputeCpu5secUtilizaition(void)
{
    SYS_MGR_CpuUtilizationRecord_T tmp_cpu_util_record;
    UI32_T                         index;
    UI32_T                         sum, sum_float;

    sum       = 0;
    sum_float = 0;

    for (index = 0; index < SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY; index++)
    {
        sum       += sys_mgr_cpu_util_record[index].cpu_usage;
        sum_float += sys_mgr_cpu_util_record[index].cpu_usage_float;
    }

    tmp_cpu_util_record.cpu_usage = sum / SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY;
    tmp_cpu_util_record.cpu_usage_float =
        sum_float / SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY;
    tmp_cpu_util_record.record_tick =
        sys_mgr_cpu_util_record[SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY-1].record_tick;

    if (sys_mgr_cpu_util_5sec_record_valid_count == 0)
    {
        current_5sec_index = 0;
    }
    else
    {
        current_5sec_index =
            (current_5sec_index + 1) % SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY;
    }

    sys_mgr_cpu_util_5sec_record[current_5sec_index] = tmp_cpu_util_record;

    if (sys_mgr_cpu_util_5sec_record_valid_count <
            SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY)
    {
        sys_mgr_cpu_util_5sec_record_valid_count++;
    }
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_CheckCpuUtilAndSendTrap
 * -----------------------------------------------------------------------------
 * PURPOSE  : Check CPU utilization and send trap.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
static void SYS_MGR_CheckCpuUtilAndSendTrap(void)
{
    UI32_T cpu_util;
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
    TRAP_EVENT_TrapData_T data;
#endif

    cpu_util = sys_mgr_cpu_util_5sec_record[current_5sec_index].cpu_usage;

    if (sys_mgr_cpu_alarm.alarm_status)
    {
        if (cpu_util <= sys_mgr_cpu_alarm.falling_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_CPU_FALLING_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data,
                    TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_cpu_alarm.alarm_end_time);
            sys_mgr_cpu_alarm.alarm_end_tick = SYS_TIME_GetSystemTicksBy10ms();
            sys_mgr_cpu_alarm.alarm_status = FALSE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n",
                        __FUNCTION__, __LINE__,
                        (unsigned long)sys_mgr_cpu_alarm.alarm_end_time, (unsigned long)cpu_util,
                        (unsigned long)sys_mgr_cpu_alarm.falling_threshold);
            }
        }
    }
    else
    {
        if (cpu_util >= sys_mgr_cpu_alarm.rising_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_CPU_RAISE_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data,
                    TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_cpu_alarm.alarm_start_time);
            sys_mgr_cpu_alarm.alarm_start_tick = SYS_TIME_GetSystemTicksBy10ms();
            sys_mgr_cpu_alarm.alarm_status = TRUE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n",
                        __FUNCTION__, __LINE__,
                        (unsigned long)sys_mgr_cpu_alarm.alarm_start_time, (unsigned long)cpu_util,
                        (unsigned long)sys_mgr_cpu_alarm.rising_threshold);
            }
        }
    }
}

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ToggleCpuGuardDebugFlag
 * -----------------------------------------------------------------------------
 * PURPOSE  : Toggle CPU guard debug flag.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
void SYS_MGR_ToggleCpuGuardDebugFlag(void)
{
    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    cpu_guard.debug_flag = cpu_guard.debug_flag ? FALSE : TRUE;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardDebugFlag
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get CPU guard debug flag.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardDebugFlag(void)
{
    return cpu_guard.debug_flag;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set high watermark.
 * INPUT    : None.
 * OUTPUT   : watermark.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardHighWatermark(UI32_T watermark)
{
    BOOL_T ret = FALSE;

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    if (cpu_guard.config.watermark_low < watermark &&
        SYS_ADPT_CPU_UTILIZATION_WATERMARK_HIGH >= watermark)
    {

        cpu_guard.config.watermark_high = watermark;
        ret = TRUE;
    }
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get high watermark.
 * INPUT    : None.
 * OUTPUT   : watermark.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardHighWatermark(UI32_T *watermark)
{
    if (watermark == NULL)
    {
        return FALSE;
    }

    *watermark = cpu_guard.config.watermark_high;
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set low watermark.
 * INPUT    : None.
 * OUTPUT   : watermark.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardLowWatermark(UI32_T watermark)
{
    BOOL_T ret = FALSE;

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    if (cpu_guard.config.watermark_high > watermark &&
        SYS_ADPT_CPU_UTILIZATION_WATERMARK_LOW <= watermark)
    {
        cpu_guard.config.watermark_low = watermark;
        ret = TRUE;
    }
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get low watermark.
 * INPUT    : None.
 * OUTPUT   : watermark.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardLowWatermark(UI32_T *watermark)
{
    if (watermark == NULL)
    {
        return FALSE;
    }

    *watermark = cpu_guard.config.watermark_low;
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set max threshold.
 * INPUT    : None.
 * OUTPUT   : threshold.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardMaxThreshold(UI32_T threshold)
{
    BOOL_T ret = FALSE;

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    if (cpu_guard.config.threshold_min < threshold &&
        SYS_ADPT_CPU_GUARD_THRESHOLD_MAX >= threshold)
    {

        cpu_guard.config.threshold_max = threshold;
        ret = TRUE;
    }
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get max threshold.
 * INPUT    : None.
 * OUTPUT   : threshold.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardMaxThreshold(UI32_T *threshold)
{
    if (threshold == NULL)
    {
        return FALSE;
    }

    *threshold = cpu_guard.config.threshold_max;
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set min threshold.
 * INPUT    : None.
 * OUTPUT   : threshold.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardMinThreshold(UI32_T threshold)
{
    BOOL_T ret = FALSE;

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    if (cpu_guard.config.threshold_max > threshold &&
        SYS_ADPT_CPU_GUARD_THRESHOLD_MIN <= threshold)
    {
        cpu_guard.config.threshold_min = threshold;
        ret = TRUE;
    }
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get min threshold.
 * INPUT    : None.
 * OUTPUT   : threshold.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardMinThreshold(UI32_T *threshold)
{
    if (threshold == NULL)
    {
        return FALSE;
    }

    *threshold = cpu_guard.config.threshold_min;
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardStatus(BOOL_T status)
{
    if (cpu_guard.config.status == status)
    {
        return TRUE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    cpu_guard.config.status = status;

    /* Disable CPU guard, reset cpu packet rate to default
     */
    if (status == FALSE)
    {
        cpu_guard.status.cpu_rate = SYS_ADPT_CPU_GUARD_THRESHOLD_MAX;
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
        DEV_SWDRV_PMGR_SetCpuPortRate(cpu_guard.status.cpu_rate);
    }
    else
    {
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    }

    return TRUE;
    }

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard trap status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardTrapStatus(BOOL_T status)
{
    if (cpu_guard.config.trap_status == status)
    {
        return TRUE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    cpu_guard.config.trap_status = status;

    /* When trap status is switched from False to TRUE,
     * reset Trap Event state
     */
    if (status == TRUE)
        cpu_guard.status.trap_event = 0;

    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get cpu guard status.
 * INPUT    : None.
 * OUTPUT   : enable - TRUE/FALSE.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardStatus(BOOL_T *status)
{
    if (status == NULL)
    {
        return FALSE;
    }

    *status = cpu_guard.config.status;

    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuGuardInfo
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get cpu guard information.
 * INPUT    : None.
 * OUTPUT   : info - cpu guard information.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuGuardInfo(SYS_MGR_CpuGuardInfo_T *info)
{
    if (info == NULL)
    {
        return FALSE;
    }

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);
    info->watermark_high = cpu_guard.config.watermark_high;
    info->watermark_low  = cpu_guard.config.watermark_low;
    info->threshold_max  = cpu_guard.config.threshold_max;
    info->threshold_min  = cpu_guard.config.threshold_min;
    info->status         = cpu_guard.config.status;
    info->cpu_rate       = cpu_guard.status.cpu_rate;
    info->trap_status    = cpu_guard.config.trap_status;
    SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);

    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get high watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardHighWatermark(UI32_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.watermark_high;

    if (*value == SYS_DFLT_CPU_UTILIZATION_WATERMARK_HIGH)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get low watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardLowWatermark(UI32_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.watermark_low;

    if (*value == SYS_DFLT_CPU_UTILIZATION_WATERMARK_LOW)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get max threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardMaxThreshold(UI32_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.threshold_max;

    if (*value == SYS_DFLT_CPU_GUARD_THRESHOLD_MAX)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get min threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardMinThreshold(UI32_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.threshold_min;

    if (*value == SYS_DFLT_CPU_GUARD_THRESHOLD_MIN)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardStatus(BOOL_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.status;

    if (*value == SYS_DFLT_CPU_GUARD_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_MGR_GetRunningCpuGuardTrapStatus(BOOL_T *value)
{
    if (value == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *value = cpu_guard.config.trap_status;

    if (*value == SYS_DFLT_CPU_GUARD_TRAP_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_CalculateNewCpuRate
 * -----------------------------------------------------------------------------
 * PURPOSE  : Calculate new cpu rate
 * INPUT    : None
 * OUTPUT   : new_rate
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
static void SYS_MGR_CalculateNewCpuRate(UI32_T *new_rate)
{
    UI32_T old_rate;
    UI32_T cpu_util;
    UI32_T percent;
    UI32_T level;

    if (new_rate == NULL)
    {
        return;
    }

    cpu_util = sys_mgr_cpu_util_record[current_index].cpu_usage;

    *new_rate = old_rate = cpu_guard.status.cpu_rate;

    if (cpu_util > cpu_guard.config.watermark_high)
    {
        level = cpu_util - cpu_guard.config.watermark_high;

        percent = 98 - level / 5;

        *new_rate = old_rate * percent / 100;

        if (*new_rate < cpu_guard.config.threshold_min)
        {
            *new_rate = cpu_guard.config.threshold_min;
        }
    }
    else if (cpu_util < cpu_guard.config.watermark_low)
    {
        level = cpu_guard.config.watermark_low - cpu_util;

        percent = 101 + level / 10;

        *new_rate = old_rate * percent / 100;

        if (*new_rate > cpu_guard.config.threshold_max)
        {
            *new_rate = cpu_guard.config.threshold_max;
        }
    }

    if (cpu_guard.debug_flag == TRUE && old_rate != *new_rate)
    {
        printf("cpu_util=%lu, new_rate=%lu\n", (unsigned long)cpu_util, (unsigned long)*new_rate);
    }

    return;
}

static void SYS_MGR_CpuGuardSendTrap(UI32_T old_rate, UI32_T new_rate)
{
    TRAP_EVENT_TrapData_T data;
    UI32_T cpu_util = sys_mgr_cpu_util_record[current_index].cpu_usage;;

    memset(&data, 0, sizeof(TRAP_EVENT_TrapData_T));

    if (TRUE == cpu_guard.config.trap_status)
    {
        if (cpu_util > cpu_guard.config.watermark_high)
        {
            data.trap_type = TRAP_EVENT_CPU_GUARD_CONTROL;
        }
        else if (cpu_util < cpu_guard.config.watermark_low)
        {
            data.trap_type = TRAP_EVENT_CPU_GUARD_RELEASE;
        }

        data.community_specified = FALSE;

        /* The control trap is sent when CPU utilization rises above the
         * high-watermark first time or when CPU utilization rises from
         * below the low-watermark to above the high-watermark.
         * The release trap is sent when CPU utilization falls from above
         * the high-watermark to below the low-watermark.
         */
        if (data.trap_type != cpu_guard.status.trap_event)
        {
            if (cpu_guard.status.trap_event == 0 &&
                data.trap_type == TRAP_EVENT_CPU_GUARD_RELEASE)
                return;

            SNMP_PMGR_ReqSendTrapOptional(&data,
                TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
            cpu_guard.status.trap_event = data.trap_type;
        }
    }
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_CpuRateLimitController
 * -----------------------------------------------------------------------------
 * PURPOSE  : Restrict CPU ingress packet rate according to CPU
 *            utilization automatically.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
static void SYS_MGR_CpuRateLimitController(void)
{
    UI32_T new_rate = 0;

    SYS_MGR_ENTER_CRITICAL_SECTION(sys_mgr_sem_id);

    if (cpu_guard.config.status == FALSE)
    {
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
        return;
    }

    SYS_MGR_CalculateNewCpuRate(&new_rate);

    if (new_rate != cpu_guard.status.cpu_rate)
    {
        SYS_MGR_CpuGuardSendTrap(cpu_guard.status.cpu_rate, new_rate);
        cpu_guard.status.cpu_rate = new_rate;
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
        DEV_SWDRV_PMGR_SetCpuPortRate(new_rate);
    }
    else
    {
        SYS_MGR_LEAVE_CRITICAL_SECTION(sys_mgr_sem_id);
    }

    return;
}
#endif

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_CpuUtilizationMonitoringProcess
 * -----------------------------------------------------------------------------
 * PURPOSE  : Update CPU utilization info.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
void SYS_MGR_CpuUtilizationMonitoringProcess(void)
{
    SYS_MGR_ComputeCpuUtilization();
#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    SYS_MGR_CpuRateLimitController();
#endif

    if (current_index == SYS_MGR_MAX_NBR_OF_CPU_UTIL_ENTRY-1)
    {
        SYS_MGR_ComputeCpu5secUtilizaition();
        SYS_MGR_CheckCpuUtilAndSendTrap();
    }
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUsagePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : The current CPU utilization in percent in the past 5 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_p       -- cpu usage (X%)
 *            cpu_util_float_p -- fractional part of cpu usage (X * 0.001%)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUsagePercentage(UI32_T *cpu_util_p, UI32_T *cpu_util_float_p)
{
    if (cpu_util_p == NULL || cpu_util_float_p == NULL)
        return FALSE;

    if (sys_mgr_cpu_util_5sec_record_valid_count == 0)
        return FALSE;

    *cpu_util_p = sys_mgr_cpu_util_5sec_record[current_5sec_index].cpu_usage;
    *cpu_util_float_p = sys_mgr_cpu_util_5sec_record[current_5sec_index].cpu_usage_float;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUsageMaximum
 * ---------------------------------------------------------------------
 * PURPOSE  : The maximum CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_max_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUsageMaximum(UI32_T *cpu_util_max_p)
{
    int i, max_i;

    if (cpu_util_max_p == NULL)
        return FALSE;

    if (sys_mgr_cpu_util_5sec_record_valid_count == 0)
        return FALSE;

    for (max_i = 0, i = 1; i < sys_mgr_cpu_util_5sec_record_valid_count; i++)
        if (sys_mgr_cpu_util_5sec_record[max_i].cpu_usage < sys_mgr_cpu_util_5sec_record[i].cpu_usage)
            max_i = i;

    *cpu_util_max_p = sys_mgr_cpu_util_5sec_record[max_i].cpu_usage;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUsageAverage
 * ---------------------------------------------------------------------
 * PURPOSE  : The average CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_avg_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUsageAverage(UI32_T *cpu_util_avg_p)
{
    int i, sum;

    if (cpu_util_avg_p == NULL)
        return FALSE;

    if (sys_mgr_cpu_util_5sec_record_valid_count == 0)
        return FALSE;

    for (sum = 0, i = 0; i < sys_mgr_cpu_util_5sec_record_valid_count; i++)
        sum += sys_mgr_cpu_util_5sec_record[i].cpu_usage;

    *cpu_util_avg_p = sum / sys_mgr_cpu_util_5sec_record_valid_count;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUsagePeakTime
 * ---------------------------------------------------------------------
 * PURPOSE  : The start time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_time -- peak start time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUsagePeakTime(UI32_T *cpu_util_peak_time)
{
    if (cpu_util_peak_time == NULL)
        return FALSE;

    *cpu_util_peak_time = sys_mgr_cpu_alarm.alarm_start_time;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUsagePeakDuration
 * ---------------------------------------------------------------------
 * PURPOSE  : The duration time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_duration -- peak duration time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUsagePeakDuration(UI32_T *cpu_util_peak_duration)
{
    UI32_T current_time;

    if (cpu_util_peak_duration == NULL)
        return FALSE;

    if (sys_mgr_cpu_alarm.alarm_status)
    {
        SYS_TIME_GetRealTimeBySec(&current_time);
        *cpu_util_peak_duration = (SYS_TIME_GetSystemTicksBy10ms() - sys_mgr_cpu_alarm.alarm_start_tick) / SYS_BLD_TICKS_PER_SECOND;
    }
    else
    {
        *cpu_util_peak_duration = (sys_mgr_cpu_alarm.alarm_end_tick - sys_mgr_cpu_alarm.alarm_start_tick) / SYS_BLD_TICKS_PER_SECOND;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of CPU utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUtilAlarmStatus(BOOL_T *alarm_status_p)
{
    if (alarm_status_p == NULL)
        return FALSE;

    *alarm_status_p = sys_mgr_cpu_alarm.alarm_status;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of CPU utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuUtilRisingThreshold(UI32_T rising_threshold)
{
    if (rising_threshold > 100 || (provision_complete && rising_threshold <= sys_mgr_cpu_alarm.falling_threshold))
        return FALSE;

    sys_mgr_cpu_alarm.rising_threshold = rising_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    if (rising_threshold_p == NULL)
        return FALSE;

    *rising_threshold_p = sys_mgr_cpu_alarm.rising_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningCpuUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    if (rising_threshold_p == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *rising_threshold_p = sys_mgr_cpu_alarm.rising_threshold;

    if (*rising_threshold_p == SYS_DFLT_SYSMGR_CPU_UTILIZATION_RAISING_THRESHOLD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set falling threshold of CPU utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuUtilFallingThreshold(UI32_T falling_threshold)
{
    if (provision_complete == TRUE && falling_threshold >= sys_mgr_cpu_alarm.rising_threshold)
        return FALSE;

    sys_mgr_cpu_alarm.falling_threshold = falling_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    if (falling_threshold_p == NULL)
        return FALSE;

    *falling_threshold_p = sys_mgr_cpu_alarm.falling_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningCpuUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    if (falling_threshold_p == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *falling_threshold_p = sys_mgr_cpu_alarm.falling_threshold;

    if (*falling_threshold_p == SYS_DFLT_SYSMGR_CPU_UTILIZATION_FALLING_THRESHOLD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetCpuUtilByName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get task CPU util info
 * INPUT    : cpu_util_info_p->task_name
 *            get_next
 * OUTPUT   : cpu_util_info_p
 * RETURN   : TRUE/FALSE
 * NOTES    : To get the first entry by task_name[0] = 0 and get_next = TRUE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetCpuUtilByName(
    SYS_MGR_TaskCpuUtilInfo_T *cpu_util_info_p,
    BOOL_T get_next)
{
    SYS_MGR_TaskCpuUtilEntry_T key, *entry_p;

    if (cpu_util_info_p == NULL)
        return FALSE;

    strncpy(key.task_name, cpu_util_info_p->task_name, sizeof(key.task_name)-1);
    key.task_name[sizeof(key.task_name)-1] = 0;
    entry_p = &key;

    while (1)
    {
        if (NULL == (entry_p = SYS_MGR_GetTaskCpuUtilEntryPtrByName(entry_p->task_name, get_next)))
            return FALSE;

        if (entry_p->cpu_util_record_valid_count > 0)
            break;

        if (!get_next)
            return FALSE;
    }

    SYS_MGR_FillTaskCpuUtilInfo(entry_p, cpu_util_info_p);

    return TRUE;
}

/* LOCAL SUBPROGRAM SPECIFICATIONS for task CPU util
 */

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_CompareTaskCpuUtilEntryById
 *-----------------------------------------------------------------
 * FUNCTION: A compare func to compare two task cpu util entries
 * INPUT   : e1, e2
 * OUTPUT  : None.
 * RETURN  : int
 * NOTE    : None.
 *----------------------------------------------------------------*/
static int SYS_MGR_CompareTaskCpuUtilEntryById(void *e1, void *e2)
{
    SYS_MGR_TaskCpuUtilEntry_T *p1 = *(SYS_MGR_TaskCpuUtilEntry_T **)e1;
    SYS_MGR_TaskCpuUtilEntry_T *p2 = *(SYS_MGR_TaskCpuUtilEntry_T **)e2;
    int diff;

    if (p1 == p2)
        return 0;

    if ((diff = !!p1 - !!p2))
        return (diff > 0 ? 1 : -1);

    if ((diff = p1->task_id - p2->task_id))
        return (diff > 0 ? 1 : -1);

    return 0;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_CompareTaskCpuUtilEntryByName
 *-----------------------------------------------------------------
 * FUNCTION: A compare func to compare two task cpu util entries
 * INPUT   : e1, e2
 * OUTPUT  : None.
 * RETURN  : int
 * NOTE    : None.
 *----------------------------------------------------------------*/
static int SYS_MGR_CompareTaskCpuUtilEntryByName(void *e1, void *e2)
{
    SYS_MGR_TaskCpuUtilEntry_T *p1 = *(SYS_MGR_TaskCpuUtilEntry_T **)e1;
    SYS_MGR_TaskCpuUtilEntry_T *p2 = *(SYS_MGR_TaskCpuUtilEntry_T **)e2;
    int diff;

    if (p1 == p2)
        return 0;

    if ((diff = !!p1 - !!p2))
        return (diff > 0 ? 1 : -1);

    if ((diff = strcasecmp(p1->task_name, p2->task_name)))
        return (diff > 0 ? 1 : -1);

    return 0;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_CreateTaskCpuUtilEntry
 *-----------------------------------------------------------------
 * FUNCTION: To create a task cpu util entry
 * INPUT   : task_id
 * OUTPUT  : None
 * RETURN  : pointer to created entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_CreateTaskCpuUtilEntry(UI32_T task_id)
{
    SYS_MGR_TaskCpuUtilEntry_T *entry_p;

    if (NULL == (entry_p = L_PT_Allocate(&sys_mgr_task_cpu_util_entry_buf_desc)))
    {
        return NULL;
    }

    memset(entry_p, 0, sizeof(*entry_p));
    entry_p->task_id = task_id;

    if (entry_p->task_id == SYS_MGR_UNNAMED_TASK_CPU_UTIL_ENTRY__TASK_ID)
    {
        strncpy(entry_p->task_name, SYS_MGR_UNNAMED_TASK_CPU_UTIL_ENTRY__TASK_NAME, sizeof(entry_p->task_name)-1);
        entry_p->task_name[sizeof(entry_p->task_name)-1] = 0;
    }
    else
    {
        if (SYSFUN_TaskIDToName(entry_p->task_id, entry_p->task_name, sizeof(entry_p->task_name)) != SYSFUN_OK)
        {
            entry_p->task_name[0] = 0;
        }
    }

    if (!L_SORT_LST_Set(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p))
    {
        SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_WARN, "Can't add to id_lst: task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
        L_PT_Free(&sys_mgr_task_cpu_util_entry_buf_desc, entry_p);
        return NULL;
    }

    if (entry_p->task_name[0] != 0)
    {
        if (!L_SORT_LST_Set(&sys_mgr_task_cpu_util_entry_name_lst, &entry_p))
        {
            SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_WARN, "Can't add to name_lst: task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
            L_SORT_LST_Delete(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p);
            L_PT_Free(&sys_mgr_task_cpu_util_entry_buf_desc, entry_p);
            return NULL;
        }
    }

    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, "task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
    return entry_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_DestroyCpuUtilEntry
 *-----------------------------------------------------------------
 * FUNCTION: To destroy a task cpu util entry
 * INPUT   : entry_p
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static void SYS_MGR_DestroyCpuUtilEntry(SYS_MGR_TaskCpuUtilEntry_T *entry_p)
{
    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, "task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
    if (entry_p->task_name[0] != 0)
        L_SORT_LST_Delete(&sys_mgr_task_cpu_util_entry_name_lst, &entry_p);
    L_SORT_LST_Delete(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p);
    L_PT_Free(&sys_mgr_task_cpu_util_entry_buf_desc, entry_p);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetTaskCpuUtilEntryPtrById
 *-----------------------------------------------------------------
 * FUNCTION: To find a task cpu util entry
 * INPUT   : task_id
 *           get_next
 * OUTPUT  : None.
 * RETURN  : pointer to found entry or NULL if failed
 * NOTE    : To get the first entry by task_id = 0xffffffff and get_next = TRUE
 *----------------------------------------------------------------*/
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_GetTaskCpuUtilEntryPtrById(UI32_T task_id, BOOL_T get_next)
{
    SYS_MGR_TaskCpuUtilEntry_T key, *entry_p;
    BOOL_T found;

    key.task_id = task_id;
    entry_p = &key;

    if (get_next)
    {
        if (task_id == 0xffffffff)
        {
            found = L_SORT_LST_Get_1st(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p);
        }
        else
        {
            found = L_SORT_LST_Get_Next(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p);
        }
    }
    else
    {
        found = L_SORT_LST_Get(&sys_mgr_task_cpu_util_entry_id_lst, &entry_p);
    }

    if (!found)
    {
        SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_TRACE, "Entry not found: task_id:%lu get_next:%d\n", (unsigned long)task_id, (int)get_next);
        return NULL;
    }

    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_TRACE, "task_id:%lu get_next:%d\n", (unsigned long)task_id, (int)get_next);
    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_TRACE, " found: task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
    return entry_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetTaskCpuUtilEntryPtrByName
 *-----------------------------------------------------------------
 * FUNCTION: To find a task cpu util entry
 * INPUT   : task_name
 *           get_next
 * OUTPUT  : None.
 * RETURN  : pointer to found entry or NULL if failed
 * NOTE    : To get the first entry by task_name[0] = 0 and get_next = TRUE
 *----------------------------------------------------------------*/
static SYS_MGR_TaskCpuUtilEntry_T *SYS_MGR_GetTaskCpuUtilEntryPtrByName(char *task_name, BOOL_T get_next)
{
    SYS_MGR_TaskCpuUtilEntry_T key, *entry_p;
    BOOL_T found;

    strncpy(key.task_name, task_name, sizeof(key.task_name)-1);
    key.task_name[sizeof(key.task_name)-1] = 0;
    entry_p = &key;

    if (get_next)
    {
        if (task_name[0] == 0)
        {
            found = L_SORT_LST_Get_1st(&sys_mgr_task_cpu_util_entry_name_lst, &entry_p);
        }
        else
        {
            found = L_SORT_LST_Get_Next(&sys_mgr_task_cpu_util_entry_name_lst, &entry_p);
        }
    }
    else
    {
        found = L_SORT_LST_Get(&sys_mgr_task_cpu_util_entry_name_lst, &entry_p);
    }

    if (!found)
    {
        SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, "Entry not found: task_name:%s get_next:%d\n", task_name, (int)get_next);
        return NULL;
    }

    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, "task_name:%s get_next:%d\n", task_name, (int)get_next);
    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, " found: task_id:%lu task_name:%s\n", (unsigned long)entry_p->task_id, entry_p->task_name);
    return entry_p;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_UpdateTaskCpuUtil
 * ---------------------------------------------------------------------
 * PURPOSE  : Update task CPU utilization info.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
static void SYS_MGR_UpdateTaskCpuUtil(UI32_T delta_idle_ticks)
{
    SYS_MGR_CpuUtilizationRecord_T tmp_cpu_util_record;
    SYS_MGR_TaskCpuUtilEntry_T key, *entry_p;
    UI32_T task_id_ar[SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM];
    UI32_T task_id_num;
    UI32_T task_id;
    UI32_T task_user_ticks, task_sys_ticks;
    UI32_T delta_busy_ticks, delta_during_ticks;
    UI32_T sys_ticks;
    UI32_T precise_usage;
    UI32_T i;

    /* retrieve task list
     */
    task_id_num = sizeof(task_id_ar)/sizeof(*task_id_ar);

    if (SYSFUN_GetTaskIdList(task_id_ar, &task_id_num) != SYSFUN_OK)
    {
        return;
    }

    /* collect task busy time
     */
    sys_ticks = SYSFUN_GetSysTick();
    delta_during_ticks = delta_idle_ticks;

    for (i = 0; i < task_id_num; i++)
    {
        task_id = task_id_ar[i];

        /* get current CPU usage
         */
        if (SYSFUN_GetCpuUsageByTaskId(task_id, &task_user_ticks, &task_sys_ticks) != SYSFUN_OK)
        {
            continue;
        }

        /* get current CPU usage
         */
        if (NULL == (entry_p = SYS_MGR_GetTaskCpuUtilEntryPtrById(task_id, FALSE)))
        {
            if (NULL == (entry_p = SYS_MGR_CreateTaskCpuUtilEntry(task_id)))
            {
                continue;
            }

            entry_p->task_user_ticks = task_user_ticks;
            entry_p->task_sys_ticks = task_sys_ticks;
            entry_p->create_timestamp = sys_ticks;
            continue;
        }

        entry_p->delta_task_user_ticks = task_user_ticks - entry_p->task_user_ticks;
        entry_p->delta_task_sys_ticks = task_sys_ticks - entry_p->task_sys_ticks;

        delta_busy_ticks = entry_p->delta_task_user_ticks + entry_p->delta_task_sys_ticks;
        delta_during_ticks += delta_busy_ticks;

        entry_p->task_user_ticks = task_user_ticks;
        entry_p->task_sys_ticks = task_sys_ticks;

        entry_p->delta_update_timestamp = sys_ticks;

        /* for unnamed task.
         */
        if (entry_p->task_name[0] == 0)
        {
            SYS_MGR_TaskCpuUtilEntry_T *unnamed_entry_p;

            task_id = SYS_MGR_UNNAMED_TASK_CPU_UTIL_ENTRY__TASK_ID;

            if (NULL == (unnamed_entry_p = SYS_MGR_GetTaskCpuUtilEntryPtrById(task_id, FALSE)))
            {
                if (NULL == (unnamed_entry_p = SYS_MGR_CreateTaskCpuUtilEntry(task_id)))
                {
                    continue;
                }
            }

            entry_p->aggregate_task_id = task_id;

            if (unnamed_entry_p->delta_update_timestamp != sys_ticks)
            {
                unnamed_entry_p->delta_task_user_ticks = 0;
                unnamed_entry_p->delta_task_sys_ticks = 0;

                unnamed_entry_p->task_user_ticks = 0;
                unnamed_entry_p->task_sys_ticks = 0;
            }

            unnamed_entry_p->delta_task_user_ticks += entry_p->delta_task_user_ticks;
            unnamed_entry_p->delta_task_sys_ticks += entry_p->delta_task_sys_ticks;

            unnamed_entry_p->task_user_ticks += entry_p->task_user_ticks;
            unnamed_entry_p->task_sys_ticks += entry_p->task_sys_ticks;

            unnamed_entry_p->delta_update_timestamp = sys_ticks;
        }
    } /* end of for (i) */

    /* calculate task cpu usage
     */
    if (delta_during_ticks == 0)
    {
        return;
    }

    key.task_id = 0xffffffff;
    entry_p = &key;

    while (NULL != (entry_p = SYS_MGR_GetTaskCpuUtilEntryPtrById(entry_p->task_id, TRUE)))
    {
        /* new entry, ignore util calculation
         */
        if (entry_p->create_timestamp == sys_ticks)
        {
            continue;
        }

        /* expired entry, remove it
         */
        if (entry_p->delta_update_timestamp != sys_ticks)
        {
            SYS_MGR_DestroyCpuUtilEntry(entry_p);
            continue;
        }

        delta_busy_ticks = entry_p->delta_task_user_ticks + entry_p->delta_task_sys_ticks;
        precise_usage = 100000 * delta_busy_ticks / delta_during_ticks;

        tmp_cpu_util_record.cpu_usage = precise_usage / 1000;
        tmp_cpu_util_record.cpu_usage_float = precise_usage % 1000;
        tmp_cpu_util_record.record_tick = sys_ticks;

        /* update CPU usage record
         */
        if (entry_p->cpu_util_record_valid_count == 0)
            entry_p->current_5sec_index = 0;
        else
            entry_p->current_5sec_index = (entry_p->current_5sec_index + 1) % SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY;

        entry_p->cpu_util_record[entry_p->current_5sec_index] = tmp_cpu_util_record;

        if (entry_p->cpu_util_record_valid_count < SYS_MGR_MAX_NBR_OF_5_SECONDS_ENTRY)
            entry_p->cpu_util_record_valid_count++;
    } /* end of while (entry_p) */
} /* end of SYS_MGR_UpdateTaskCpuUtil */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_FillTaskCpuUtilInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : Fill task cpu util info by a task cpu util entry
 * INPUT    : cpu_util_info_p->task_name
 *            get_next
 * OUTPUT   : task_name
 *            cpu_util_p       -- cpu usage (X%)
 *            cpu_util_float_p -- fractional part of cpu usage (X * 0.001%)
 * RETURN   : TRUE/FALSE
 * NOTES    : To get the first entry by task_name[0] = 0 and get_next = TRUE
 * ---------------------------------------------------------------------
 */
static BOOL_T SYS_MGR_FillTaskCpuUtilInfo(
    SYS_MGR_TaskCpuUtilEntry_T *entry_p,
    SYS_MGR_TaskCpuUtilInfo_T *cpu_util_info_p)
{
    UI32_T sum, avg;
    int i, max_i;

    strncpy(cpu_util_info_p->task_name, entry_p->task_name, sizeof(cpu_util_info_p->task_name)-1);
    cpu_util_info_p->task_name[sizeof(cpu_util_info_p->task_name)-1] = 0;

    cpu_util_info_p->cpu_util = entry_p->cpu_util_record[entry_p->current_5sec_index].cpu_usage;
    cpu_util_info_p->cpu_util_float = entry_p->cpu_util_record[entry_p->current_5sec_index].cpu_usage_float;

    for (max_i = 0, sum = 0, i = 1; i < entry_p->cpu_util_record_valid_count; i++)
    {
        if (entry_p->cpu_util_record[max_i].cpu_usage < entry_p->cpu_util_record[i].cpu_usage ||
            (entry_p->cpu_util_record[max_i].cpu_usage == entry_p->cpu_util_record[i].cpu_usage &&
             entry_p->cpu_util_record[max_i].cpu_usage_float < entry_p->cpu_util_record[i].cpu_usage_float))
        {
            max_i = i;
        }

        sum +=
            entry_p->cpu_util_record[i].cpu_usage * 1000 +
            entry_p->cpu_util_record[i].cpu_usage_float;
    }

    cpu_util_info_p->cpu_util_max = entry_p->cpu_util_record[max_i].cpu_usage;
    cpu_util_info_p->cpu_util_max_float = entry_p->cpu_util_record[max_i].cpu_usage_float;

    avg = sum / entry_p->cpu_util_record_valid_count;
    cpu_util_info_p->cpu_util_avg = avg / 1000;
    cpu_util_info_p->cpu_util_avg_float = avg % 1000;

    SYS_MGR_DBG_Printf(SYS_MGR_DBG_F_CPU_UTIL, SYS_MGR_DBG_L_VERB, "task_name:%s cpu_util/avg/max:%lu.%03lu/%lu.%03lu/%lu.%03lu\n",
        cpu_util_info_p->task_name,
        (unsigned long)cpu_util_info_p->cpu_util, (unsigned long)cpu_util_info_p->cpu_util_float,
        (unsigned long)cpu_util_info_p->cpu_util_avg, (unsigned long)cpu_util_info_p->cpu_util_avg_float,
        (unsigned long)cpu_util_info_p->cpu_util_max, (unsigned long)cpu_util_info_p->cpu_util_max_float);

    return TRUE;
}

#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE) */
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_MemoryUtilizationMonitoringProcess
 * ---------------------------------------------------------------------
 * PURPOSE  : Update memory utilization info.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_MemoryUtilizationMonitoringProcess(void)
{
    SYS_MGR_MemoryUtilizationRecord_T tmp_mem_util_record;
    UI32_T mem_util;
    MEM_SIZE_T total_bytes, free_bytes;
    MEM_SIZE_T free_kbytes, total_kbytes;
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
    TRAP_EVENT_TrapData_T data;
#endif

    /* get current memory info
     */
    if (SYSFUN_GetMemoryUsage(&total_bytes, &free_bytes) != SYSFUN_OK)
    {
        return;
    }

    tmp_mem_util_record.free_bytes = free_bytes;
    tmp_mem_util_record.used_bytes = SYS_HWCFG_DRAM_SIZE - free_bytes;

    /* check threshold
     */
    free_kbytes = tmp_mem_util_record.free_bytes >> 10;
    total_kbytes = SYS_HWCFG_DRAM_SIZE >> 10;

    mem_util = 100 * free_kbytes / total_kbytes;

    if (sys_mgr_mem_alarm.alarm_status)
    {
        if (mem_util <= sys_mgr_mem_alarm.falling_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_MEM_FALLING_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_mem_alarm.alarm_end_time);
            sys_mgr_mem_alarm.alarm_status = FALSE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n", __FUNCTION__, __LINE__, (unsigned long)sys_mgr_mem_alarm.alarm_end_time, (unsigned long)mem_util, (unsigned long)sys_mgr_mem_alarm.falling_threshold);
            }
        }
    }
    else
    {
        if (mem_util >= sys_mgr_mem_alarm.rising_threshold)
        {
#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
            data.community_specified = FALSE;
            data.trap_type = TRAP_EVENT_MEM_RAISE_TRAP;

            SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif /* if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

            SYS_TIME_GetRealTimeBySec(&sys_mgr_mem_alarm.alarm_start_time);
            sys_mgr_mem_alarm.alarm_status = TRUE;

            if (sys_mgr_debug_alarm)
            {
                printf("%s:%d time:%lu current:%lu threshold:%lu\n", __FUNCTION__, __LINE__, (unsigned long)sys_mgr_mem_alarm.alarm_start_time, (unsigned long)mem_util, (unsigned long)sys_mgr_mem_alarm.rising_threshold);
            }
        }
    }

    /* update memrory utilization
     */
    sys_mgr_mem_util_record = tmp_mem_util_record;
}

/*------------------------------------------------------------------------------
 * Function : SYS_MGR_GetMemoryUtilizationBrief
 *------------------------------------------------------------------------------
 * Purpose  : This function will get memory utilization
 * INPUT    : none
 * OUTPUT   : sys_mem_brief_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 *-----------------------------------------------------------------------------*/
BOOL_T  SYS_MGR_GetMemoryUtilizationBrief(SYS_MGR_MemoryUtilBrief_T *sys_mem_brief_p)
{
    sys_mem_brief_p->free_bytes = sys_mgr_mem_util_record.free_bytes;
    sys_mem_brief_p->free_blocks = sys_mgr_mem_util_record.free_blocks;
    sys_mem_brief_p->free_max_block_size = sys_mgr_mem_util_record.free_max_block_size;
    sys_mem_brief_p->used_bytes = sys_mgr_mem_util_record.used_bytes;
    sys_mem_brief_p->used_blocks = sys_mgr_mem_util_record.used_blocks;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetMemoryUtilTotalMemory
 * ---------------------------------------------------------------------
 * PURPOSE  : Get total memory capability
 * INPUT    : None.
 * OUTPUT   : total_memory_p -- total size of usable memory (bytes)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMemoryUtilTotalMemory(MEM_SIZE_T *total_memory_p)
{
    *total_memory_p = SYS_HWCFG_DRAM_SIZE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetMemoryUtilFreePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : Get free memory percentage
 * INPUT    : None.
 * OUTPUT   : free_percentage_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMemoryUtilFreePercentage(UI32_T *free_percentage_p)
{
    MEM_SIZE_T free_kbytes, total_kbytes;

    free_kbytes = sys_mgr_mem_util_record.free_bytes >> 10;
    total_kbytes = free_kbytes + (sys_mgr_mem_util_record.used_bytes >> 10);

    *free_percentage_p = 100 * free_kbytes / total_kbytes;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetMemoryUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of memory utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMemoryUtilAlarmStatus(BOOL_T *alarm_status_p)
{
    if (alarm_status_p == NULL)
        return FALSE;

    *alarm_status_p = sys_mgr_mem_alarm.alarm_status;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetMemoryUtilRisingThreshold(UI32_T rising_threshold)
{
    if (rising_threshold > 100 || (provision_complete && rising_threshold <= sys_mgr_mem_alarm.falling_threshold))
        return FALSE;

    sys_mgr_mem_alarm.rising_threshold = rising_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMemoryUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    if (rising_threshold_p == NULL)
        return FALSE;

    *rising_threshold_p = sys_mgr_mem_alarm.rising_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningMemoryUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    if (rising_threshold_p == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *rising_threshold_p = sys_mgr_mem_alarm.rising_threshold;

    if (*rising_threshold_p == SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_RAISING_THRESHOLD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetMemoryUtilFallingThreshold(UI32_T falling_threshold)
{
    if (provision_complete && falling_threshold >= sys_mgr_mem_alarm.rising_threshold)
        return FALSE;

    sys_mgr_mem_alarm.falling_threshold = falling_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetMemoryUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    if (falling_threshold_p == NULL)
        return FALSE;

    *falling_threshold_p = sys_mgr_mem_alarm.falling_threshold;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningMemoryUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    if (falling_threshold_p == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *falling_threshold_p = sys_mgr_mem_alarm.falling_threshold;

    if (*falling_threshold_p == SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_FALLING_THRESHOLD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE) */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGR_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYS_MGR mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    if (ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_IPC_RESULT_FAIL;
        ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch (SYS_MGR_MSG_CMD(ipcmsg_p))
    {
        case SYS_MGR_IPC_CMD_GETCONSOLECFG:
        {
            SYS_MGR_IPCMsg_ConsoleCfg_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetConsoleCfg(&data_p->consolecfg);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_ConsoleCfg_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETRUNNINGPASSWORDTHRESHOLD:
        {
            SYS_MGR_IPCMsg_PasswordThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningPasswordThreshold(&data_p->passwordthreshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T);
            break;
        }
         case SYS_MGR_IPC_CMD_SETPASSWORDTHRESHOLD:
        {
            SYS_MGR_IPCMsg_PasswordThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetPasswordThreshold (data_p->passwordthreshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
         case SYS_MGR_IPC_CMD_GETRUNNINGCONSOLEEXECTIMEOUT:
        {
            SYS_MGR_IPC_ExecTimeOut_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningConsoleExecTimeOut(&data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SETCONSOLEEXECTIMEOUT:
        {
            SYS_MGR_IPC_ExecTimeOut_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetConsoleExecTimeOut(data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_MGR_IPC_CMD_GETRUNNINGCONSOLESILENTTIME:
        {
            SYS_MGR_IPC_ConsoleSilentTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningConsoleSilentTime(&data_p->silent_time);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SETCONSOLESILENTTIME:
        {
            SYS_MGR_IPC_ConsoleSilentTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetConsoleSilentTime(data_p->silent_time);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_MGR_IPC_CMD_GET_RUNNING_TELNET_SILENT_TIME:
        {
            SYS_MGR_IPC_ConsoleSilentTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningTelnetSilentTime(&data_p->silent_time);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SET_TELNET_SILENT_TIME:
        {
            SYS_MGR_IPC_ConsoleSilentTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetTelnetSilentTime(data_p->silent_time);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_MGR_IPC_CMD_GETTELNETCFG:
        {
            SYS_MGR_IPC_TelnetCfg_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetTelnetCfg(&data_p->telnetcfg);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TelnetCfg_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETRUNNINGTELNETPASSWORDTHRESHOLD:
        {
            SYS_MGR_IPCMsg_PasswordThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningTelnetPasswordThreshold(&data_p->passwordthreshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SETTELNETPASSWORDTHRESHOLD:
        {
            SYS_MGR_IPCMsg_PasswordThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetTelnetPasswordThreshold(data_p->passwordthreshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_MGR_IPC_CMD_GETRUNNINGTELNETEXECTIMEOUT:
        {
            SYS_MGR_IPC_ExecTimeOut_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningTelnetExecTimeOut(&data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SETTELNETEXECTIMEOUT:
        {
            SYS_MGR_IPC_ExecTimeOut_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetTelnetExecTimeOut(data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_MGR_IPC_CMD_GETUARTPARAMETERS:
        {
            SYS_MGR_IPC_UartCfg_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetUartParameters(&data_p->uartcfg);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartCfg_T);
            break;
        }

#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
        case SYS_MGR_IPC_CMD_GETUARTOPERBAUDRATE:
        {
            SYS_MGR_IPC_UartBaudRate_T *data_p = (SYS_MGR_IPC_UartBaudRate_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetUartOperBaudrate(&(data_p->uart_operbaudrate));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartBaudRate_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETAUTOBAUDRATESWITCH:
        {
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_Get_Autobaudrate_Switch ();
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif

        case SYS_MGR_IPC_CMD_GETRUNNINGUARTPARAMETERS:
        {
            SYS_MGR_IPC_UartRunningCfg_T *data_p = (SYS_MGR_IPC_UartRunningCfg_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningUartParameters((SYS_MGR_Uart_RunningCfg_T *)data_p);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartRunningCfg_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETUARTBAUDRATE:
        {
            SYS_MGR_IPC_UartBaudRate_T *data_p = (SYS_MGR_IPC_UartBaudRate_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetUartBaudrate(data_p->uart_operbaudrate);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETUARTPARITY:
        {
            SYS_MGR_IPC_UartParity_T *data_p = (SYS_MGR_IPC_UartParity_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetUartParity(data_p->parity);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETUARTDATABITS:
        {
            SYS_MGR_IPC_UartDataBits_T *data_p = (SYS_MGR_IPC_UartDataBits_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetUartDataBits(data_p->data_length);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETUARTSTOPBITS:
        {
            SYS_MGR_IPC_UartStopBits_T *data_p = (SYS_MGR_IPC_UartStopBits_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetUartStopBits(data_p->stop_bits);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GETPROMPTSTRING:
        {
            SYS_MGR_IPC_PromptString_T *data_p = (SYS_MGR_IPC_PromptString_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetPromptString(data_p->prompt_string);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETPROMPTSTRING:
        {
            SYS_MGR_IPC_PromptString_T *data_p = (SYS_MGR_IPC_PromptString_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetPromptString(data_p->prompt_string);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGPROMPTSTRING:
        {
            SYS_MGR_IPC_PromptString_T *data_p = (SYS_MGR_IPC_PromptString_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningPromptString(data_p->prompt_string);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T);
            break;
        }

        case SYS_MGR_IPC_CMD_LOGWATCHDOGEXCEPTIONINFO:
        {
            SYS_MGR_IPC_LogWatchDogExceptionInfo_T *data_p = (SYS_MGR_IPC_LogWatchDogExceptionInfo_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_LogWatchDogExceptionInfo(&(data_p->wd_own));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GETPOWERSTATUS:
        {
            SYS_MGR_IPC_PowerStatus_T *data_p = (SYS_MGR_IPC_PowerStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetPowerStatus(&(data_p->power_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTPOWERSTATUS:
        {
            SYS_MGR_IPC_PowerStatus_T *data_p = (SYS_MGR_IPC_PowerStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextPowerStatus(&(data_p->power_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETSWITCHINDIVPOWER:
        {
            SYS_MGR_IPC_SwitchIndivPower_T *data_p = (SYS_MGR_IPC_SwitchIndivPower_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSwitchIndivPower(&(data_p->indiv_power));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTSWITCHINDIVPOWER:
        {
            SYS_MGR_IPC_SwitchIndivPower_T *data_p = (SYS_MGR_IPC_SwitchIndivPower_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextSwitchIndivPower(&(data_p->indiv_power));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T);
            break;
        }

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
        case SYS_MGR_IPC_CMD_GETSWALARMINPUTSTATUS:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSwAlarmInputStatus(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETNEXTSWALARMINPUT:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextSwAlarmInput(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETSWALARMINPUT:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSwAlarmInput(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETSWALARMINPUTNAME:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSwAlarmInputName(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SETSWALARMINPUTNAME:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwAlarmInputName(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GETRUNNINGALARMINPUTNAME:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningAlarmInputName(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
        case SYS_MGR_IPC_CMD_GETMAJORALARMOUTPUTSTATUS:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMajorAlarmOutputCurrentStatus(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMINORALARMOUTPUTSTATUS:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMinorAlarmOutputCurrentStatus(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETALARMOUTPUTSTATUS:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetAlarmOutputCurrentStatus(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTALARMOUTPUTSTATUS:
        {
            SYS_MGR_IPC_SwAlarm_T *data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextAlarmOutputCurrentStatus(&(data_p->sw_alarm));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T);
            break;
        }
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_MGR_IPC_CMD_GETFANSTATUS:
        {
            SYS_MGR_IPC_FanStatus_T *data_p = (SYS_MGR_IPC_FanStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetFanStatus(&(data_p->switch_fan_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTFANSTATUS:
        {
            SYS_MGR_IPC_FanStatus_T *data_p = (SYS_MGR_IPC_FanStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextFanStatus(&(data_p->switch_fan_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETFANSPEED:
        {
            SYS_MGR_IPC_FanStatus_T *data_p = (SYS_MGR_IPC_FanStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetFanSpeed(&(data_p->switch_fan_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTFANSPEED:
        {
            SYS_MGR_IPC_FanStatus_T *data_p = (SYS_MGR_IPC_FanStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextFanSpeed(&(data_p->switch_fan_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETFANSPEED:
        {
            SYS_MGR_IPC_FanStatus_T *data_p = (SYS_MGR_IPC_FanStatus_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetFanSpeed(&(data_p->switch_fan_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
        case SYS_MGR_IPC_CMD_SETFANSPEEDFORCEFULL:
        {
            SYS_MGR_IPC_FanSpeedForceFull_T *data_p = (SYS_MGR_IPC_FanSpeedForceFull_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetFanSpeedForceFull(data_p->mode);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GETFANSPEEDFORCEFULL:
        {
            SYS_MGR_IPC_FanSpeedForceFull_T *data_p = (SYS_MGR_IPC_FanSpeedForceFull_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetFanSpeedForceFull(&(data_p->mode));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGFANSPEEDFORCEFULL:
        {
            SYS_MGR_IPC_FanSpeedForceFull_T *data_p = (SYS_MGR_IPC_FanSpeedForceFull_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningFanSpeedForceFull(&(data_p->mode));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T);
            break;
        }
#endif /* #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */
#endif /* #if   (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_MGR_IPC_CMD_GETTHERMALSTATUS:
        {
            SYS_MGR_IPC_SwitchThermalEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetThermalStatus(&(data_p->switch_thermal_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTTHERMALSTATUS:
        {
            SYS_MGR_IPC_SwitchThermalEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextThermalStatus(&(data_p->switch_thermal_status));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETDEFAULTSWITCHTHERMALACTIONENTRY:
        {
            SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalActionEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetDefaultSwitchThermalActionEntry(&(data_p->action_entry));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETSWITCHTHERMALACTIONENTRY:
        {
            SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalActionEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSwitchThermalActionEntry(&(data_p->action_entry));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETNEXTSWITCHTHERMALACTIONENTRY:
        {
            SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalActionEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetNextSwitchThermalActionEntry(&(data_p->action_entry));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONENTRY:
        {
            SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p = (SYS_MGR_IPC_SwitchThermalActionEntry_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwitchThermalActionEntry(&(data_p->action_entry));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p = (SYS_MGR_IPC_SwitchThermalActionThreshold_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwitchThermalActionRisingThreshold(data_p->unit_index, data_p->thermal_index, data_p->index, data_p->threshold_action);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p = (SYS_MGR_IPC_SwitchThermalActionThreshold_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwitchThermalActionFallingThreshold(data_p->unit_index, data_p->thermal_index, data_p->index, data_p->threshold_action);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONACTION:
        {
            SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p = (SYS_MGR_IPC_SwitchThermalActionThreshold_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwitchThermalActionAction(data_p->unit_index, data_p->thermal_index, data_p->index, data_p->threshold_action);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONSTATUS:
        {
            SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p = (SYS_MGR_IPC_SwitchThermalActionThreshold_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetSwitchThermalActionStatus(data_p->unit_index, data_p->thermal_index, data_p->index, data_p->threshold_action);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_THERMALSTATUSCHANGED:
        {
            SYS_MGR_IPC_SwitchThermalEntry_T  *data_p = (SYS_MGR_IPC_SwitchThermalEntry_T *)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_ThermalStatusChanged_CallBack(data_p->switch_thermal_status.switch_unit_index, data_p->switch_thermal_status.switch_thermal_index, data_p->switch_thermal_status.switch_thermal_temp_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#endif /* #if   (SYS_CPNT_THERMAL_DETECT == TRUE) */

        case SYS_MGR_IPC_CMD_SETCONSOLELOGINTIMEOUT:
        {
            SYS_MGR_IPC_LoginTimeOut_T *data_p = (SYS_MGR_IPC_LoginTimeOut_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetConsoleLoginTimeOut(data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SETTELNETLOGINTIMEOUT:
        {
            SYS_MGR_IPC_LoginTimeOut_T *data_p = (SYS_MGR_IPC_LoginTimeOut_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetTelnetLoginTimeOut(data_p->time_out_value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCONSOLELOGINTIMEOUT:
        {
            SYS_MGR_IPC_LoginTimeOut_T *data_p = (SYS_MGR_IPC_LoginTimeOut_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningConsoleLoginTimeOut(&(data_p->time_out_value));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGTELNETLOGINTIMEOUT:
        {
            SYS_MGR_IPC_LoginTimeOut_T *data_p = (SYS_MGR_IPC_LoginTimeOut_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningTelnetLoginTimeOut(&(data_p->time_out_value));
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETSYSDESCR:
        {
            SYS_MGR_IPC_SysDescr_T *data_p = (SYS_MGR_IPC_SysDescr_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSysDescr(data_p->unit_id, data_p->sys_descrption);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysDescr_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETSYSOBJECTID:
        {
            SYS_MGR_IPC_SysObjectID_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetSysObjectID(data_p->unit_id, data_p->sys_oid);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysObjectID_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETPRODUCTNAME:
        {
            SYS_MGR_IPC_ProductName_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetProductName(data_p->unit_id, data_p->product_name);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductName_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETDEVICENAME:
        {
            SYS_MGR_IPC_DeviceName_T *data_p = (SYS_MGR_IPC_DeviceName_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetDeviceName(data_p->unit_id, data_p->device_name);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_DeviceName_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETPRIVATEMIBROOT:
        {
            SYS_MGR_IPC_MibRoot_T *data_p = (SYS_MGR_IPC_MibRoot_T*)SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetPrivateMibRoot(data_p->unit_id, data_p->mib_root);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MibRoot_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMODELNAME:
        {
            SYS_MGR_IPC_ModelName_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetModelName(data_p->unit_id, data_p->model_name);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ModelName_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_PRODUCT_MANUFACTURER:
        {
            SYS_MGR_IPC_ProductManufacturer_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetProductManufacturer(data_p->product_manufacturer);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductManufacturer_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_PRODUCT_DESCRIPTION:
        {
            SYS_MGR_IPC_ProductDescription_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetProductDescription(data_p->product_description);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductDescription_T);
            break;
        }

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
        case SYS_MGR_IPC_CMD_GETCPUUSAGEPERCENTAGE:
        {
            SYS_MGR_IPC_CpuUsagePercentage_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUsagePercentage(&data_p->cpu_util, &data_p->cpu_util_float);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsagePercentage_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUSAGEMAXIMUM:
        {
            SYS_MGR_IPC_CpuUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUsageMaximum(&data_p->cpu_stat);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUSAGEAVERAGE:
        {
            SYS_MGR_IPC_CpuUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUsageAverage(&data_p->cpu_stat);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKTIME:
        {
            SYS_MGR_IPC_CpuUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUsagePeakTime(&data_p->cpu_stat);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKDURATION:
        {
            SYS_MGR_IPC_CpuUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUsagePeakDuration(&data_p->cpu_stat);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUTILALARMSTATUS:
        {
            SYS_MGR_IPC_CpuUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            BOOL_T alarm_status;
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUtilAlarmStatus(&alarm_status);
            data_p->cpu_stat = alarm_status;
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuUtilRisingThreshold(data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUtilRisingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuUtilRisingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuUtilFallingThreshold(data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUtilFallingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_CpuUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuUtilFallingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T);
            break;
        }

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
        case SYS_MGR_IPC_CMD_SETCPUGUARDHIGHWATERMARK:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardHighWatermark(data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUGUARDLOWWATERMARK:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardLowWatermark(data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUGUARDMAXTHRESHOLD:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardMaxThreshold(data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUGUARDMINTHRESHOLD:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardMinThreshold(data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUGUARDSTATUS:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardStatus(data_p->status);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETCPUGUARDTRAPSTATUS:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetCpuGuardTrapStatus(data_p->status);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETCPUGUARDINFO:
        {
            SYS_MGR_IPC_CpuGuardInfo_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuGuardInfo(&data_p->info);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuardInfo_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDHIGHWATERMARK:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardHighWatermark(&data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDLOWWATERMARK:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardLowWatermark(&data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMAXTHRESHOLD:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardMaxThreshold(&data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMINTHRESHOLD:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardMinThreshold(&data_p->value);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDSTATUS:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardStatus(&data_p->status);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDTRAPSTATUS:
        {
            SYS_MGR_IPC_CpuGuard_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningCpuGuardTrapStatus(&data_p->status);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T);
            break;
        }
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
        case SYS_MGR_IPC_CMD_GETCPUUTILBYNAME:
        {
            SYS_MGR_IPC_TaskCpuUtil_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetCpuUtilByName(&data_p->cpu_util_info, data_p->get_next);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TaskCpuUtil_T);
            break;
        }
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE) */
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
        case SYS_MGR_IPC_CMD_GETMEMORYUTILIZATIONBRIEF:
        {
            SYS_MGR_IPC_MemoryUsageBrief_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilizationBrief(&data_p->sys_mem_brief);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageBrief_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMEMORYUTILTOTALMEMORY:
        {
            SYS_MGR_IPC_MemorySize_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilTotalMemory(&data_p->mem_size);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemorySize_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMEMORYUTILFREEPERCENTAGE:
        {
            SYS_MGR_IPC_MemoryPercentage_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilFreePercentage(&data_p->percentage);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryPercentage_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMEMORYUTILALARMSTATUS:
        {
            SYS_MGR_IPC_MemoryUsageStat_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilAlarmStatus(&data_p->status);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageStat_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETMEMORYUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetMemoryUtilRisingThreshold(data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMEMORYUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilRisingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILRISINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningMemoryUtilRisingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_SETMEMORYUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_SetMemoryUtilFallingThreshold(data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETMEMORYUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetMemoryUtilFallingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILFALLINGTHRESHOLD:
        {
            SYS_MGR_IPC_MemoryUsageThreshold_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_MGR_GetRunningMemoryUtilFallingThreshold(&data_p->threshold);
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T);
            break;
        }
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
        case SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_IN_TIME:
        {
            SYS_MGR_IPC_NextReloadInTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_QueryNextReloadInTime(
                data_p->remain_seconds,
                &data_p->next_reload_time
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadInTime_T);
            break;
        }
        case SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_AT_TIME:
        {
            SYS_MGR_IPC_NextReloadAtTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_QueryNextReloadAtTime(
                data_p->reload_at,
                &data_p->next_reload_time,
                &data_p->function_active
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T);
            break;
        }
        case SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_REGULARITY_TIME:
        {
            SYS_MGR_IPC_NextReloadRegularityTime_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_QueryNextReloadRegularityTime(
                data_p->reload_regularity,
                &data_p->next_reload_time,
                &data_p->function_active
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadRegularityTime_T);
            break;
        }
        case SYS_MGR_IPC_CMD_GET_RELOAD_TIME_INFOR:
        {
            SYS_MGR_IPC_ReloadTimeInfo_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetReloadTimeInfo(
                &data_p->remain_seconds,
                &data_p->reload_time
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadTimeInfo_T);
            break;
        }
        case SYS_MGR_IPC_CMD_SET_RELOAD_IN:
        {
            SYS_MGR_IPC_ReloadIn_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_SetReloadIn(
                data_p->minute,
                (UI8_T*)""
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SET_RELOAD_AT:
        {
            SYS_MGR_IPC_ReloadAt_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_SetReloadAt(
                data_p->reload_at,
                (UI8_T*)""
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_SET_RELOAD_REGULARITY:
        {
            SYS_MGR_IPC_ReloadRegularity_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_SetReloadRegularity(
                data_p->reload_regularity,
                (UI8_T*)""
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_GET_RELOAD_IN_INFO:
        {
            SYS_MGR_IPC_ReloadInInfo_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetReloadInInfo(
                &data_p->remain_seconds,
                &data_p->next_reload_time
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadInInfo_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_RELOAD_AT_INFO:
        {
            SYS_MGR_IPC_ReloadAtInfo_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetReloadAtInfo(
                &data_p->reload_at,
                &data_p->next_reload_time,
                &data_p->remain_seconds,
                &data_p->function_active
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAtInfo_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_AT_INFO:
        {
            SYS_MGR_IPC_ReloadAt_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetRunningReloadAtInfo(
                &data_p->reload_at
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAt_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_RELOAD_REGULARITY_INFO:
        {
            SYS_MGR_IPC_ReloadRegularityInfo_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetReloadRegularityInfo(
                &data_p->reload_regularity,
                &data_p->next_reload_time,
                &data_p->remain_seconds,
                &data_p->function_active
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularityInfo_T);
            break;
        }

        case SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_REGULARITY_INFO:
        {
            SYS_MGR_IPC_ReloadRegularity_T *data_p = SYS_MGR_MSG_DATA(ipcmsg_p);
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_GetRunningReloadRegularityInfo(
                &data_p->reload_regularity
                );
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularity_T);
            break;
        }

        case SYS_MGR_IPC_CMD_RELOAD_IN_CANCEL:
        {
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_ReloadInCancel();
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_RELOAD_AT_CANCEL:
        {
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_ReloadAtCancel();
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SYS_MGR_IPC_CMD_RELOAD_REGULARITY_CANCEL:
        {
            SYS_MGR_MSG_RETVAL(ipcmsg_p) = SYS_RELOAD_MGR_ReloadRegularityCancel();
            ipcmsg_p->msg_size = SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */

        default:
            SYS_MGR_MSG_RETVAL(ipcmsg_p)=0;
            ipcmsg_p->msg_size=sizeof(SYS_MGR_IPCMsg_Header_T);
            printf("%s(): Invalid cmd.%ld\n", __FUNCTION__,(long)SYS_MGR_MSG_CMD(ipcmsg_p));
         break;
    }
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_SetDebugLevel
 *-----------------------------------------------------------------
 * FUNCTION: Set debug level
 * INPUT   : func
 *           level
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None.
 *----------------------------------------------------------------*/
void SYS_MGR_SetDebugLevel(SYS_MGR_Debug_Func_T func, SYS_MGR_Debug_Level_T level)
{
    if (func < SYS_MGR_DBG_F_MAX)
    {
        sys_mgr_func_dbg_lvl[func] = level;
    }
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetDebugLevel
 *-----------------------------------------------------------------
 * FUNCTION: Get debug level
 * INPUT   : func
 * OUTPUT  : None
 * RETURN  : level
 * NOTE    : None.
 *----------------------------------------------------------------*/
SYS_MGR_Debug_Level_T SYS_MGR_GetDebugLevel(SYS_MGR_Debug_Func_T func)
{
    if (func < SYS_MGR_DBG_F_MAX)
    {
        return sys_mgr_func_dbg_lvl[func];
    }

    return 0;
}

