/* Module Name: SYSLOG_OM.H
 * Purpose: Initialize the database resources and provide some Get/Set function
 *          for accessing the system log database.
 *
 * Notes:
 *
 * History:
 *    10/29/01       -- Aaron Chuang, Create
 *                   -- Separete parts from syslog_mgr.c, and generate the syslog_om.c
 *
 *   07/18/07        -- Rich Lee, Porting to Linux
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SYSLOG_OM_H
#define SYSLOG_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "syslog_adpt.h"
#include "syslog_type.h"
#include "sysfun.h"
#include "l_inet.h"

/* NAME CONSTANT DECLARATIONS
 */
#define SYSLOG_OM_IP_EQUAL             0x01
#define SYSLOG_OM_IP_LARGE             0x02
#define SYSLOG_OM_IP_LESS              0x04

#define SYSLOG_OM_DEBUG_OM_ERR            0x01L
#define SYSLOG_OM_DEBUG_MGR_ERR           0x02L

/* MACRO FUNCTION DECLARATIONS
 */
#define SYSLOG_OM_EnterCriticalSection(sem_id)    SYSFUN_OM_ENTER_CRITICAL_SECTION(sem_id)
#define SYSLOG_OM_LeaveCriticalSection(sem_id, orig_priority)    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sem_id, orig_priority)


/* DATA TYPE DECLARATIONS
 */

#define SYSLOG_OM_GET_MSGBUFSIZE(field_name)                       \
            (SYSLOG_OM_MSGBUF_TYPE_SIZE +                        \
            sizeof(((SYSLOG_OM_IPCMsg_T*)0)->data.field_name))

#define SYSLOG_OM_MSGBUF_TYPE_SIZE sizeof(union SYSLOG_OM_IpcMsg_Type_U)

/*
#define SYSLOG_OM_MSGBUF_TYPE_SIZE sizeof(union SYSLOG_OM_IpcMsg_Type_U)
#define SYSLOG_OM_GET_MSGBUFSIZE(struct_name) \
        (SYSLOG_OM_MSGBUF_TYPE_SIZE + sizeof(struct struct_name));
  */

typedef struct SYSLOG_OM_Config_S
{
    UI32_T  syslog_status;      /* Enable/Disable */
    UI32_T  uc_log_level;       /* which level need to log to uc */
    UI32_T  flash_log_level;    /* which level nned to log to flash */
} SYSLOG_OM_Config_T;

typedef struct SYSLOG_OM_RecordOwnerInfo_S
{
    UI8_T   level;          /* Reference SYSLOG_LEVEL_E         */
    UI32_T  module_no;      /* Reference SYSLOG_MODULE_E        */
    UI8_T   function_no;    /* Definition by each module itself */
    UI8_T   error_no;       /* Definition by each module itself */
} SYSLOG_OM_RecordOwnerInfo_T;

typedef struct SYSLOG_OM_Record_S
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;
    UI32_T  log_time;       /* seconds
                             * If the device have RTC, the log RTC time is equal with
                             * log_time plus lastest root RTC time.
                             */
    UI32_T  profile_id;
    UI8_T   message[SYSLOG_ADPT_MESSAGE_LENGTH + 1];
} SYSLOG_OM_Record_T;

typedef struct
{
    UI32_T count;
} SYSLOG_OM_ProfileStat_T;

typedef struct SYSLOG_OM_Header_S
{
    UI32_T  front;
    UI32_T  rear;
    UI32_T  count;
    SYSLOG_OM_ProfileStat_T stat[SYSLOG_OM_MAX_NBR_OF_PROFILE];
} SYSLOG_OM_Header_T;

typedef struct SYSLOG_UcNormalDb_S
{
    SYSLOG_OM_Header_T  header;
    SYSLOG_OM_Record_T  entry[SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB];
}  SYSLOG_UcNormalDb_T;

typedef struct SYSLOG_UcFlashDb_S
{
    SYSLOG_OM_Header_T  header;
    SYSLOG_OM_Record_T  entry[SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB];
}  SYSLOG_UcFlashDb_T;

typedef struct SYSLOG_OM_LogfileHeader_S
{
    UI32_T  count;
    UI32_T  sequence_no;

    UI32_T  bytes_of_header;
    UI32_T  bytes_of_record;

    /* Make header have the same size with SYSLOG_OM_Record_T
     */
#define REVERSED_SIZE               \
    (                               \
        sizeof(SYSLOG_OM_Record_T)  \
            - sizeof(UI32_T)        \
            - sizeof(UI32_T)        \
            - sizeof(UI32_T)        \
            - sizeof(UI32_T)        \
    )

    UI8_T   reversed[REVERSED_SIZE];
} SYSLOG_OM_LogfileHeader_T;

