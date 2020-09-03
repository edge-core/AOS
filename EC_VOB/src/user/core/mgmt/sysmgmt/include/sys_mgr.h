#ifndef SYS_MGR_H
#define SYS_MGR_H

/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_MGR.H
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the all system
 *          wide parameters.
 *
 * Note: 1. For SYS_MGR_Getxxx routine, caller need to provide the storing
 *          Buffer for display string type.
 *       2. The naming constant defined in this package shall be reused by
 *          all the BNBU L2/L3 switch projects.
 *       3. This package shall be reusable for all all the BNBU L2/L3 switch projects.
 *
 *
 *
 *  History
 *
 *   Jason Hsue     10/30/2001      new created
 *   Charles Cheng   3/12/2003      new mechanism for fan status detection
 *   Echo Chen       8/02/2007      modified for linux platform
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "stktplg_pom.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
#include "sys_reload_mgr.h"
#endif
#include "sysmgmt_type.h"

#define SYS_MGR_CPU_UTIL_TASK_NAME_MAX_LEN      SYSFUN_TASK_NAME_LENGTH

/* TYPE DECLARATIONS
 */
typedef enum
{
    SYS_MGR_DBG_F_CPU_UTIL,
    SYS_MGR_DBG_F_MAX,
} SYS_MGR_Debug_Func_T;

typedef enum
{
    SYS_MGR_DBG_L_CRIT,
    SYS_MGR_DBG_L_ERR,
    SYS_MGR_DBG_L_WARN,
    SYS_MGR_DBG_L_VERB,
    SYS_MGR_DBG_L_TRACE,    /* verbose for timer/polling */
    SYS_MGR_DBG_L_MAX,
} SYS_MGR_Debug_Level_T;

typedef struct SYS_MGR_CpuRec_Stat_Set_S
{
    UI32_T  used_ticks_5sec;
    UI32_T  used_ticks_1min;
    UI32_T  used_ticks_5min;
    UI32_T  used_ticks_15min;
    UI32_T  accumulated_ticks;
} SYS_MGR_CpuRec_Stat_Set_T;

typedef struct SYS_MGR_CpuRec_Stat_Entry_S
{
    UI32_T                    tid;
    SYS_MGR_CpuRec_Stat_Set_T task_stat;
} SYS_MGR_CpuRec_Stat_Entry_T;

/* Refer leaf3526A.h
 */
typedef enum
{
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    SYS_MGR_UART_BAUDRATE_AUTO   = 0,
#endif
    SYS_MGR_UART_BAUDRATE_1200   = 1200,
    SYS_MGR_UART_BAUDRATE_2400   = 2400,
    SYS_MGR_UART_BAUDRATE_4800   = 4800,
    SYS_MGR_UART_BAUDRATE_9600 = 9600,
    SYS_MGR_UART_BAUDRATE_19200 = 19200,
    SYS_MGR_UART_BAUDRATE_38400 = 38400,
    SYS_MGR_UART_BAUDRATE_57600 = 57600,
    SYS_MGR_UART_BAUDRATE_115200 = 115200

} SYS_MGR_Uart_BaudRate_T;

#define SYS_MGR_NUMBER_OF_SUPPORT_BAUDRATE      8
#define SYS_MGR_MAX_BAUDRATE                SYS_ADPT_MAX_UART_BAUDRATE
#define SYS_MGR_MIN_BAUDRATE                SYS_ADPT_MIN_UART_BAUDRATE


typedef enum
{
    SYS_MGR_UART_PARITY_NONE = 0,
    SYS_MGR_UART_PARITY_EVEN,
    SYS_MGR_UART_PARITY_ODD,
    SYS_MGR_UART_PARITY_MARK,
    SYS_MGR_UART_PARITY_SPACE
} SYS_MGR_Uart_Parity_T;

typedef enum
{
    SYS_MGR_UART_DATA_LENGTH_5_BITS = 5,
    SYS_MGR_UART_DATA_LENGTH_6_BITS = 6,
    SYS_MGR_UART_DATA_LENGTH_7_BITS = 7,
    SYS_MGR_UART_DATA_LENGTH_8_BITS = 8
} SYS_MGR_Uart_Data_Length_T;

typedef enum
{
    SYS_MGR_UART_STOP_BITS_1_BITS   = 1,
    SYS_MGR_UART_STOP_BITS_2_BITS   = 2
} SYS_MGR_Uart_Stop_Bits_T;


/* TYPE DECLARATIONS
 */
typedef struct SYS_MGR_Console_S
{
    UI32_T  password_threshold;
    UI32_T  exec_timeout;
    UI32_T  login_timeout;
    UI32_T  silent_time;
} SYS_MGR_Console_T;

typedef struct SYS_MGR_Telnet_S
{
    UI32_T  password_threshold;
    UI32_T  exec_timeout;
    UI32_T  login_timeout;
    UI32_T  silent_time;
} SYS_MGR_Telnet_T;

/* RS232, unit wise */
typedef struct SYS_MGR_Uart_Cfg_S
{
    UI32_T  baudrate;
    UI8_T   parity;
    UI8_T   data_length;
    UI8_T   stop_bits;
} SYS_MGR_Uart_Cfg_T;

typedef struct SYS_MGR_Uart_RunningCfg_S
{
    UI32_T  baudrate;
    UI8_T   parity;
    UI8_T   data_length;
    UI8_T   stop_bits;
    BOOL_T  baurdrate_changed;  /* TRUE: changed,  FALSE: no changed  */
    BOOL_T  parity_changed;
    BOOL_T  data_length_changed;
    BOOL_T  stop_bits_changed;
} SYS_MGR_Uart_RunningCfg_T;

typedef struct SYS_MGR_WatchDogExceptionInfo_S
{
    UI32_T   reg_lr;        /* Link Register        */
    UI32_T   reg_srr0;      /* SRR0 Register        */
    UI32_T   reg_srr1;      /* SRR1 Register        */
    UI32_T   reg_cr;        /* Condition Register   */
    UI32_T   reg_r1;        /* R1   Register        */

    UI8_T    name[20];      /* pointer to task name          */
    UI32_T   priority;      /* task priority                 */
    UI32_T   status;        /* task status                   */
    UI32_T   delay;         /* delay/timeout ticks           */
    UI32_T   pStackBase;    /* points to bottom of stack     */
    UI32_T   pStackLimit;   /* points to stack limit         */
    UI32_T   pStackEnd;     /* points to init stack limit    */
    UI32_T   stackSize;     /* size of stack in bytes        */
    UI32_T   stackCurrent;  /* current stack usage in bytes  */
    UI32_T   stackMargin;   /* current stack margin in bytes */

} SYS_MGR_WatchDogExceptionInfo_T;


typedef struct SYS_MGR_PowerStatus_S
{
    UI32_T  sw_unit_index;                 /* index */
    UI32_T  sw_power_status;
} SYS_MGR_PowerStatus_T;


typedef struct SYS_MGR_Switch_Info_S
{
    UI32_T  sw_indiv_power_unit_index;   /* index */
    UI32_T  sw_indiv_power_index;        /* index */
    UI32_T  sw_indiv_power_status;
    UI32_T  sw_indiv_power_type;         /* VAL_swIndivPowerType_XXX */
} SYS_MGR_IndivPower_T;

typedef struct SYS_MGR_SwAlarmEntry_S
{
    UI32_T  sw_alarm_unit_index;   /* index */
    UI32_T  sw_alarm_input_index;  /* index */
    char    sw_alarm_input_name[MAXSIZE_swAlarmInputName+1];
    UI32_T  sw_alarm_status;
    UI32_T  sw_alarm_status_2;
} SYS_MGR_SwAlarmEntry_T;

/* from proprietary MIB
 */
typedef struct SYS_MGR_SwitchFanEntry_S
{
    UI32_T  switch_unit_index;   /* key */
    UI32_T  switch_fan_index;    /* key */
    UI32_T  switch_fan_status;
    UI32_T  switch_fan_fail_counter;
    UI32_T  switch_fan_speed;
    UI32_T  switch_fan_oper_speed;
} SYS_MGR_SwitchFanEntry_T;

/* from proprietary MIB
 */
typedef struct SYS_MGR_SwitchThermalEntry_S
{
    UI32_T  switch_unit_index;   /* key */
    UI32_T  switch_thermal_index;    /* key */
    I32_T   switch_thermal_temp_value;
} SYS_MGR_SwitchThermalEntry_T;

typedef struct
{
    UI32_T      unit_index;         /* key */
    UI32_T      thermal_index;      /* key */
    UI32_T      action_index;       /* key */
    I32_T       rising_threshold;
    I32_T       falling_threshold;
    UI32_T      action;            /* bits: bit 0:none; bit 1:trap */
    UI32_T      status;
} SYS_MGR_SwitchThermalActionEntry_T;

typedef struct
{
    MEM_SIZE_T free_bytes;
    MEM_SIZE_T free_blocks;
    MEM_SIZE_T free_max_block_size;
    MEM_SIZE_T used_bytes;
    MEM_SIZE_T used_blocks;
} SYS_MGR_MemoryUtilBrief_T;

typedef struct
{
    UI32_T cpu_util;            /* cpu usage (X%) */
    UI32_T cpu_util_float;      /* fractional part of cpu usage (X * 0.001%) */
    UI32_T cpu_util_max;
    UI32_T cpu_util_max_float;
    UI32_T cpu_util_avg;
    UI32_T cpu_util_avg_float;
    char task_name[SYS_MGR_CPU_UTIL_TASK_NAME_MAX_LEN+1];   /* key */
} SYS_MGR_TaskCpuUtilInfo_T;

typedef struct SYS_MGR_CpuGuardInfo_S
{
    UI32_T watermark_high;
    UI32_T watermark_low;
    UI32_T threshold_max;
    UI32_T threshold_min;
    UI32_T cpu_rate;
    BOOL_T status;
    BOOL_T trap_status;
} SYS_MGR_CpuGuardInfo_T;

/* Add funtion typedef macro for pmgr function*/

/* MACRO FUNCTION DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SYS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of SYS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_MGR_GET_MSGBUFSIZE(datatype_name) \
        ( sizeof(SYS_MGR_IPCMsg_Header_T) + sizeof (datatype_name) )

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SYS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of SYS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_MGR_MSG_CMD
 *              SYS_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the SYS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The SYS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_MGR_MSG_CMD(msg_p)    (((SYS_MGR_IPCMsg_T *)msg_p->msg_buf)->header.cmd)
#define SYS_MGR_MSG_RETVAL(msg_p) (((SYS_MGR_IPCMsg_T *)msg_p->msg_buf)->header.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_MGR_MSG_DATA(msg_p)   ((void *)&((SYS_MGR_IPCMsg_T *)msg_p->msg_buf)->data)

/* MGR handler will use this when it can't handle the message.
 *                                  (is in transition mode)
 */
#define SYS_MGR_IPC_RESULT_FAIL  (-1)

/* definitions of command in sys_mgr which will be used in ipc message
 */