typedef struct SYSLOG_LogfileDb_S
{
    SYSLOG_OM_LogfileHeader_T  header;
    SYSLOG_OM_Record_T         entry[1];
} SYSLOG_LogfileDbCommon_T;

typedef struct  SYSLOG_OM_Remote_Server_Config_S
{
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    UI32_T               facility;           /* which type need to log to server */
    UI32_T               level;              /* which level need to log to server */
#endif/*SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER*/
    L_INET_AddrIp_T      ipaddr;             /* server ipaddress */
    char                 name[SYS_ADPT_DEVICE_NAME_STRING_LEN + 1];
    UI32_T               udp_port;           /*udp port*/
    UI8_T                index;
} SYSLOG_OM_Remote_Server_Config_T;

typedef struct
{
    SYSLOG_OM_LogfileHeader_T  header;
    SYSLOG_OM_Record_T  entry[SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE];
}  SYSLOG_LogfileDb_T;

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
typedef struct SYSLOG_Login_Out_LogfileDb_T
{
    SYSLOG_OM_LogfileHeader_T  header;
    SYSLOG_OM_Record_T         entry[SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_LOGIN_OUT_FILE];
} SYSLOG_Login_Out_LogfileDb_T;
#endif /*(SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE) */

/* IPC message structure
 */
typedef struct
{
    union SYSLOG_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_b;
        UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        struct SYSLOG_OM_IPCMSG_OM_Data_S
        {
            UI32_T  syslog_status;      /* Enable/Disable */
            UI32_T  uc_log_level;       /* which level need to log to uc */
            UI32_T  flash_log_level;    /* which level nned to log to flash */
        }syslog_om_data;

        struct  SYSLOG_OM_IPCMSG_Remote_Config_S
        {
            UI32_T  status;             /* Enable/Disable */
            UI32_T  server_index;
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
            UI32_T  facility;           /* which type need to log to server */
            UI32_T  level;              /* which level need to log to server */
#endif
            SYSLOG_OM_Remote_Server_Config_T server_config;
        } syslog_remote_config;

    } data;
} SYSLOG_OM_IPCMsg_T;




enum
{
    SYSLOG_OM_IPC_GET_FACILITY_TYPE,
    SYSLOG_OM_IPC_GET_REMOTE_LOG_LEVEL,
    SYSLOG_OM_IPC_GET_REMOTE_LOG_STATUS,
    SYSLOG_OM_IPC_GET_SERVER_IPADDR,
    SYSLOG_OM_IPC_GET_SYSLOG_STATUS,
    SYSLOG_OM_IPC_GET_UC_LOG_LEVEL
};

#if (SYS_CPNT_REMOTELOG == TRUE)
/*fuzhimin,20090414,end*/