enum
{
    SYS_MGR_IPC_CMD_GETCONSOLECFG,
    SYS_MGR_IPC_CMD_GETRUNNINGPASSWORDTHRESHOLD,
    SYS_MGR_IPC_CMD_SETPASSWORDTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGCONSOLEEXECTIMEOUT,
    SYS_MGR_IPC_CMD_SETCONSOLEEXECTIMEOUT,
    SYS_MGR_IPC_CMD_GETRUNNINGCONSOLESILENTTIME,
    SYS_MGR_IPC_CMD_SETCONSOLESILENTTIME,
    SYS_MGR_IPC_CMD_GET_RUNNING_TELNET_SILENT_TIME,
    SYS_MGR_IPC_CMD_SET_TELNET_SILENT_TIME,
    SYS_MGR_IPC_CMD_GETTELNETCFG,
    SYS_MGR_IPC_CMD_GETRUNNINGTELNETPASSWORDTHRESHOLD,
    SYS_MGR_IPC_CMD_SETTELNETPASSWORDTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGTELNETEXECTIMEOUT,
    SYS_MGR_IPC_CMD_SETTELNETEXECTIMEOUT,
    SYS_MGR_IPC_CMD_GETUARTPARAMETERS,
    SYS_MGR_IPC_CMD_GETUARTOPERBAUDRATE,
    SYS_MGR_IPC_CMD_GETAUTOBAUDRATESWITCH,
    SYS_MGR_IPC_CMD_GETRUNNINGUARTPARAMETERS,
    SYS_MGR_IPC_CMD_SETUARTBAUDRATE,
    SYS_MGR_IPC_CMD_SETUARTPARITY,
    SYS_MGR_IPC_CMD_SETUARTDATABITS,
    SYS_MGR_IPC_CMD_SETUARTSTOPBITS,
    SYS_MGR_IPC_CMD_GETPROMPTSTRING,
    SYS_MGR_IPC_CMD_SETPROMPTSTRING,
    SYS_MGR_IPC_CMD_GETRUNNINGPROMPTSTRING,
    SYS_MGR_IPC_CMD_LOGWATCHDOGEXCEPTIONINFO,
    SYS_MGR_IPC_CMD_GETPOWERSTATUS,
    SYS_MGR_IPC_CMD_GETNEXTPOWERSTATUS,
    SYS_MGR_IPC_CMD_GETSWITCHINDIVPOWER,
    SYS_MGR_IPC_CMD_GETNEXTSWITCHINDIVPOWER,
    SYS_MGR_IPC_CMD_GETSWALARMINPUTSTATUS,
    SYS_MGR_IPC_CMD_GETMAJORALARMOUTPUTSTATUS,
    SYS_MGR_IPC_CMD_GETMINORALARMOUTPUTSTATUS,
    SYS_MGR_IPC_CMD_GETALARMOUTPUTSTATUS,
    SYS_MGR_IPC_CMD_GETNEXTALARMOUTPUTSTATUS,
    SYS_MGR_IPC_CMD_GETFANSTATUS,
    SYS_MGR_IPC_CMD_GETNEXTFANSTATUS,
    SYS_MGR_IPC_CMD_GETFANSPEED,
    SYS_MGR_IPC_CMD_GETNEXTFANSPEED,
    SYS_MGR_IPC_CMD_SETFANSPEED,
    SYS_MGR_IPC_CMD_SETFANSPEEDFORCEFULL,
    SYS_MGR_IPC_CMD_GETFANSPEEDFORCEFULL,
    SYS_MGR_IPC_CMD_GETRUNNINGFANSPEEDFORCEFULL,
    SYS_MGR_IPC_CMD_FANSTATUSCHANGED,
    SYS_MGR_IPC_CMD_GETTHERMALSTATUS,
    SYS_MGR_IPC_CMD_GETNEXTTHERMALSTATUS,
    SYS_MGR_IPC_CMD_GETDEFAULTSWITCHTHERMALACTIONENTRY,
    SYS_MGR_IPC_CMD_GETSWITCHTHERMALACTIONENTRY,
    SYS_MGR_IPC_CMD_GETNEXTSWITCHTHERMALACTIONENTRY,
    SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONENTRY,
    SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONACTION,
    SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONSTATUS,
    SYS_MGR_IPC_CMD_THERMALSTATUSCHANGED,
    SYS_MGR_IPC_CMD_SETCONSOLELOGINTIMEOUT,
    SYS_MGR_IPC_CMD_SETTELNETLOGINTIMEOUT,
    SYS_MGR_IPC_CMD_GETRUNNINGCONSOLELOGINTIMEOUT,
    SYS_MGR_IPC_CMD_GETRUNNINGTELNETLOGINTIMEOUT,
    SYS_MGR_IPC_CMD_GETSYSDESCR,
    SYS_MGR_IPC_CMD_GETSYSOBJECTID,
    SYS_MGR_IPC_CMD_GETPRODUCTNAME,
    SYS_MGR_IPC_CMD_GETDEVICENAME,
    SYS_MGR_IPC_CMD_GETPRIVATEMIBROOT,
    SYS_MGR_IPC_CMD_GETMODELNAME,
    SYS_MGR_IPC_CMD_GET_PRODUCT_MANUFACTURER,
    SYS_MGR_IPC_CMD_GET_PRODUCT_DESCRIPTION,
    SYS_MGR_IPC_CMD_GETCPUUSAGEPERCENTAGE,
    SYS_MGR_IPC_CMD_GETCPUUSAGEMAXIMUM,
    SYS_MGR_IPC_CMD_GETCPUUSAGEAVERAGE,
    SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKTIME,
    SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKDURATION,
    SYS_MGR_IPC_CMD_GETCPUUTILALARMSTATUS,
    SYS_MGR_IPC_CMD_SETCPUUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETCPUUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_SETCPUUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETCPUUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_SETCPUGUARDHIGHWATERMARK,
    SYS_MGR_IPC_CMD_SETCPUGUARDLOWWATERMARK,
    SYS_MGR_IPC_CMD_SETCPUGUARDMAXTHRESHOLD,
    SYS_MGR_IPC_CMD_SETCPUGUARDMINTHRESHOLD,
    SYS_MGR_IPC_CMD_SETCPUGUARDSTATUS,
    SYS_MGR_IPC_CMD_SETCPUGUARDTRAPSTATUS,
    SYS_MGR_IPC_CMD_GETCPUGUARDINFO,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDHIGHWATERMARK,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDLOWWATERMARK,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMAXTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMINTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDSTATUS,
    SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDTRAPSTATUS,
    SYS_MGR_IPC_CMD_GETCPUUTILBYNAME,
    SYS_MGR_IPC_CMD_GETMEMORYUTILIZATIONBRIEF,
    SYS_MGR_IPC_CMD_GETMEMORYUTILTOTALMEMORY,
    SYS_MGR_IPC_CMD_GETMEMORYUTILFREEPERCENTAGE,
    SYS_MGR_IPC_CMD_GETMEMORYUTILALARMSTATUS,
    SYS_MGR_IPC_CMD_SETMEMORYUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETMEMORYUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILRISINGTHRESHOLD,
    SYS_MGR_IPC_CMD_SETMEMORYUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETMEMORYUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILFALLINGTHRESHOLD,
    SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_IN_TIME,
    SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_AT_TIME,
    SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_REGULARITY_TIME,
    SYS_MGR_IPC_CMD_GET_RELOAD_TIME_INFOR,
    SYS_MGR_IPC_CMD_SET_RELOAD_IN,
    SYS_MGR_IPC_CMD_SET_RELOAD_AT,
    SYS_MGR_IPC_CMD_SET_RELOAD_REGULARITY,
    SYS_MGR_IPC_CMD_GET_RELOAD_IN_INFO,
    SYS_MGR_IPC_CMD_GET_RELOAD_AT_INFO,
    SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_AT_INFO,
    SYS_MGR_IPC_CMD_GET_RELOAD_REGULARITY_INFO,
    SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_REGULARITY_INFO,
    SYS_MGR_IPC_CMD_RELOAD_IN_CANCEL,
    SYS_MGR_IPC_CMD_RELOAD_AT_CANCEL,
    SYS_MGR_IPC_CMD_RELOAD_REGULARITY_CANCEL,
    SYS_MGR_IPC_CMD_GETNEXTSWALARMINPUT,
    SYS_MGR_IPC_CMD_GETSWALARMINPUT,
    SYS_MGR_IPC_CMD_GETSWALARMINPUTNAME,
    SYS_MGR_IPC_CMD_SETSWALARMINPUTNAME,
    SYS_MGR_IPC_CMD_GETRUNNINGALARMINPUTNAME,
};


/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in sys pmgr and mgr
 */

/* Message declarations for IPC.
 */
typedef struct
{
    SYS_MGR_Console_T consolecfg ;
} SYS_MGR_IPCMsg_ConsoleCfg_T;

typedef struct
{
    UI32_T passwordthreshold;
} SYS_MGR_IPCMsg_PasswordThreshold_T;

typedef struct
{
    UI32_T time_out_value;
} SYS_MGR_IPC_ExecTimeOut_T;

typedef struct
{
    UI32_T silent_time;
} SYS_MGR_IPC_ConsoleSilentTime_T;

typedef struct
{
    SYS_MGR_Telnet_T telnetcfg;
} SYS_MGR_IPC_TelnetCfg_T;

typedef struct
{
    SYS_MGR_Uart_Cfg_T uartcfg;
} SYS_MGR_IPC_UartCfg_T;

typedef struct
{
   SYS_MGR_Uart_BaudRate_T uart_operbaudrate;
} SYS_MGR_IPC_UartBaudRate_T;

typedef struct
{

} SYS_MGR_IPC_AutoBaudRate_T;

typedef struct
{
    SYS_MGR_Uart_RunningCfg_T uart_runningcfg;
} SYS_MGR_IPC_UartRunningCfg_T;

typedef struct
{
    SYS_MGR_Uart_Parity_T parity;
} SYS_MGR_IPC_UartParity_T;

typedef struct
{
    SYS_MGR_Uart_Data_Length_T data_length;
} SYS_MGR_IPC_UartDataBits_T;

typedef struct
{
    SYS_MGR_Uart_Stop_Bits_T stop_bits;
} SYS_MGR_IPC_UartStopBits_T;

typedef struct
{
    UI8_T prompt_string[ SYS_ADPT_MAX_PROMPT_STRING_LEN + 1 ];
} SYS_MGR_IPC_PromptString_T;

typedef struct
{
    SYS_MGR_WatchDogExceptionInfo_T	wd_own;
} SYS_MGR_IPC_LogWatchDogExceptionInfo_T;
typedef struct
{
    SYS_MGR_PowerStatus_T power_status;
} SYS_MGR_IPC_PowerStatus_T;

typedef struct
{
    SYS_MGR_IndivPower_T indiv_power ;
} SYS_MGR_IPC_SwitchIndivPower_T;

typedef struct
{
    SYS_MGR_SwAlarmEntry_T sw_alarm;
} SYS_MGR_IPC_SwAlarm_T;

typedef struct
{
    SYS_MGR_SwitchFanEntry_T switch_fan_status;
} SYS_MGR_IPC_FanStatus_T;

typedef struct
{
    BOOL_T mode;
} SYS_MGR_IPC_FanSpeedForceFull_T;

typedef struct
{
    SYS_MGR_SwitchThermalEntry_T switch_thermal_status;
} SYS_MGR_IPC_SwitchThermalEntry_T;

typedef struct
{
    SYS_MGR_SwitchThermalActionEntry_T action_entry;
} SYS_MGR_IPC_SwitchThermalActionEntry_T;

typedef struct
{
    UI32_T unit_index;
    UI32_T thermal_index;
    UI32_T index;
    UI32_T threshold_action;
} SYS_MGR_IPC_SwitchThermalActionThreshold_T;

typedef struct
{
    UI32_T time_out_value;
} SYS_MGR_IPC_LoginTimeOut_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T sys_descrption[ MAXSIZE_sysDescr+1 ];
} SYS_MGR_IPC_SysDescr_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T sys_oid[ SYS_ADPT_MAX_OID_STRING_LEN+1  ];
} SYS_MGR_IPC_SysObjectID_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T product_name[ MAXSIZE_swProdName +1 ];
} SYS_MGR_IPC_ProductName_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T device_name[ SYS_ADPT_DEVICE_NAME_STRING_LEN +1 ];
} SYS_MGR_IPC_DeviceName_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T mib_root[ SYS_ADPT_MAX_OID_STRING_LEN+1  ];
} SYS_MGR_IPC_MibRoot_T;

typedef struct
{
    UI32_T unit_id;
    UI8_T model_name[ SYS_ADPT_MAX_MODEL_NAME_SIZE+1  ];
} SYS_MGR_IPC_ModelName_T;

typedef struct
{
    char product_manufacturer[MAXSIZE_swProdManufacturer + 1];
} SYS_MGR_IPC_ProductManufacturer_T;

typedef struct
{
    char product_description[MAXSIZE_swProdDescription + 1];
} SYS_MGR_IPC_ProductDescription_T;

typedef struct
{
    UI32_T cpu_util;
    UI32_T cpu_util_float;
} SYS_MGR_IPC_CpuUsagePercentage_T;

typedef struct
{
    UI32_T cpu_stat;
} SYS_MGR_IPC_CpuUsageStat_T;

typedef struct
{
    UI32_T threshold;
} SYS_MGR_IPC_CpuUsageThreshold_T;

typedef struct
{
    SYS_MGR_TaskCpuUtilInfo_T cpu_util_info;
    BOOL_T get_next;
} SYS_MGR_IPC_TaskCpuUtil_T;

typedef struct
{
    SYS_MGR_CpuGuardInfo_T info;
} SYS_MGR_IPC_CpuGuardInfo_T;

typedef union
{
    UI32_T value;
    BOOL_T status;
} SYS_MGR_IPC_CpuGuard_T;

typedef struct
{
    SYS_MGR_MemoryUtilBrief_T sys_mem_brief;
} SYS_MGR_IPC_MemoryUsageBrief_T;

typedef struct
{
    MEM_SIZE_T mem_size;
} SYS_MGR_IPC_MemorySize_T;

typedef struct
{
    UI32_T percentage;
} SYS_MGR_IPC_MemoryPercentage_T;

typedef struct
{
    BOOL_T status;
} SYS_MGR_IPC_MemoryUsageStat_T;