/*    SYSLOG_OM_Remote_Config_T is the structure of remotelog config.
 *    Note:
 *      remotelog_status:
 *            -- REMOTELOG_STATUS_ENABLE  = 0,
 *            -- REMOTELOG_STATUS_DISABLE = 1
 *         facility:
 *           -- REMOTELOG_FACILITY_KERN        = 0,  ( kernel messages )
 *           -- REMOTELOG_FACILITY_USER        = 1,  ( random user-level messages )
 *           -- REMOTELOG_FACILITY_MAIL        = 2,  ( mail system )
 *           -- REMOTELOG_FACILITY_DAEMON      = 3,  ( system daemons )
 *           -- REMOTELOG_FACILITY_AUTH        = 4,  ( security/authorization messages )
 *           -- REMOTELOG_FACILITY_SYSLOG      = 5,  ( messages generated internally by syslogd )
 *           -- REMOTELOG_FACILITY_LPR         = 6,  ( line printer subsystem )
 *           -- REMOTELOG_FACILITY_NEWS        = 7,  ( network news subsystem )
 *           -- REMOTELOG_FACILITY_UUCP        = 8,  ( UUCP subsystem )
 *           -- REMOTELOG_FACILITY_CRON        = 9,  ( clock daemon )
 *           -- REMOTELOG_FACILITY_AUTH1       = 10, ( security/authorization messages )
 *           -- REMOTELOG_FACILITY_FTP         = 11, ( FTP daemon )
 *           -- REMOTELOG_FACILITY_NTP         = 12, ( NTP subsystem )
 *           -- REMOTELOG_FACILITY_AUDIT       = 13, ( log audit )
 *           -- REMOTELOG_FACILITY_ALERT       = 14, ( log alert )
 *           -- REMOTELOG_FACILITY_CRON1       = 15, ( clock daemon )
 *           -- REMOTELOG_FACILITY_LOCAL0      = 16, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL1      = 17, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL2      = 18, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL3      = 19, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL4      = 20, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL5      = 21, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL6      = 22, ( reserved for local use )
 *           -- REMOTELOG_FACILITY_LOCAL7      = 23, ( reserved for local use )
 *        remotelog_level: high or equal this level will belog to remote server(when remotelog_status is enabled).
 *           -- SYSLOG_LEVEL_EMERG  = 0,    ( System unusable                  )
 *           -- SYSLOG_LEVEL_ALERT  = 1,    ( Immediate action needed          )
 *           -- SYSLOG_LEVEL_CRIT   = 2,    ( Critical conditions              )
 *           -- SYSLOG_LEVEL_ERR    = 3,    ( Error conditions                 )
 *           -- SYSLOG_LEVEL_WARNING= 4,    ( Warning conditions               )
 *           -- SYSLOG_LEVEL_NOTICE = 5,    ( Normal but significant condition )
 *           -- SYSLOG_LEVEL_INFO   = 6,    ( Informational messages only      )
 *           -- SYSLOG_LEVEL_DEBUG  = 7     ( Debugging messages               )
 *        server_ipaddr: remote server ip address.
 *
 */
typedef struct  SYSLOG_OM_Remote_Config_S
{
    UI32_T  status;             /* Enable/Disable */
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
    UI32_T  facility;           /* which type need to log to server */
    UI32_T  level;              /* which level need to log to server */
    char    facility_str[SYSLOG_TYPE_MAX_FACILITY_STR_LEN + 1];
    char    level_str[SYSLOG_TYPE_MAX_LEVEL_STR_LEN + 1];
#endif

    SYSLOG_OM_Remote_Server_Config_T server_config[SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER];
} SYSLOG_OM_Remote_Config_T;

/*  SYSLOG_OM_Remote_Record_T is structure of the remotelog entry.
 *  Note:
 *      owner_info: owner information of this entry.
 *      log_time:   log time of this entry.
 *      message:    message information(0..1024)
 */
typedef struct  SYSLOG_OM_Remote_Record_S
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;
    UI32_T  log_time;       /* seconds
                             * If the device have RTC, the log RTC time is equal with
                             * log_time plus lastest root RTC time.
                             */
    UI8_T   message[SYSLOG_ADPT_MESSAGE_LENGTH + 1];
}  SYSLOG_OM_Remote_Record_T;

typedef struct    SYSLOG_EVENT_SyslogData_S
{
    struct SYSLOG_EVENT_SyslogData_S  *next;
    SYSLOG_OM_Remote_Record_T     remotelog_entry;
} SYSLOG_EVENT_SyslogData_T;

#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  SYSLOG_OM_SetDebugFlag
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void
SYSLOG_OM_SetDebugFlag(
    UI32_T debug_flag
);

/* FUNCTION NAME:  SYSLOG_OM_GetDebugFlag
 * PURPOSE  : get backdoor debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug flag
 * NOTES    : none
 */
UI32_T
SYSLOG_OM_GetDebugFlag(
);

/* FUNCTION NAME: SYSLOG_OM_Init
 * PURPOSE: This function is used to initialize the system log database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the system log database.
 *
 */
void SYSLOG_OM_Init(void);

/* FUNCTION NAME: SYSLOG_OM_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the system log database.
 *
 */
BOOL_T SYSLOG_OM_Initiate_System_Resources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYSLOG OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYSLOG_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME: SYSLOG_OM_Initiate_UcDatabase
 * PURPOSE: This function is used to initialize the UC database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the UC normal database.
 *
 */
BOOL_T SYSLOG_OM_Initiate_UcDatabase(void);

/* FUNCTION NAME: SYSLOG_OM_Clear_UcDatabaseOldVersionRecord
 * PURPOSE: This function is used to check code version.
 *          If the code version is old version, clear UC detabase.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 * NOTES:   1. Only need to check version when switch warmstart.
 *          2. Switch will clear UC detabase when update code from
 *             old version to new version.
 *
 */