typedef struct
{
    UI32_T threshold;
} SYS_MGR_IPC_MemoryUsageThreshold_T;

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)

typedef struct
{
    I32_T remain_seconds;
    SYS_TIME_DST next_reload_time;
} SYS_MGR_IPC_NextReloadInTime_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADAT_DST reload_at;
    SYS_TIME_DST next_reload_time;
    BOOL_T function_active;
} SYS_MGR_IPC_NextReloadAtTime_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity;
    SYS_TIME_DST next_reload_time;
    BOOL_T function_active;
} SYS_MGR_IPC_NextReloadRegularityTime_T;

typedef struct
{
    I32_T remain_seconds;
    SYS_TIME_DST reload_time;
} SYS_MGR_IPC_ReloadTimeInfo_T;

typedef struct
{
    UI32_T minute;
} SYS_MGR_IPC_ReloadIn_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADAT_DST reload_at;
} SYS_MGR_IPC_ReloadAt_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity;
} SYS_MGR_IPC_ReloadRegularity_T;

typedef struct
{
    I32_T   remain_seconds;
    SYS_TIME_DST    next_reload_time;
} SYS_MGR_IPC_ReloadInInfo_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADAT_DST  reload_at;
    SYS_TIME_DST    next_reload_time;
    I32_T   remain_seconds;
    BOOL_T  function_active;
} SYS_MGR_IPC_ReloadAtInfo_T;

typedef struct
{
    SYS_RELOAD_OM_RELOADREGULARITY_DST  reload_regularity;
    SYS_TIME_DST    next_reload_time;
    I32_T   remain_seconds;
    BOOL_T  function_active;
} SYS_MGR_IPC_ReloadRegularityInfo_T;
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */

typedef struct
{
    /* empty struct.
     */
} SYS_MGR_IPCMsg_EmptyData_T;

typedef union
{
        UI32_T cmd;    /* for sending IPC request. SYS_MGR_IPC_CMD1 ... */
        UI32_T result; /* for response */
}  SYS_MGR_IPCMsg_Header_T;

typedef union
{
    SYS_MGR_IPCMsg_ConsoleCfg_T consolecfg_data;
    SYS_MGR_IPCMsg_PasswordThreshold_T passwordthreshold_data;
    SYS_MGR_IPC_ExecTimeOut_T exectimeout_data;
    SYS_MGR_IPC_ConsoleSilentTime_T silent_time_data;
    SYS_MGR_IPC_TelnetCfg_T  telnetcfg_data;
    SYS_MGR_IPC_UartCfg_T uartcfg_data;
    SYS_MGR_IPC_UartBaudRate_T uartbaudrate_data;
    SYS_MGR_IPC_AutoBaudRate_T autobaudrate_data;
    SYS_MGR_IPC_UartRunningCfg_T uart_runningcfg_data;
    SYS_MGR_IPC_UartParity_T parity_data;
    SYS_MGR_IPC_UartDataBits_T databits_data;
    SYS_MGR_IPC_UartStopBits_T stopbits_data;
    SYS_MGR_IPC_PromptString_T promptstring_data;
    SYS_MGR_IPC_LogWatchDogExceptionInfo_T wd_data;
    SYS_MGR_IPC_PowerStatus_T powerstatus_data;
    SYS_MGR_IPC_SwitchIndivPower_T indiv_power_data;
    SYS_MGR_IPC_FanStatus_T switch_fan_status_data;
    SYS_MGR_IPC_SwitchThermalEntry_T switch_thermal_status_data;
    SYS_MGR_IPC_SwitchThermalActionEntry_T actionentry_data;
    SYS_MGR_IPC_SwitchThermalActionThreshold_T actionthreshold_data;
    SYS_MGR_IPC_LoginTimeOut_T logintimeout_data;
    SYS_MGR_IPC_SysObjectID_T sysobjectid_data;
    SYS_MGR_IPC_ProductName_T productname_data;
    SYS_MGR_IPC_DeviceName_T devicename_data;
    SYS_MGR_IPC_MibRoot_T mibroot_data;
    SYS_MGR_IPC_ModelName_T modelname_data;
    SYS_MGR_IPC_ProductManufacturer_T product_manufacturer_data;
    SYS_MGR_IPC_ProductDescription_T  product_description_data;
    SYS_MGR_IPC_CpuUsagePercentage_T cpu_usage_data;
    SYS_MGR_IPC_CpuUsageStat_T cpu_stat_data;
    SYS_MGR_IPC_CpuUsageThreshold_T cpu_threshold_data;
    SYS_MGR_IPC_MemoryUsageBrief_T mem_brief_data;
    SYS_MGR_IPC_MemoryUsageStat_T mem_stat_data;
    SYS_MGR_IPC_MemoryPercentage_T mem_percentage;
	SYS_MGR_IPC_MemoryPercentage_T mem_size;
    SYS_MGR_IPC_MemoryUsageThreshold_T mem_threshold_data;
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_MGR_IPC_NextReloadInTime_T  next_reload_in_tm;
    SYS_MGR_IPC_NextReloadAtTime_T  next_reload_at_tm;
    SYS_MGR_IPC_NextReloadRegularityTime_T  next_reload_regularity_tm;
    SYS_MGR_IPC_ReloadTimeInfo_T    reload_tm_info;
    SYS_MGR_IPC_ReloadIn_T  reload_in;
    SYS_MGR_IPC_ReloadAt_T  reload_at;
    SYS_MGR_IPC_ReloadRegularity_T  reload_regularity;
    SYS_MGR_IPC_ReloadInInfo_T  reload_in_info;
    SYS_MGR_IPC_ReloadAtInfo_T  reload_at_info;
    SYS_MGR_IPC_ReloadRegularityInfo_T  reload_regularity_info;
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */
    SYS_MGR_IPCMsg_EmptyData_T empty_data;
} SYS_MGR_IPCMsg_Data_T;

typedef struct
{
    SYS_MGR_IPCMsg_Header_T header;
    SYS_MGR_IPCMsg_Data_T data;
} SYS_MGR_IPCMsg_T;

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
BOOL_T SYS_MGR_LogWatchDogExceptionInfo(SYS_MGR_WatchDogExceptionInfo_T *wd_own);

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
BOOL_T SYS_MGR_ShowWatchDogTimerLogInfoFromLogFile(void);