BOOL_T
SYSLOG_OM_Clear_UcDatabaseOldVersionRecord(
    void
);

/* FUNCTION NAME: SYSLOG_OM_GetSyslogStatus
 * PURPOSE: This function is used to get the system log status.
 * INPUT:   *syslog_status -- output buffer of system log status value.
 * OUTPUT:  *syslog_status -- system log status value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetSyslogStatus(UI32_T *syslog_status);


/* FUNCTION NAME: SYSLOG_OM_SetSyslogStatus
 * PURPOSE: This function is used to set the system log status.
 * INPUT:   syslog_status -- setting value of system log status.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_SetSyslogStatus(UI32_T syslog_status);


/* FUNCTION NAME: SYSLOG_OM_GetUcLogLevel
 * PURPOSE: This function is used to get the un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcLogLevel(UI32_T *uc_log_level);


/* FUNCTION NAME: SYSLOG_OM_SetUcLogLevel
 * PURPOSE: This function is used to set the un-cleared memory log level.
 * INPUT:   uc_log_level -- setting value of un-cleared memory log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_MGR_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_MGR_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_MGR_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_MGR_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:
 *
 */
UI32_T SYSLOG_OM_SetUcLogLevel(UI32_T uc_log_level);


/* FUNCTION NAME: SYSLOG_OM_GetFlashLogLevel
 * PURPOSE: This function is used to get the flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetFlashLogLevel(UI32_T *flash_log_level);


/* FUNCTION NAME: SYSLOG_OM_SetFlashLogLevel
 * PURPOSE: This function is used to set the flash log level.
 * INPUT:   flash_log_level -- setting value of flash log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_MGR_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_MGR_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_MGR_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_MGR_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:
 *
 */
UI32_T SYSLOG_OM_SetFlashLogLevel(UI32_T flash_log_level);


/* FUNCTION NAME: SYSLOG_OM_GetSyslogConfig
 * PURPOSE: This function is used to get the system log config.
 * INPUT:   *config -- output buffer of system log config structure.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetSyslogConfig(SYSLOG_OM_Config_T *config);


/* FUNCTION NAME: SYSLOG_OM_AddUcNormalEntry
 * PURPOSE: Add a log message to un-cleared memory normal log DB.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_AddUcNormalEntry(SYSLOG_OM_Record_T *syslog_entry);


/* FUNCTION NAME: SYSLOG_OM_AddUcFlashEntry
 * PURPOSE: Add a log message to un-cleared memory flash log DB.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_AddUcFlashEntry(SYSLOG_OM_Record_T *syslog_entry);

/* FUNCTION NAME: SYSLOG_OM_ClearUcRamEntries
 * PURPOSE: Clear log message from system log module in UC Ram memory.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_ClearUcRamEntries(
    void
);

/* FUNCTION NAME: SYSLOG_OM_ClearUcFlashEntries
 * PURPOSE: Clear log message from system log module in UC Flash memory.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_ClearUcFlashEntries(
    void
);

/* FUNCTION NAME: SYSLOG_OM_GetUcNormalHeader
 * PURPOSE: Get header from un-cleared memory normal log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcNormalHeader(SYSLOG_OM_Header_T *header);


/* FUNCTION NAME: SYSLOG_OM_GetUcFlashHeader
 * PURPOSE: Get header from un-cleared memory flash log DB.
 * INPUT:   *header -- output buffer of un-cleared flash db header.
 * OUTPUT:  *header -- un-cleared flash db header.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcFlashHeader(SYSLOG_OM_Header_T *header);


/* FUNCTION NAME: SYSLOG_OM_SetUcFlashHeader
 * PURPOSE: Set header from un-cleared memory flash log DB.
 * INPUT:   header  -- un-cleared flash db header.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_SetUcFlashHeader(SYSLOG_OM_Header_T *header);


/* FUNCTION NAME: SYSLOG_OM_GetUcNormalEntry
 * PURPOSE: Get a log message from un-cleared memory normal log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcNormalEntry(UI32_T syslog_index, SYSLOG_OM_Record_T *syslog_entry);


/* FUNCTION NAME: SYSLOG_OM_GetUcFlashEntry
 * PURPOSE: Get a log message from un-cleared memory flash log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcFlashEntry(UI32_T syslog_index, SYSLOG_OM_Record_T *syslog_entry);


/* FUNCTION NAME: SYSLOG_OM_CheckUcFlashFull
 * PURPOSE: Check un-cleared memory flash log DB is full or not.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- Full.
 *          FALSE   -- Not full.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_CheckUcFlashFull(void);

#if (SYS_CPNT_REMOTELOG == TRUE)
/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerIPAddr
 * PURPOSE: This function is used to set the log mode.
 * INPUT:   index -- index of server ip address.
 *          *ipaddr -- buffer of server ip address.
 * OUTPUT:  *ipaddr -- value of server ip address.
 * RETUEN:  REMOTELOG_SUCCESS    --  OK, Successful, Without any Error
 *          REMOTELOG_FAIL   --  fail
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerIPAddr(
    UI8_T index,
    L_INET_AddrIp_T *ipaddr
);

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogStatus
 * PURPOSE: This function is used to enable/disable remotelog status.
 * INPUT:   remotelog_status -- status of remotelog.
 * RETUEN:  REMOTELOG_SUCCESS    --  OK, Successful, Without any Error
 *          REMOTELOG_INVALID   --  input value is invalid
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogStatus(
    UI32_T remotelog_status
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogStatus
 * PURPOSE: This function is used to get remotelog status.
 * INPUT:   *remotelog_status -- output buffer of remotelog status
 * OUTPUT:  *remotelog_status -- value of remotelog status.
 * RETUEN:  REMOTELOG_SUCCESS    --  OK, Successful, Without any Error
 *          REMOTELOG_INVALID_BUFFER   --  output buffer is invalid
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogStatus(
    UI32_T *remotelog_status
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogConfiguration

 * PURPOSE: This function is used to get the remote log config.
 * INPUT:   *config -- output buffer of remote log config structure.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_GetRemoteLogConfiguration(
    SYSLOG_OM_Remote_Config_T *config
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogMessage

 * PURPOSE: This function is used to get the remote log message.
 * INPUT:   *config -- output buffer of remote log message.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   For backdoor use only
 *
 */