/* Define SYS_MGR System Information data type
 *
 * NOTES: 1. All the info followed are read by STK_TPLG.
 *        2. This info is unit based(per unit will have its own info).
 *        3. STK_TPLG will provide same info structure as SYS_MGR and whenever
 *           CLI, WEB or SNMP tries to get system info, SYS_MGR will just call
 *           STK_TPLG to get whole info and pass to caller.
 *        4. If power_status is good, then the value will be TRUE, else FALSE.
 *        5. If redundant_power_status is good, then the value will be TRUE. else FALSE.
 *        6. If the system doesn't provide redundant power, redundant_power_status will
 *           always be FALSE.
 *        7. fan_status is a BIT map variable.  For every bit,
 *              1: fan is fail
 *              0: fan is ok
 *           BIT_0 represents the first fan and BIT_1 tells the second fan's fail status.
 *           So if the fan_fail_status is eaual to zero, it means all fans are OK.
 *        8. fan_number_of_one_box tells how many fans in a box.
 *        9. The example of string format of Serial Number is "ACT01381234"
 *           The meaning is  "ACTyywwnnnn"
 *              ACT  - Brand name or Model name
 *              01   - Year 2001(By Manufacture Year)
 *              38   - the 38th week of Year 2001(By Manufacture Week)
 *              1234 - the 1234th manufactured product in 38th week of Year 2001
 *           The character of "yy" is count from 01, 02, 09, 10, ...., 99
 *           The character of "ww" is count as 01, 02, 09, 10, 11, ..., 52
 *           The character of "nnnn" is used as hexadecial "0 - 9, A, B, C, D, E, F"
 *       10. The format mainboard/agent board Hard Ware Version will be "R01A" or "R0A"
 *       11. If the CPU is on the mainboard, then both mainboard and agent HW version
 *           will be the same, the caller only needs to show one of them.
 *       12. The Software version will be "V1.02.03.04"
 *            1  - Big change, For instance, L2 upgrade to L3
 *            02 - Component added or modified.  For instance, add LACP feature
 *            03 - Minor change or bug fix.
 *            04 - the version which increases for testing.
 *           The character is increased by decimal, ie. V1.02.03.09 -> V1.02.03.10
 */

#define SYS_MGR_Info_T STKTPLG_OM_Info_T

/* Default constant value of Uart config setting
 */
#define SYS_MGR_UART_BAUDRATE_DEF               SYS_DFLT_UART_OPER_BAUDRATE
#define SYS_MGR_UART_OPER_BAUDRATE_DEF          SYS_DFLT_UART_OPER_BAUDRATE
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
    #define SYS_MGR_UART_ADMIN_BAUDRATE_DEF     SYS_MGR_UART_BAUDRATE_AUTO
    #define SYS_MGR_INTERMEDIATE_BAUDRATE_FOR_AUTO_DETECTION    SYS_MGR_UART_BAUDRATE_19200
#else
    #define SYS_MGR_UART_ADMIN_BAUDRATE_DEF     SYS_DFLT_UART_OPER_BAUDRATE
#endif
#define SYS_MGR_UART_PARITY_DEF                 SYS_MGR_UART_PARITY_NONE
#define SYS_MGR_UART_DATA_LENGTH_DEF            SYS_MGR_UART_DATA_LENGTH_8_BITS
#define SYS_MGR_UART_STOP_BITS_DEF              SYS_MGR_UART_STOP_BITS_1_BITS


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYS_MGR_Create_InterCSC_Relation(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_MGR_SetTransitionMode(void);

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
void SYS_MGR_EnterTransitionMode(void);

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
void SYS_MGR_EnterMasterMode(void);

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
void SYS_MGR_EnterSlaveMode(void);

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
BOOL_T SYS_MGR_LoadConfig(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetConsoleCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of console setting
 * INPUT:   None
 * OUTPUT:  console setting(by record)
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetConsoleCfg(SYS_MGR_Console_T *consoleCfg);

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
UI32_T SYS_MGR_GetRunningPasswordThreshold(UI32_T *passwordThreshold);


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
BOOL_T SYS_MGR_SetPasswordThreshold(UI32_T passwordThreshold);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console exec time out  is successfully
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
UI32_T SYS_MGR_GetRunningConsoleExecTimeOut(UI32_T *time_out_value);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetConsoleExecTimeOut(UI32_T time_out_value);


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
UI32_T SYS_MGR_GetRunningConsoleSilentTime(UI32_T *silent_time);


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
BOOL_T SYS_MGR_SetConsoleSilentTime(UI32_T silent_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	 - SYS_MGR_GetRunningTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE:	This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *			the	non-default	console	silent time	is successfully
 *			retrieved.
 *			Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by	CLI	to save	the
 *			 "running configuration" to	local or remote	files.
 *		  2. Since only	non-default	configuration will be saved, this
 *			 function shall	return non-default system name.
 *		  3. Caller	has	to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_MGR_GetRunningTelnetSilentTime(UI32_T *silent_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME	 - SYS_MGR_SetTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE:	This function will set password	retry count
 * INPUT	: silent time value
 * OUTPUT	: None
 * RETURN	: TRUE/FALSE
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetTelnetSilentTime(UI32_T silent_time);

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
BOOL_T SYS_MGR_GetTelnetCfg(SYS_MGR_Telnet_T *telnetCfg);


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
UI32_T SYS_MGR_GetRunningTelnetPasswordThreshold(UI32_T *passwordThreshold);


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
BOOL_T SYS_MGR_SetTelnetPasswordThreshold(UI32_T passwordThreshold);


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
UI32_T SYS_MGR_GetRunningTelnetExecTimeOut(UI32_T *time_out_value);


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
BOOL_T SYS_MGR_SetTelnetExecTimeOut(UI32_T time_out_value);


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
BOOL_T SYS_MGR_GetUartParameters(SYS_MGR_Uart_Cfg_T *uartCfg);

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
BOOL_T SYS_MGR_GetUartOperBaudrate(SYS_MGR_Uart_BaudRate_T *uart_operbaudrate);
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
void SYS_MGR_ProvisionComplete(void);

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
UI32_T SYS_MGR_GetRunningUartParameters(SYS_MGR_Uart_RunningCfg_T *uart_cfg);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set baudrate of serial port
 * INPUT    : SYS_MGR_Uart_BaudRate_T Baudrate
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartBaudrate(SYS_MGR_Uart_BaudRate_T baudrate);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartParity
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Parity of serial port
 * INPUT    : SYS_MGR_Console_Parity_T Parity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartParity(SYS_MGR_Uart_Parity_T parity);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartDataBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set DataLength of serial port
 * INPUT    : SYS_MGR_Uart_Parity_T DataLength
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetUartDataBits(SYS_MGR_Uart_Data_Length_T data_length);


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
BOOL_T SYS_MGR_SetUartStopBits(SYS_MGR_Uart_Stop_Bits_T stop_bits);


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
BOOL_T SYS_MGR_GetSysInfo(UI32_T unit, SYS_MGR_Info_T *sys_info);


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
void SYS_MGR_RestartSystem(void);


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
void SYS_MGR_RestartSystemFromPOST(void);


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
BOOL_T SYS_MGR_GetPromptString(UI8_T *prompt_string);


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
BOOL_T SYS_MGR_SetPromptString(UI8_T *prompt_string);


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
UI32_T SYS_MGR_GetRunningPromptString(UI8_T *prompt_string);

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
BOOL_T SYS_MGR_BaudRateCheck();
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_ActiveBaudRateCheck
 * ---------------------------------------------------------------------
 * PURPOSE  : To search correct baudrate by active sending control sequence
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   : TRUE on success, FALSE on failure
 *
 *
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_ActiveBaudRateCheck();
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_PassiveBaudRateCheck
 * ---------------------------------------------------------------------
 * PURPOSE  : To search correct baudrate when an ENTER key to be entered.
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   : TRUE on success, FALSE on failure
 *
 *
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_PassiveBaudRateCheck();
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
BOOL_T SYS_MGR_PollBaudRate(BOOL_T *is_key_detected, UI8_T *user_input , UI8_T *user_input_length);
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
BOOL_T SYS_MGR_GuessBaudrate(UI8_T* ch,UI8_T ch_length, SYS_MGR_Uart_BaudRate_T current_baudrate, SYS_MGR_Uart_BaudRate_T *sugg_baudrate);

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
BOOL_T SYS_MGR_Get_Autobaudrate_Switch();
#endif


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
BOOL_T SYS_MGR_GetPowerStatus(SYS_MGR_PowerStatus_T *power_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_MGR_GetNextSwitchInfo
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
BOOL_T SYS_MGR_GetNextPowerStatus(SYS_MGR_PowerStatus_T *power_status);


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
BOOL_T SYS_MGR_GetSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power);


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
BOOL_T SYS_MGR_GetNextSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power);

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
BOOL_T SYS_MGR_GetSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetNextSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetSwAlarmInputStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_SetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetMajorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetMinorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
BOOL_T SYS_MGR_GetNextAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm);

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
void SYS_MGR_Register_PowerStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T power, UI32_T status));

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_FanStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- callback function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchFanStatus_ok/VAL_switchFanStatus_failure.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_FanStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T fan, UI32_T status));

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
BOOL_T  SYS_MGR_FanStatusChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T status);

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
BOOL_T  SYS_MGR_FanSpeedChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T speed);

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
void SYS_MGR_SimulateFanSpeedChanged(UI32_T unit, UI32_T fan, UI32_T speed);

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
BOOL_T SYS_MGR_GetFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

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
BOOL_T SYS_MGR_GetNextFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

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
BOOL_T SYS_MGR_GetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

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
BOOL_T SYS_MGR_GetNextFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To set the next fan speed.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status);

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
BOOL_T SYS_MGR_SetFanSpeedForceFull(BOOL_T mode);

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
BOOL_T SYS_MGR_GetFanSpeedForceFull(BOOL_T *mode);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningFanSpeedForceFull(BOOL_T *mode);
#endif
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Register_ThermalStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Register the callback function, when a fan status is changed
 *            the registered function will be called.
 * INPUT    : fun -- callback function pointer
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : When callback the registered function, the argument status is
 *            VAL_switchThermalStatus_ok/VAL_switchThermalStatus_failure.
 * ---------------------------------------------------------------------
 */