BOOL_T
SYSLOG_OM_GetRemoteLogMessage(
    UI8_T *msg
);

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogMessage

 * PURPOSE: This function is used to set the remote log message.
 * INPUT:   msg -- remote log message.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Called by mgr only for save remotelog message string
 *
 */
BOOL_T
SYSLOG_OM_SetRemoteLogMessage(
    UI8_T *msg
);


/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerPortByIndex
 * PURPOSE: This function is used to get udp port.
 * INPUT:   *udp_port -- udp port.
 * OUTPUT:  *udp_port -- udp port.
 * RETUEN:  REMOTELOG_SUCCESS    --  OK, Successful, Without any Error
 *          REMOTELOG_FAIL   --  fail
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerPortByIndex(
    UI32_T index,
    UI32_T *udp_port
);

/* FUNCTION NAME: SYSLOG_OM_QueueEnqueue
 * PURPOSE: This function is used to enqueue remotelog entry.
 * INPUT:   new_blk -- remotelog event data pointer.
 * OUTPUT:  None.
 * RETUEN:  Error code.
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_QueueEnqueue(
    SYSLOG_EVENT_SyslogData_T *new_blk);

/* FUNCTION NAME: SYSLOG_OM_QueueDequeue
 * PURPOSE: This function is used to dequeue remotelog entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  remotelog event data
 * NOTES:   None.
 *
 */
SYSLOG_EVENT_SyslogData_T *
SYSLOG_OM_QueueDequeue();

/* FUNCTION NAME: SYSLOG_MGR_IsQueueEmpty
 * PURPOSE: This function is used to check if queue is empty.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  TRUE -- Queue is empty
 *          FALSE -- Queue is not empty
 * NOTES:   None.
 *
 */
BOOL_T
SYSLOG_OM_IsQueueEmpty();

/*fuzhimin,20090414,end*/

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerByIndex
 * PURPOSE: This function is used to set the log mode.
 * INPUT:   *ipaddr -- buffer of server ip address.
 *          index -- index of server ip address
 * OUTPUT:  *ipaddr -- value of server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerByIndex(
    SYSLOG_OM_Remote_Server_Config_T *server_config,
    UI32_T index
);