void SYS_MGR_Register_ThermalStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T thermal, UI32_T status));

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
BOOL_T SYS_MGR_GetThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status);

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
BOOL_T SYS_MGR_GetNextThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDefaultSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the default switch thermal action entry.
 * INPUT    : *entry --- default entry pointer
 * OUTPUT   : *entry --- default entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetDefaultSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

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
BOOL_T SYS_MGR_GetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

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
BOOL_T SYS_MGR_GetNextSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

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
BOOL_T SYS_MGR_SetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry);

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
BOOL_T SYS_MGR_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T rising_threshold);

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
BOOL_T SYS_MGR_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T falling_threshold);

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
BOOL_T SYS_MGR_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T action);

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
BOOL_T SYS_MGR_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T status);

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
BOOL_T	SYS_MGR_ThermalStatusChanged_CallBack(UI32_T unit, UI32_T thermal_idx, I32_T temperature);

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

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
BOOL_T SYS_MGR_SetConsoleLoginTimeOut(UI32_T time_out_value);


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
BOOL_T SYS_MGR_SetTelnetLoginTimeOut(UI32_T time_out_value);


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
UI32_T SYS_MGR_GetRunningConsoleLoginTimeOut(UI32_T *time_out_value);


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
UI32_T SYS_MGR_GetRunningTelnetLoginTimeOut(UI32_T *time_out_value);


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
BOOL_T SYS_MGR_GetSysDescr(UI32_T unit_id, UI8_T *sys_descrption);

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
BOOL_T SYS_MGR_GetSysObjectID(UI32_T unit_id, UI8_T *sys_oid);

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
BOOL_T SYS_MGR_GetProductName(UI32_T unit_id, UI8_T *product_name);

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
BOOL_T SYS_MGR_GetDeviceName(UI32_T unit_id, UI8_T *device_name);

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
BOOL_T SYS_MGR_GetPrivateMibRoot(UI32_T unit_id, UI8_T *mib_root);

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
void SYS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


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
void SYS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetModelName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get model name by board ID.
 * INPUT    : unit_id     -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                           means local unit.
 * OUTPUT   : model_name  -- model name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_MODEL_NAME_SIZE + 1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_MGR_GetModelName(UI32_T unit_id, UI8_T *model_name);

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
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
void SYS_MGR_ToggleCpuGuardDebugFlag(void);

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
BOOL_T SYS_MGR_GetCpuGuardDebugFlag(void);

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
BOOL_T SYS_MGR_SetCpuGuardHighWatermark(UI32_T watermark);

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
BOOL_T SYS_MGR_GetCpuGuardHighWatermark(UI32_T *watermark);

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
BOOL_T SYS_MGR_SetCpuGuardLowWatermark(UI32_T watermark);

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
BOOL_T SYS_MGR_GetCpuGuardLowWatermark(UI32_T *watermark);

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
BOOL_T SYS_MGR_SetCpuGuardMaxThreshold(UI32_T threshold);

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
BOOL_T SYS_MGR_GetCpuGuardMaxThreshold(UI32_T *threshold);

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
BOOL_T SYS_MGR_SetCpuGuardMinThreshold(UI32_T threshold);

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
BOOL_T SYS_MGR_GetCpuGuardMinThreshold(UI32_T *threshold);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard status.
 * INPUT    : enable - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_MGR_SetCpuGuardStatus(BOOL_T status);

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
BOOL_T SYS_MGR_GetCpuGuardStatus(BOOL_T *status);

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
BOOL_T SYS_MGR_GetCpuGuardInfo(SYS_MGR_CpuGuardInfo_T *info);

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
SYS_MGR_GetRunningCpuGuardHighWatermark(UI32_T *value);

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
SYS_MGR_GetRunningCpuGuardLowWatermark(UI32_T *value);

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
SYS_MGR_GetRunningCpuGuardMaxThreshold(UI32_T *value);

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
SYS_MGR_GetRunningCpuGuardMinThreshold(UI32_T *value);

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
SYS_MGR_GetRunningCpuGuardStatus(BOOL_T *value);

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
SYS_MGR_GetRunningCpuGuardTrapStatus(BOOL_T *value);
#endif

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
void SYS_MGR_CpuUtilizationMonitoringProcess(void);

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
BOOL_T SYS_MGR_GetCpuUsagePercentage(UI32_T *cpu_util_p, UI32_T *cpu_util_float_p);

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
BOOL_T SYS_MGR_GetCpuUsageMaximum(UI32_T *cpu_util_max_p);

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
BOOL_T SYS_MGR_GetCpuUsageAverage(UI32_T *cpu_util_avg_p);

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
BOOL_T SYS_MGR_GetCpuUsagePeakTime(UI32_T *cpu_util_peak_time);

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
BOOL_T SYS_MGR_GetCpuUsagePeakDuration(UI32_T *cpu_util_peak_duration);

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
BOOL_T SYS_MGR_GetCpuUtilAlarmStatus(BOOL_T *alarm_status_p);

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
BOOL_T SYS_MGR_SetCpuUtilRisingThreshold(UI32_T rising_threshold);

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
BOOL_T SYS_MGR_GetCpuUtilRisingThreshold(UI32_T *rising_threshold_p);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningCpuUtilRisingThreshold(UI32_T *rising_threshold_p);

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
BOOL_T SYS_MGR_SetCpuUtilFallingThreshold(UI32_T falling_threshold);

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
BOOL_T SYS_MGR_GetCpuUtilFallingThreshold(UI32_T *falling_threshold_p);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningCpuUtilFallingThreshold(UI32_T *falling_threshold_p);

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
    BOOL_T get_next);
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
void SYS_MGR_MemoryUtilizationMonitoringProcess(void);

/*------------------------------------------------------------------------------
 * Function : SYS_MGR_GetMemoryUtilizationBrief
 *------------------------------------------------------------------------------
 * Purpose  : This function will get memory utilization
 * INPUT    : none
 * OUTPUT   : sys_mem_brief_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 *-----------------------------------------------------------------------------*/
BOOL_T  SYS_MGR_GetMemoryUtilizationBrief(SYS_MGR_MemoryUtilBrief_T *sys_mem_brief_p);

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
BOOL_T SYS_MGR_GetMemoryUtilTotalMemory(MEM_SIZE_T *total_memory_p);

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
BOOL_T SYS_MGR_GetMemoryUtilFreePercentage(UI32_T *free_percentage_p);

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
BOOL_T SYS_MGR_GetMemoryUtilAlarmStatus(BOOL_T *alarm_status_p);

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
BOOL_T SYS_MGR_SetMemoryUtilRisingThreshold(UI32_T rising_threshold);

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
BOOL_T SYS_MGR_GetMemoryUtilRisingThreshold(UI32_T *rising_threshold_p);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningMemoryUtilRisingThreshold(UI32_T *rising_threshold_p);

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
BOOL_T SYS_MGR_SetMemoryUtilFallingThreshold(UI32_T falling_threshold);

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
BOOL_T SYS_MGR_GetMemoryUtilFallingThreshold(UI32_T *falling_threshold_p);

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
SYS_TYPE_Get_Running_Cfg_T SYS_MGR_GetRunningMemoryUtilFallingThreshold(UI32_T *falling_threshold_p);
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
BOOL_T SYS_MGR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

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
void    SYS_MGR_PowerStatusChanged_CallBack(UI32_T unit, UI32_T power, UI32_T status);
#endif

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
void SYS_MGR_PowerTypeChanged_CallBack(UI32_T unit, UI32_T power, UI32_T type);

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME	- SYS_MGR_AlarmInputStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Alarm input status changed callback function,	register to	sysdrv
 * INPUT   : unit	            -- which unit
 *			 status	            -- SYS_MGR_SYSTEM_ALARM_INPUT_1_MASK
 *					               SYS_MGR_SYSTEM_ALARM_INPUT_2_MASK
 *					               SYS_MGR_SYSTEM_ALARM_INPUT_3_MASK
 *					               SYS_MGR_SYSTEM_ALARM_INPUT_4_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE	   : None
 * -------------------------------------------------------------------------*/
void	SYS_MGR_AlarmInputStatusChanged_CallBack(UI32_T unit, UI32_T status);
#endif

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
void SYS_MGR_MajorAlarmOutputStatusChanged_CallBack(UI32_T unit, UI32_T status);

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
void SYS_MGR_MinorAlarmOutputStatusChanged_CallBack(UI32_T unit, UI32_T status);

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
void SYS_MGR_SetDebugLevel(SYS_MGR_Debug_Func_T func, SYS_MGR_Debug_Level_T level);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetDebugLevel
 *-----------------------------------------------------------------
 * FUNCTION: Get debug level
 * INPUT   : func
 * OUTPUT  : None
 * RETURN  : level
 * NOTE    : None.
 *----------------------------------------------------------------*/
SYS_MGR_Debug_Level_T SYS_MGR_GetDebugLevel(SYS_MGR_Debug_Func_T func);

#endif /* End of SYS_MGR_H */