/* FUNCTION NAME: SYSLOG_OM_GetNextRemoteLogServer
 * PURPOSE: This function is used to get the server_config data
 * INPUT:   *ipaddr   -- the index of the server_config
 *          *facility -- facility
 *          *level    -- level
 *          *udp_port -- udp_port
 * OUTPUT:  *facility -- facility
 *          *level    -- level
 *          *udp_port -- udp_port
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetNextRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServer
 * PURPOSE: This function is used to get the server_config data
 * INPUT:   *ipaddr   -- the index of the server_config
 *          *facility -- facility
 *          *level    -- level
 *          *udp_port -- udp_port
 * OUTPUT:  *facility -- facility
 *          *level    -- level
 *          *udp_port -- udp_port
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config,
    UI32_T *index
);

/* FUNCTION NAME: SYSLOG_OM_CreateRemoteLogServer
 * PURPOSE: This function is used to set the log mode.
 * INPUT:   ipaddr -- value of server ip.
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_CreateRemoteLogServer(
    L_INET_AddrIp_T *ipaddr
);

/* FUNCTION NAME: SYSLOG_OM_DeleteRemoteLogServer
 * PURPOSE: This function is used to delete server ip address.
 * INPUT:   ipaddr -- deleted server ip.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          REMOTELOG_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_DeleteRemoteLogServer(
    L_INET_AddrIp_T *ipaddr
);

/* FUNCTION NAME: SetRemoteLogServerPort
 * PURPOSE: This function is used to get the remote log server port of the input
 *          ip address.
 * INPUT:   *ipaddr -- value of server ip.
 *          *port_p -- value of udp_port
 * OUTPUT:  *port_p -- value of udp_port
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerPort(
    L_INET_AddrIp_T *ipaddr,
    UI32_T port
);

/* FUNCTION NAME: GetRemoteLogServerPort
 * PURPOSE: This function is used to get the remote log server port of the input
 *          ip address.
 * INPUT:   *ipaddr -- value of server ip.
 *          *port_p -- value of udp_port
 * OUTPUT:  *port_p -- value of udp_port
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerPort(
    L_INET_AddrIp_T *ipaddr,
    UI32_T * port_p
);

/* FUNCTION NAME: SYSLOG_OM_DeleteAllRemoteLogServer
 * PURPOSE: This function is used to delete all server ip address.
 * INPUT:
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          REMOTELOG_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_DeleteAllRemoteLogServer(
);

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogServerFacility
 * PURPOSE: This function is used to set the remote log server facility and ip address.
 * INPUT:   ipaddr -- value of server ip.
 *              facility -- value of facility
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerFacility(
    L_INET_AddrIp_T *ipaddr,
    UI32_T facility
);

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogServerLevel
 * PURPOSE: This function is used to set the remote log server facility and ip address.
 * INPUT:   ipaddr -- value of server ip.
 *          facility -- value of facility
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS     --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL        --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerLevel(
    L_INET_AddrIp_T *ipaddr,
    UI32_T level
);

#else
/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogFacility
 * PURPOSE: This function is used to set the remote log server facility and ip address.
 * INPUT:   facility -- value of facility
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogFacility(
    UI32_T facility
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogFacility
 * PURPOSE: This function is used to get the log facility.
 * INPUT:   *facility_p -- output buffer of remotelog facility.
 * OUTPUT:  *facility_p -- value of remotelog facility.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  remotelog facility is invalid
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogFacility(
    UI32_T *facility_p
);

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogLevel
 * PURPOSE: This function is used to set remotelog level.
 * INPUT:   level -- value of remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS   --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  input value is invalid
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogLevel(
    UI32_T level
);

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogLevel
 * PURPOSE: This function is used to get remotelog level.
 * INPUT:   *remotelog -- output buffer of remotelog level.
 * OUTPUT:  *remotelog -- value of remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  input value is invalid
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_GetRemoteLogLevel(
    UI32_T *level_p
);

#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif /*endif (SYS_CPNT_REMOTELOG == TRUE)*/

/* FUNCTION NAME: SYSLOG_OM_GetUcLogPointers
 * PURPOSE: This function is used to save logs on RAM to Flash before reload.
 * INPUT:   None.
 * OUTPUT:  ret_sys_log_ram_db_pp       -- Sys log on RAM.
 *          ret_sys_log_flash_db_pp     -- Sys log on RAM buffer.
 * RETUEN:  TRUE/FALSE.
 * NOTES:   The returned pointer should be used only to save or restore
 *          into/from Flash only.
 */
BOOL_T SYSLOG_OM_GetUcLogPointers(SYSLOG_UcNormalDb_T **ret_sys_log_ram_db_pp,
    SYSLOG_UcFlashDb_T **ret_sys_log_flash_db_pp);

/* MACRO FUNCTION DECLARATIONS
 */

#endif /* SYSLOG_OM_H */
