/* Module Name: SYSLOG_MGR.C
 * Purpose: Initialize the resources and provide som function
 *          for the system log module.
 *
 * Notes:
 *
 * History:
 *    10/17/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "sysfun.h"
#include "l_inet.h"
#include "syslog_type.h"
#include "syslog_adpt.h"
#include "syslog_mgr.h"
#include "syslog_om.h"
#include "fs_type.h"
#include "fs.h"
#include "uc_mgr.h"
#include "sys_cpnt.h"
#include "backdoor_mgr.h"
#include  "syslog_backdoor.h"
#if (SYS_CPNT_REMOTELOG == TRUE)
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* ES3550MO-PoE-FLF-AA-00053
 * use CmnLib APIs to replace socket ntohs(), htons()... function
 */
#include "l_stdlib.h"
//rich#include "socket.h"
#include "mib2_mgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#include "netcfg_type.h"
#endif
#ifdef FTTH_OKI    /* FTTH_OKI */
#include "leaf_es3626a.h"
#endif
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
#include "eh_type.h"
#include "eh_mgr.h"
#endif
#include "sys_module.h"

#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_mgr.h"
#endif

#if (SYS_CPNT_DAI == TRUE)
#include "dai_type.h"
#endif

#include "stktplg_mgr.h"
#include "stktplg_pmgr.h"
#include "stktplg_om.h"
#include "stktplg_pom.h"
#include "l_mm.h"
#include "ip_lib.h"
#include "ipal_types.h"
#include "ipal_route.h"

#include "unistd.h" /*maggie liu remove warning*/
#include "trap_event.h"
#if (SYS_CPNT_POE == TRUE)
#include "leaf_3621.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSLOG_FILE_1   ".logfile_1"
#define SYSLOG_FILE_2   ".logfile_2"

#define SYSLOG_LOGIN_OUT_FILE_1   ".login_out_logfile_1"
#define SYSLOG_LOGIN_OUT_FILE_2   ".login_out_logfile_2"

#define SYSLOG_MGR_TEMP_MESSAGE_LENGTH 128

/* DATA TYPE DECLARATIONS
 */
#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif /* _countof */

#ifndef ASSERT
#define ASSERT(eq)
#endif

#define SYSLOG_MGR_IS_DEBUG_ERROR_ON(flag)            (flag & SYSLOG_OM_DEBUG_MGR_ERR)

#define SYSLOG_MGR_DEBUG_LOG(fmt, ...)                          \
    {                                                           \
        UI32_T debug_flag = SYSLOG_OM_GetDebugFlag();           \
        if (SYSLOG_MGR_IS_DEBUG_ERROR_ON(debug_flag))           \
        {                                                       \
            printf("%s:%d ", __FUNCTION__, __LINE__);           \
            printf(fmt,__VA_ARGS__);                            \
            printf("\r\n");                                     \
        }                                                       \
    }

typedef struct
{
    SYSLOG_MGR_PrepareCommonPtr_T prepare_db_p;

    UI32_T  max_entries_of_prepare_db;
    UI32_T  max_entries_of_logfile;

    UI32_T  last_logfile;
    UI32_T  next_logfile;
    UI32_T  last_sequence_no;
    UI32_T  logfile_total_count;

    UI32_T num_of_logfile;
    char *logfile_ar[SYSLOG_MGR_MAX_NUM_OF_LOGFILE];

} SYSLOG_MGR_Profile_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T
SYSLOG_MGR_ExistLogfileInFileSystem(
    void
);

static BOOL_T
SYSLOG_MGR_CreateLogfile(
);

static BOOL_T
SYSLOG_MGR_GetLastLogfileInfo(
    SYSLOG_MGR_Profile_T *profile_p
);

static BOOL_T   SYSLOG_MGR_RegisterCallbackFunction(void);

static BOOL_T
SYSLOG_MGR_AddFormatMsgEntry_local(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2
);

/* FUNCTION NAME: SYSLOG_MGR_FormatMessage
 * PURPOSE: Format a message string.
 * INPUT:   message_index   -- The message identifier for the requested message.
 *          size            -- The size of the output buffer in chars.
 * OUTPUT:  message         -- A pointer to a buffer that receives the
 *                             null-terminated string that specifies the
 *                             formatted message.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the size of the formated message string is larger than output
 *          buffer, the formated message string will be cut out.
 */
static BOOL_T
SYSLOG_MGR_FormatMessage(
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2,
    char    *message,
    UI32_T  size
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SYSLOG_MGR_GetUserInfoString
 *------------------------------------------------------------------------
 * FUNCTION: Get user information string
 * INPUT   : user_info_p      -- user information entry
 *           cb_str           -- count of bytes of str
 * OUTPUT  : str_p            -- display string
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
SYSLOG_MGR_GetUserInfoString(
    SYSLOG_MGR_UserInfo_T  *user_info_p,
    char                   *str_p,
    UI32_T                 cb_str
);

static BOOL_T SYSLOG_MGR_AddEntry_local(SYSLOG_OM_Record_T *syslog_entry, BOOL_T flush_immediately);

#if (SYS_CPNT_REMOTELOG == TRUE)
static UI32_T
SYSLOG_MGR_AddFormatMsgEntry_Remote(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2
);
static UI32_T SYSLOG_MGR_AddEntry_Remote(SYSLOG_OM_Remote_Record_T *log_entry);
static UI32_T SYSLOG_MGR_IsValidIPAddress(L_INET_AddrIp_T *ip_addr);

static void SYSLOG_MGR_QueueEnqueue(SYSLOG_EVENT_SyslogData_T *p);

static SYSLOG_EVENT_SyslogData_T  *SYSLOG_MGR_QueueDequeue();

static int
SYSLOG_MGR_BuildLogMsg(
    SYSLOG_OM_Remote_Record_T *remotelog_entry,
    SYSLOG_OM_Remote_Server_Config_T server_config,
    char *buffer);

static void
SYSLOG_MGR_SendLogPacket(
    SYSLOG_OM_Remote_Record_T *remotelog_entry,
    char *buffer);
/*fuzhimin,20090417,end*/
#endif

#if (SYS_CPNT_SMTP == TRUE)
static BOOL_T
SYSLOG_MGR_AddFormatMsgEntry_Smtp(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2
);
static BOOL_T SYSLOG_MGR_AddEntry_Smtp(SYSLOG_OM_Record_T *log_entry);
#endif

static BOOL_T __SYSLOG_MGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry, BOOL_T flush_immediately);

/* daniel */
extern UI32_T NETCFG_POM_IP_GetNextRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

static void SYSLOG_MGR_RestoreUcLogsFromFlash();
static void SYSLOG_MGR_DeleteUcLogsFromFlash();

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC
static UI32_T                   syslog_mgr_sem_id;   /* semaphore id */
static SYSLOG_MGR_Profile_T     syslog_mgr_profile[SYSLOG_OM_MAX_NBR_OF_PROFILE];

static SYSLOG_MGR_Prepare_T     prepare_db;

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
static SYSLOG_MGR_PrepareLoginOut_T prepare_login_out_db;
#endif /* SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT */

static SYSLOG_MGR_UcDatabase_T  uc_database;
static UI32_T                   log_up_time;
#if (SYS_CPNT_REMOTELOG == TRUE)
static UI32_T  syslog_task_id;
static UI32_T                   spanning_tree_state;

const static char *MONTH_MAPPING_STR[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
    };

static char *SYSLOG_MGR_MODULE_STRING_ARRAY[] = {SYSMOD_LIST(MODULE_NAME)};
#endif

/* MACRO FUNCTIONS DECLARACTION
 */

#define SYSLOG_MGR_LOCK()       SYSFUN_GetSem(syslog_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYSLOG_MGR_UNLOCK()     SYSFUN_SetSem(syslog_mgr_sem_id)

/*
#define sockcall    _nu_sockcall  //daniel

struct sockcall {
    int pna;
    int (*close)();
    int (*socket)();
    int (*bind)();
    int (*select)();
    int (*recvfrom)();
    int (*sendto)();
    int (*ioctl)();
    int (*shutdown)();
    int (*connect)();
    int (*getsockname)();
    int (*setsockopt)();
    int (*send)();
    int (*recv)();
    int (*listen)();
    int (*accept)();
    int (*getpeername)();
    int (*shr_socket)();
    int (*get_id)();
    int (*set_id)();
};  //daniel

extern struct sockcall *sockcall; //daniel


#define close           (*sockcall->close) //daniel
*/

#define s_close(fd) close(fd)

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: SYSLOG_MGR_GetOperationMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
SYS_TYPE_Stacking_Mode_T SYSLOG_MGR_GetOperationMode(void)
{
    return ___csc_mgr_operating_mode;
}


/* FUNCTION NAME: SYSLOG_MGR_EnterMasterMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterMasterMode(void)
{
    UI32_T boot_reason;
    UI32_T my_unit_id;

    /* check if UC database need to initial */
    if(!STKTPLG_POM_GetMyUnitID(&my_unit_id))
    {
        printf("SYSLOG_OM_Initiate_UcDatabase:STKTPLG_MGR_GetMyUnitID failure\n");
        return FALSE;
    }
    if(!STKTPLG_POM_GetUnitBootReason(my_unit_id, &boot_reason))
    {
        printf("SYSLOG_OM_Initiate_UcDatabase:STKTPLG_MGR_GetUnitBootReason failure\n");
        return FALSE;
    }

    if(boot_reason == STKTPLG_OM_REBOOT_FROM_COLDSTART)
    {
        SYSLOG_OM_Initiate_UcDatabase();
    }
    else
    {
        SYSLOG_MGR_RestoreUcLogsFromFlash();

        SYSLOG_OM_Clear_UcDatabaseOldVersionRecord();
    }

    SYSLOG_MGR_DeleteUcLogsFromFlash();

    SYSFUN_ENTER_MASTER_MODE();
    return TRUE;
}


/* FUNCTION NAME: SYSLOG_MGR_SetTransitionMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return TRUE;
}

/* FUNCTION NAME: SYSLOG_MGR_EntertTransitionMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    SYSLOG_OM_Init();
    return TRUE;
}

/* FUNCTION NAME: SYSLOG_MGR_EntertSlaveMode()
 * PURPOSE:
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return TRUE;
}

/* FUNCTION NAME: SYSLOG_MGR_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. Create semphore for syslog module using.
 *          2. Check syslog file exist or not in file system,
 *             If EXIST, nothing, otherwise create it.
 *          3. Set log_up_time = 0.
 *          4. Initialize OM database of the system log module.
 *          5. Call by SYSLOG_TASK_Initiate_System_Resources() only.
 *
 */
BOOL_T SYSLOG_MGR_Initiate_System_Resources(void)
{
    /* Init profile
     */
    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].prepare_db_p = (SYSLOG_MGR_PrepareCommonPtr_T)&prepare_db;
    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].max_entries_of_prepare_db = SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_DB;

    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].max_entries_of_logfile = SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_FILE;

    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].num_of_logfile = SYSLOG_MGR_MAX_NUM_OF_LOGFILE;
    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].logfile_ar[0] = SYSLOG_FILE_1;

#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)
    syslog_mgr_profile[SYSLOG_OM_DEFAULT_PROFILE].logfile_ar[1] = SYSLOG_FILE_2;
#endif /* SYS_CPNT_SYSLOG_BACKUP */

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].prepare_db_p = (SYSLOG_MGR_PrepareCommonPtr_T)&prepare_login_out_db;
    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].max_entries_of_prepare_db = SYSLOG_ADPT_MAX_CAPABILITY_OF_PREPARE_LOGIN_OUT_DB;

    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].max_entries_of_logfile = SYSLOG_ADPT_MAX_ENTRIES_IN_SYSLOG_LOGIN_OUT_FILE;

    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].num_of_logfile = SYSLOG_MGR_MAX_NUM_OF_LOGFILE;
    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].logfile_ar[0] = SYSLOG_LOGIN_OUT_FILE_1;

#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)
    syslog_mgr_profile[SYSLOG_OM_LOGIN_OUT_PROFILE].logfile_ar[1] = SYSLOG_LOGIN_OUT_FILE_2;
#endif /* SYS_CPNT_SYSLOG_BACKUP */

#endif /* SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT */

    /* create semaphore
     */
    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &syslog_mgr_sem_id) !=SYSFUN_OK)
    {
        perror("\r\nCreate syslog_mgr_sem_id failed.");
        return(FALSE);
    }

    /* Register callback function (move from task to mgr)
     */
    SYSLOG_MGR_RegisterCallbackFunction();

    if (!SYSLOG_MGR_ExistLogfileInFileSystem())
    {
        if (TRUE != SYSLOG_MGR_CreateLogfile())
        {
            printf("syslog: init: Failed to create logfile.\r\n");
            return FALSE;
        }
    }

    log_up_time = 0;

    if (!SYSLOG_OM_Initiate_System_Resources())
    {

        while (TRUE);
    }



#if (SYS_CPNT_REMOTELOG == TRUE)
    syslog_task_id = 0;

    spanning_tree_state = SYSLOG_ADPT_STA_UNSTABLED_STATE;

#if 0//rich
    NETCFG_MGR_RegisterRifUp_CallBack(SYSLOG_MGR_RifUp_CallBack);
    NETCFG_MGR_RegisterRifDown_CallBack(SYSLOG_MGR_RifDown_CallBack);
#endif
#endif

    return(TRUE);
} /* SYSLOG_MGR_Initiate_System_Resources */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYSLOG_MGR_Create_InterCSC_Relation(void)
{
    /* Register callback function (move from task to mgr) */
    SYSLOG_MGR_RegisterCallbackFunction();

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("syslog",
                                                      SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY,
                                                      SYSLOG_BACKDOOR_Main);
/*
    BACKDOOR_PMGR_Register_SubsysBackdoorFunc_CallBack("syslog",
        SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, SYSLOG_BACKDOOR_Main);
*/
/* rich , for linux use sys_callback and this event is handle by
 *  IPCFG_SignalAllProtocolRifDown()
 *  IPCFG_SignalAllProtocolRifActive()
 */
#if 0
#if (SYS_CPNT_REMOTELOG == TRUE)
    NETCFG_MGR_RegisterRifUp_CallBack(SYSLOG_MGR_RifUp_CallBack);
    NETCFG_MGR_RegisterRifDown_CallBack(SYSLOG_MGR_RifDown_CallBack);
#endif
#endif
} /* end of SYSLOG_MGR_Create_InterCSC_Relation */

/* FUNCTION NAME: SYSLOG_MGR_LoadDefaultOM
 * PURPOSE: This function is used to load the default OM value
 *          to the system log module when re-stacking.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. Initialize OM database of the system log module.
 *          2. Set log_up_time = 0.
 *          3. Call by SYSLOG_TAK_EnterTransitionMode() only.
 */
BOOL_T SYSLOG_MGR_LoadDefaultOM(void)
{

    SYSLOG_OM_Init();

    log_up_time = 0;

    return(TRUE);
}



/* FUNCTION NAME: SYSLOG_MGR_GetSyslogStatus
 * PURPOSE: This function is used to get the system log status.
 * INPUT:   *syslog_status -- output buffer of system log status value.
 * OUTPUT:  *syslog_status -- system log status value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. syslog status symbol is defined as following:
 *          -- SYSLOG_STATUS_ENABLE  = 0,
 *          -- SYSLOG_STATUS_DISABLE = 1,
 *
 */
BOOL_T SYSLOG_MGR_GetSyslogStatus(UI32_T *syslog_status)
{
    BOOL_T  ret;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    ret = SYSLOG_OM_GetSyslogStatus(syslog_status);



    return ret;
} /* End of SYSLOG_MGR_GetSyslogStatus */


/* FUNCTION NAME: SYSLOG_MGR_SetSyslogStatus
 * PURPOSE: This function is used to set the system log status.
 * INPUT:   syslog_status -- setting value of system log status.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_SetSyslogStatus(UI32_T syslog_status)
{
    UI32_T  om_status;
    BOOL_T  ret;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */
    if (syslog_status < SYSLOG_STATUS_ENABLE || syslog_status > SYSLOG_STATUS_DISABLE)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "Syslog status: %1u", syslog_status);
        EH_PMGR_Handle_Exception1(SYS_MODULE_SYSLOG, SYSLOG_MGR_SET_SYSLOG_STATUS_FUNC_NO,
                EH_TYPE_MSG_IS_INVALID, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1);
#endif

        return FALSE;
    }

    SYSLOG_OM_GetSyslogStatus(&om_status);

    if (syslog_status != om_status)
    {
        ret = SYSLOG_OM_SetSyslogStatus(syslog_status);


        return ret;
    }


    return TRUE;
} /* End of SYSLOG_MGR_GetSyslogStatus */


/* FUNCTION NAME: SYSLOG_MGR_GetUcLogLevel
 * PURPOSE: This function is used to get the un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 */
BOOL_T SYSLOG_MGR_GetUcLogLevel(UI32_T *uc_log_level)
{
    BOOL_T  ret;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */
    if (uc_log_level == NULL)
    {

#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_PMGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_GET_UC_LOG_LEVEL_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return FALSE;
    }

    ret = SYSLOG_OM_GetUcLogLevel(uc_log_level);



    return ret;
} /* End of SYSLOG_MGR_GetUcLogLevel */


/* FUNCTION NAME: SYSLOG_MGR_SetUcLogLevel
 * PURPOSE: This function is used to set the un-cleared memory log level.
 * INPUT:   uc_log_level -- setting value of un-cleared memory log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 *
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *          3. un-cleared memory log level must be lower priority than
 *             un-cleared memory flash level.
 *
 */
UI32_T SYSLOG_MGR_SetUcLogLevel(UI32_T uc_log_level)
{
    UI32_T  ret = SYSLOG_RETURN_OK;
    UI32_T  om_level;
    UI32_T  flash_log_level;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYSLOG_RETURN_OK;
    }

    /* not in slave mode */
    if (uc_log_level < SYSLOG_LEVEL_EMERG || uc_log_level > SYSLOG_LEVEL_DEBUG)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "RAM log level (0-7):");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG, SYSLOG_MGR_SET_UC_LOG_LEVEL_FUNC_NO,
                EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (SYSLOG_LEVEL_INFO),
                buff1);
#endif

        return SYSLOG_LEVEL_VLAUE_INVALID;
    }
    SYSLOG_OM_GetFlashLogLevel(&flash_log_level);
    if (uc_log_level < flash_log_level)
    {

#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_SET_UC_LOG_LEVEL_FUNC_NO,
                EH_TYPE_MSG_LOG_LEVEL_RAM_LOWER, (SYSLOG_LEVEL_INFO));
#endif
        return SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL;
    }
    SYSLOG_OM_GetUcLogLevel(&om_level);
    if (uc_log_level != om_level)
    {

        ret = SYSLOG_OM_SetUcLogLevel(uc_log_level);

    }

    return ret;
} /* End of SYSLOG_MGR_SetUcLogLevel */


/* FUNCTION NAME: SYSLOG_MGR_GetFlashLogLevel
 * PURPOSE: This function is used to get the flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
BOOL_T SYSLOG_MGR_GetFlashLogLevel(UI32_T *flash_log_level)
{
    BOOL_T  ret;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    ret = SYSLOG_OM_GetFlashLogLevel(flash_log_level);



    return ret;
} /* End of SYSLOG_MGR_GetFlashLogLevel */


/* FUNCTION NAME: SYSLOG_MGR_SetFlashLogLevel
 * PURPOSE: This function is used to set the flash log level.
 * INPUT:   flash_log_level -- setting value of flash log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:   1. un-cleared memory log level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *          3. un-cleared memory flash level must be higher priority than
 *             un-cleared memory log level.
 *
 */
UI32_T SYSLOG_MGR_SetFlashLogLevel(UI32_T flash_log_level)
{
    UI32_T  ret = SYSLOG_RETURN_OK;
    UI32_T  om_level;
    UI32_T  uc_log_level;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYSLOG_RETURN_OK;
    }

    /* not in slave mode */
    if (flash_log_level < SYSLOG_LEVEL_EMERG || flash_log_level > SYSLOG_LEVEL_DEBUG)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "Flash log level (0-7):");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG, SYSLOG_MGR_SET_FLASH_LOG_LEVEL_FUNC_NO,
                EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_INFO),
                buff1);
#endif

        return SYSLOG_LEVEL_VLAUE_INVALID;
    }
    SYSLOG_OM_GetUcLogLevel(&uc_log_level);
    if (flash_log_level > uc_log_level)
    {

#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_SET_FLASH_LOG_LEVEL_FUNC_NO,
                EH_TYPE_MSG_LOG_LEVEL_FLASH_HIGER, (SYSLOG_LEVEL_INFO));
#endif
        return SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL;
    }
    SYSLOG_OM_GetFlashLogLevel(&om_level);
    if (flash_log_level != om_level)
    {

        ret = SYSLOG_OM_SetFlashLogLevel(flash_log_level);

    }

    return ret;
} /* End of SYSLOG_MGR_SetFlashLogLevel */


/* FUNCTION NAME: SYSLOG_MGR_AddFormatMsgEntry
 * PURPOSE: Add a log message to system log module using format message.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_MGR_AddFormatMsgEntry(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2)
{
    UI32_T  syslog_status;
    BOOL_T ret = TRUE;

#if (SYS_CPNT_REMOTELOG == TRUE)
    UI32_T  remotelog_status;
#endif

#if (SYS_CPNT_SMTP == TRUE)
    UI32_T  smtp_status;
#endif

    SYSLOG_OM_GetSyslogStatus(&syslog_status);
    if (syslog_status == SYSLOG_STATUS_ENABLE)
    {
        //ret = SYSLOG_MGR_AddFormatMsgEntry_local(owner_info,message_index,arg_0,arg_1,arg_2);
        if(SYSLOG_MGR_AddFormatMsgEntry_local(owner_info,message_index,arg_0,arg_1,arg_2) == TRUE)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

    }
#if (SYS_CPNT_REMOTELOG == TRUE)
        /*if (ret == FALSE)
            return FALSE;*/

        SYSLOG_MGR_GetRemoteLogStatus(&remotelog_status);

        if (remotelog_status == SYSLOG_STATUS_ENABLE)
        {
            //SYSFUN_Sleep(5);

            //ret = SYSLOG_MGR_AddFormatMsgEntry_Remote(owner_info,message_index,arg_0,arg_1,arg_2);
            if(SYSLOG_MGR_AddFormatMsgEntry_Remote(owner_info,message_index,arg_0,arg_1,arg_2) == SYSLOG_REMOTE_SUCCESS)
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }
        }
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_MGR_GetSmtpAdminStatus(&smtp_status);
    if(smtp_status == SMTP_STATUS_ENABLE)
    {
        //ret = SYSLOG_MGR_AddFormatMsgEntry_Smtp(owner_info,message_index,arg_0,arg_1,arg_2);
        if(SYSLOG_MGR_AddFormatMsgEntry_Smtp(owner_info,message_index,arg_0,arg_1,arg_2) == TRUE)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }
    #endif

    return ret;
}

/* FUNCTION NAME: SYSLOG_MGR_AddFormatMsgEntry_local
 * PURPOSE: Add a log message to system log module using format message.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
static BOOL_T
SYSLOG_MGR_AddFormatMsgEntry_local(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2)
{
    UI32_T  syslog_status, uc_log_level, flash_log_level;
    SYSLOG_OM_Record_T syslog_entry;
    UI32_T  log_time;

    if (owner_info == 0)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_ADD_FORMAT_MSG_ENTRY_LOCAL_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return FALSE;
    }


    SYSLOG_OM_GetSyslogStatus(&syslog_status);

    if (syslog_status != SYSLOG_STATUS_ENABLE)
    {

        return FALSE;
    }

    if (owner_info->level > SYSLOG_LEVEL_DEBUG)
    {

        return FALSE;
    }

    SYSLOG_OM_GetUcLogLevel(&uc_log_level);

    if (uc_log_level < owner_info->level)
    {

        return FALSE;
    }

    if (owner_info->module_no >= SYS_MODULE_UNKNOWN)
    {

        return FALSE;
    }

    memset(&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));

#if (SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT == TRUE)
    if (LOGIN_MESSAGE_INDEX == message_index ||
        LOGOUT_MESSAGE_INDEX == message_index)
    {
        syslog_entry.profile_id = SYSLOG_OM_LOGIN_OUT_PROFILE;
    }
#endif /* SYS_CPNT_SYSLOG_INDEPENDENT_LOGIN_OUT */

    syslog_entry.owner_info.level = owner_info->level;
    syslog_entry.owner_info.module_no = owner_info->module_no;
    syslog_entry.owner_info.function_no = owner_info->function_no;
    syslog_entry.owner_info.error_no = owner_info->error_no;
    //memcpy(&(syslog_entry.owner_info), owner_info, sizeof(SYSLOG_OM_RecordOwnerInfo_T));
    //syslog_entry.owner_info = *owner_info;

    if (FALSE == SYSLOG_MGR_FormatMessage(message_index, arg_0, arg_1, arg_2,
                                          (char *)syslog_entry.message, sizeof(syslog_entry.message)))
    {
        return FALSE;
    }

    SYS_TIME_GetRealTimeBySec(&log_time);

    syslog_entry.log_time = log_time;

    SYSLOG_OM_AddUcNormalEntry(&syslog_entry);
    SYSLOG_OM_GetFlashLogLevel(&flash_log_level);

    if (flash_log_level >= owner_info->level)
    {
        SYSLOG_OM_AddUcFlashEntry(&syslog_entry);
    }
    /* if uc_flash_db is full, start to store log entry to flash */
    if (SYSLOG_OM_CheckUcFlashFull() == TRUE)
    {

        SYSLOG_MGR_LogUcFlashDbToLogFile();
    }

    return TRUE;

} /* End of SYSLOG_MGR_AddFormatMsgEntry */


/* FUNCTION NAME: SYSLOG_MGR_FormatMessage
 * PURPOSE: Format a message string.
 * INPUT:   message_index   -- The message identifier for the requested message.
 *          size            -- The size of the output buffer in chars.
 * OUTPUT:  message         -- A pointer to a buffer that receives the
 *                             null-terminated string that specifies the
 *                             formatted message.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the size of the formated message string is larger than output
 *          buffer, the formated message string will be cut out.
 */
static BOOL_T
SYSLOG_MGR_FormatMessage(
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2,
    char    *message,
    UI32_T  size)
{
    char   temp_string1[32] = {0}, temp_string2[32] = {0};

    if (NULL == message ||
        0 == size)
    {
        return FALSE;
    }

    memset(message, '.', (size_t)size);
    message[0] = '\0';

    switch (message_index)
    {
        case CREATE_TASK_FAIL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_1, (UI8_T *) arg_0);
            break;

        case SWITCH_TO_DEFAULT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_2, (UI8_T *) arg_0);
            break;

        case ALLOCATE_MEMORY_FAIL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_3, (UI8_T *) arg_0);
            break;

        case FREE_MEMORY_FAIL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_4, (UI8_T *) arg_0);
            break;

        case TASK_IDLE_TOO_LONG_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_5, (UI8_T *) arg_0);
            break;

        case FAN_FAIL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_6, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
            break;

        case FUNCTION_RETURN_FAIL_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_7, (UI8_T *) arg_0);
            break;

        case SYSTEM_COLDSTART_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_8);
            break;

        case SYSTEM_WARMSTART_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_9);
            break;

        case NORMAL_PORT_LINK_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_10, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (char *)arg_2);
            break;

        case NORMAL_PORT_LINK_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_11, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
            break;

        case TRUNK_PORT_LINK_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_12, (unsigned long)*(UI32_T *) arg_0);
            break;

        case TRUNK_PORT_LINK_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_13, (unsigned long)*(UI32_T *) arg_0);
            break;

        case VLAN_LINK_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_14, (unsigned long)*(UI32_T *) arg_0);
            break;

        case VLAN_LINK_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_15, (unsigned long)*(UI32_T *) arg_0);
            break;

        case AUTHENTICATION_FAILURE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_16);
            break;

        case STA_ROOT_CHANGE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_17);
            break;

        case STA_TOPOLOGY_CHANGE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_18, (UI8_T*)arg_0);
            break;

	 case STA_TOPOLOGY_CHANGE_MESSAGE_INDEX_RECEIVE:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_258, (UI8_T*)arg_0,
                           ( (UI8_T *)arg_1)[0], ( (UI8_T *)arg_1)[1],( (UI8_T *)arg_1) [2], ( (UI8_T *)arg_1)[3] ,
                           ( (UI8_T *)arg_1)[4], ( (UI8_T *)arg_1)[5]);
            break;

        case RMON_RISING_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_19, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
            break;

        case RMON_FALLING_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_20, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
            break;

        case POWER_STATUS_CHANGE_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_swIndivPowerIndex_internalPower)
                sprintf(temp_string1, "main");
            else if ((*(UI32_T *) arg_1) == VAL_swIndivPowerIndex_externalPower)
                sprintf(temp_string1, "redundant");
            else
            {
                return FALSE;
            }

            if ((*(UI32_T *) arg_2) == VAL_swIndivPowerStatus_notPresent)
                sprintf(temp_string2, "not exist");
            else if ((*(UI32_T *) arg_2) == VAL_swIndivPowerStatus_green)
                sprintf(temp_string2, "good");
            else if ((*(UI32_T *) arg_2) == VAL_swIndivPowerStatus_red)
                sprintf(temp_string2, "fail");
            else
            {

                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_21, (unsigned long)*(UI32_T *) arg_0, temp_string1, temp_string2);
            break;

        case ACTIVE_POWER_CHANGE_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_swIndivPowerIndex_internalPower)
                sprintf(temp_string1, "main");
            else if ((*(UI32_T *) arg_1) == VAL_swIndivPowerIndex_externalPower)
                sprintf(temp_string1, "redundant");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_247, (unsigned long)*(UI32_T *) arg_0, temp_string1);
            break;

        case OVER_HEAT_TRAP_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_22);
            break;

        case MAC_ADDRESS_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_23);
            break;

#ifdef FTTH_OKI  /* FTTH_OKI */
        case INTERFACE_CARD_COLD_START_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {

                return FALSE;
            }

            if ((*(UI32_T *) arg_2) == VAL_trapRestartType_command)
                sprintf(temp_string2, "command");
            else if ((*(UI32_T *) arg_2) == VAL_trapRestartType_powerOn)
                sprintf(temp_string2, "power on");
            else if ((*(UI32_T *) arg_2) == VAL_trapRestartType_none)
                sprintf(temp_string2, "none");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_24, (unsigned long)*(UI32_T *) arg_0, temp_string1, temp_string2);
            break;

        case UMC_POWER_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_25, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_POWER_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_26, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_TEST_START_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_27, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_NOT_TEST_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_28, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_OAM_SETTING_TIMEOUT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_29, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_LBERROR_TIMEOUT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_30, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_OAM_RELEASE_TIMEOUT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_31, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_NORMAL_END_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_32, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;
        case LOOPBACK_BUSY_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_33, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_STOP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_34, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case LOOPBACK_UMC_TIMEOUT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_35, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case DIAGNOSIS_START_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_36, *(UI32_T *) arg_0, temp_string1);
            break;

        case DIAGNOSIS_RESULT_MESSAGE_INDEX:
#if 0
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {

                return FALSE;
            }


            if ((*(UI32_T *) arg_2) == VAL_trapRestartType_command)
                sprintf(temp_string2, "command");
            else if ((*(UI32_T *) arg_2) == VAL_trapRestartType_powerOn)
                sprintf(temp_string2, "power on");
            else if ((*(UI32_T *) arg_2) == VAL_trapRestartType_none)
                sprintf(temp_string2, "none");
            else
            {

                return FALSE;
            }


            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_37, *(UI32_T *) arg_0, temp_string1, temp_string2);
#endif
            break;

        case FAN_FAILURE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_38, *(UI32_T *) arg_0);
            break;

        case FAN_RECOVER_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_39, *(UI32_T *) arg_0);
            break;

        case UMC_LIP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_40, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_LIP_RECOVER_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_41, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_LINKDOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_42, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_LINKUP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_43, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_MC_FAULT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_44, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case UMC_MC_FAULT_RECOVER_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_45, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case PORT_LINK_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_46, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case PORT_LINK_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_47, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case HARDWARE_FAULT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_48, *(UI32_T *) arg_0, temp_string1);
            break;

        case HARDWARE_FAULT_RECOVER_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_49, *(UI32_T *) arg_0, temp_string1);
            break;

        case LINK_CHANGE_EVENT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string1, "none");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string1, "minor-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string1, "major-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string1, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_50, *(UI32_T *) arg_0, *(UI32_T *) arg_1, temp_string1);
            break;

        case UMC_UTP_LINK_EVENT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string1, "none");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string1, "minor-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string1, "major-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string1, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_51, *(UI32_T *) arg_0, *(UI32_T *) arg_1, temp_string1);
            break;

        case UMC_LIP_EVENT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string1, "none");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string1, "minor-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string1, "major-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string1, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_52, *(UI32_T *) arg_0, *(UI32_T *) arg_1, temp_string1);
            break;

        case UMC_MC_FAULT_EVENT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string1, "none");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string1, "minor-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string1, "major-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string1, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_53, *(UI32_T *) arg_0, *(UI32_T *) arg_1, temp_string1);
            break;

        case FAN_EVENT_MESSAGE_INDEX:

            if ((*(UI32_T *) arg_1) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string1, "none");
            else if ((*(UI32_T *) arg_1) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string1, "minor-fail");
            else if ((*(UI32_T *) arg_1) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string1, "major-fail");
            else if ((*(UI32_T *) arg_1) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string1, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_54, *(UI32_T *) arg_0, temp_string1);
            break;

        case HARDWARE_FAULT_EVENT_MESSAGE_INDEX:
            if ((*(UI32_T *) arg_1) == VAL_bladeType_other)
                sprintf(temp_string1, "other");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6208)
                sprintf(temp_string1, "BBM6208");
            else if ((*(UI32_T *) arg_1) == VAL_bladeType_bbm6216)
                sprintf(temp_string1, "BBM6216");
            else
            {
                return FALSE;
            }

            if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_none)
                sprintf(temp_string2, "none");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_minorFail)
                sprintf(temp_string2, "minor-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_majorFail)
                sprintf(temp_string2, "major-fail");
            else if ((*(UI32_T *) arg_2) == VAL_trapSensitivityEventType_clear)
                sprintf(temp_string2, "clear");
            else
            {
                return FALSE;
            }

            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_55, *(UI32_T *) arg_0, temp_string1, temp_string2);
            break;
#endif  /* END_OF_FTTH_OKI */

        case PORT_SECURITY_TRAP_INDEX:
            SYSFUN_Snprintf(temp_string1, sizeof(temp_string1)-1, "%02X%02X%02X%02X%02X%02X",
                ((UI8_T *)arg_2)[0], ((UI8_T *)arg_2)[1],((UI8_T *)arg_2)[2],
                ((UI8_T *)arg_2)[3],((UI8_T *)arg_2)[4],((UI8_T *)arg_2)[5]);
            temp_string1[ sizeof(temp_string1)-1 ] = '\0';
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_56, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, temp_string1);
            break;

        case LOOPBACK_TEST_FAILURE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_57);
            break;

        case FAN_RECOVERY_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_58, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
            break;

        case MGMT_IP_FLT_REJECT_MESSAGE_INDEX:
            {
                /* ES3550MO-PoE-FLF-AA-00235
                 * ip address passed by other component must be network
                 * byte order
                 */
                UI8_T   *ip_byte_p = (UI8_T*)arg_1;

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_59, (unsigned long)*(UI32_T *) arg_0,
                    ip_byte_p[0], ip_byte_p[1], ip_byte_p[2], ip_byte_p[3]);
            }
            break;

        case MGMT_IP_FLT_INET_REJECT_MESSAGE_INDEX:
            {
                L_INET_AddrIp_T   *ip_addr = (L_INET_AddrIp_T *)arg_1;
                char    ipaddr_str[L_INET_MAX_IPADDR_STR_LEN + 1]={0};

                if (*(UI32_T *) arg_0 != VAL_trapIpFilterRejectMode_web &&
                    *(UI32_T *) arg_0 != VAL_trapIpFilterRejectMode_snmp &&
                    *(UI32_T *) arg_0 != VAL_trapIpFilterRejectMode_telnet)
                {
                    return FALSE;
                }

                L_INET_InaddrToString((L_INET_Addr_T *)ip_addr, ipaddr_str, sizeof(ipaddr_str));

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_183,
                                *(UI32_T *) arg_0 == VAL_trapIpFilterRejectMode_web ? "WEB" :
                                *(UI32_T *) arg_0 == VAL_trapIpFilterRejectMode_snmp ? "SNMP" : "Telnet",
                                ipaddr_str);
            }
            break;

        case DHCP_IP_RETRIEVE_FAILURE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_63);
            break;

        case DHCP_IP_RETRIEVE_SUCCESS_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_64);
            break;

        case SMTP_CONN_FAILURE_MESSAGE_INDEX:
            {
                /* ES3550MO-PoE-FLF-AA-00235
                 * ip address passed by other component must be network
                 * byte order
                 */
                UI32_T ip;

                ip = *(UI32_T *) arg_0;
                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_65,
                    L_INET_Ntoa(ip, (UI8_T *)temp_string1));
            }
            break;

#if (SYS_CPNT_POE == TRUE)
        case PETH_PSE_PORT_ON_OFF_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_66, *(UI32_T *) arg_0, *(UI32_T *) arg_1,
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_disabled?"disabled":
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_searching?"searching":
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_deliveringPower?"delivering power":
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_fault?"fault":
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_test?"test":
              (*(UI32_T *) arg_2)==VAL_pethPsePortDetectionStatus_otherFault?"other fault":
              "unknown status"
              );
            break;
        case PETH_MAIN_POWER_USAGE_ON_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_67, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;
        case PETH_MAIN_POWER_USAGE_OFF_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_68, *(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;
#endif  /* SYS_CPNT_POE */

        case THERMAL_RISING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_81, (UI8_T *) arg_0);
            break;

        case THERMAL_FALLING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_82, (UI8_T *) arg_0);
            break;

        case MAIN_BOARD_VER_MISMATCH_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_83, (UI8_T *) arg_0);
            break;

        case MODULE_VER_MISMATCH_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_84, (UI8_T *) arg_0);
            break;

        case MODULE_INSERTION_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_85, (UI8_T *) arg_0);
            break;

        case MODULE_REMOVAL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_86, (UI8_T *) arg_0);
            break;

        case TCN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_87, (UI8_T *) arg_0);
            break;

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
        case SECURE_ADDRESS_LEARNED_INDEX:
            sprintf(temp_string1, "%02X%02X%02X%02X%02X%02X",
                ((UI8_T *)arg_2)[0], ((UI8_T *)arg_2)[1],((UI8_T *)arg_2)[2],
                ((UI8_T *)arg_2)[3],((UI8_T *)arg_2)[4],((UI8_T *)arg_2)[5]);
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_88,temp_string1,*(UI32_T *) arg_0, *(UI32_T *) arg_1);
            break;

        case SECURE_VIOLATION2_INDEX:
            sprintf(temp_string1, "%02X%02X%02X%02X%02X%02X",
                ((UI8_T *)arg_2)[0], ((UI8_T *)arg_2)[1],((UI8_T *)arg_2)[2],
                ((UI8_T *)arg_2)[3],((UI8_T *)arg_2)[4],((UI8_T *)arg_2)[5]);
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_89,*(UI32_T *) arg_0, *(UI32_T *) arg_1,temp_string1);
            break;

        case SECURE_LOGIN_FAILURE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_90,*(UI32_T *) arg_0, *(UI32_T *) arg_1,(UI8_T *)arg_2);
            break;

        case SECURE_LOGON_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_91,*(UI32_T *) arg_0, *(UI32_T *) arg_1,(UI8_T *)arg_2);
            break;

        case SECURE_LOGOFF_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_92,*(UI32_T *) arg_0, *(UI32_T *) arg_1,(UI8_T *)arg_2);
            break;
#endif  /* (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32) */

#if(SYS_CPNT_VRRP == TRUE)
        case VRRP_VM_RECEIVE_PACKET_DA_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_93);
            break;

        case VRRP_VM_RECEIVE_PACKET_RIFNUM_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_94);
            break;

        case VRRP_VM_L_MEM_ALLOCATE_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_95);
            break;

        case VRRP_VM_L_MREF_GETPDU_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_96);
            break;

        case VRRP_VM_MISCONFIGURATION_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_97);
            break;

        case VRRP_VM_GET_PRIMARY_RIFNUM_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_98);
            break;

        case VRRP_VM_GET_PRIMARY_RIFINFO_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_99);
            break;

        case VRRP_VM_OPER_STATE_CHANGE_INDEX:
            SYSFUN_Snprintf(message, size,MESSAGE_INDEX_233,(UI8_T *)arg_0);
            break;

        case VRRP_TXRX_SENDADVERTISEMENT_L_MEM_ALLOCATE_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_100);
            break;

        case VRRP_TXRX_L_MREF_CONSTRUCTOR_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_101);
            break;

        case VRRP_TXRX_GET_PRIMARY_RIFNUM_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_102);
            break;

        case VRRP_TXRX_GET_PRIMARY_RIFINFO_ERROR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_103);
            break;
#endif

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
        case TEST_TRAP_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_104);
            break;

        case IP_FORWARDING_CHANGED_BY_MSTP_TRAP_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_105);
            break;
#endif  /* (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32) */

#if (SYS_CPNT_DAI == TRUE)
     case DAI_DROP_ARP_PACKET_MESSAGE_INDEX:
     {
         DAI_TYPE_LogEntry_T *log_entry;

         log_entry = (DAI_TYPE_LogEntry_T* ) arg_0;
         memset(temp_string1, 0, sizeof(temp_string1));
         sprintf(temp_string1, "%u.%u.%u.%u",L_INET_EXPAND_IP(log_entry->src_ip_addr));
         memset(temp_string2, 0, sizeof(temp_string2));
         sprintf(temp_string2, "%02x%02x%02x%02x%02x%02x", log_entry->src_mac[0],
             log_entry->src_mac[1],log_entry->src_mac[2],log_entry->src_mac[3],log_entry->src_mac[4],log_entry->src_mac[5]);
         SYSFUN_Snprintf(message, size, MESSAGE_INDEX_169, temp_string1, temp_string2, (long)log_entry->vlan_id, (long)log_entry->port_num);
     }
         break;
#endif

#if (SYS_CPNT_LLDP == TRUE)
        case LLDP_REM_TABLE_CHANGED_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_106);
            break;

        case LLDP_REMOTE_TABLE_CHANGED_PER_PORT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_257, (char *)arg_0);
            break;
#endif /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_LLDP_MED == TRUE)
        case LLDP_MED_TOPOLOGY_CHANGE_DETECTED_INDEX:
            {
                UI8_T tmp_str[SYS_ADPT_LLDP_MAX_CHASSIS_ID_LENGTH*2-1]={0}, index, *id_p, shift=0;

                id_p = (UI8_T *) arg_1;
                if (id_p[0] > SYS_ADPT_LLDP_MAX_CHASSIS_ID_LENGTH - 1)
                {
                    /* printf("\r\nSYSLOG_MGR_AddFormatMsgEntry_local: fatal error, length of ID is too long.\r\n"); */
                    return FALSE;
                }

                for (index = 1; index <= id_p[0]; index++, shift+=2)
                    sprintf((char *)(tmp_str+shift), "%02x",  id_p[index]);
                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_153, (unsigned long)*(UI32_T *) arg_0, tmp_str, (unsigned long)*(UI32_T *) arg_2);
            }
            break;
#endif

    #if (SYS_CPNT_EFM_OAM == TRUE)
        case DOT3_OAM_THRESHOLD_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_143, *(UI32_T *) arg_0, *(UI32_T *) arg_1, (UI8_T *) arg_2);
           break;

        case DOT3_OAM_NON_THRESHOLD_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_144, *(UI32_T *) arg_0, *(UI32_T *) arg_1, (UI8_T *) arg_2);
           break;
    #endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
        case STP_BPDU_GUARD_PORT_SHUTDOWN_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_145, (unsigned long)*(UI32_T *) arg_0);
           break;
#endif

#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
        case CPU_RAISE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_155);
            break;

        case CPU_FALLING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_156);
            break;

        case MEMORY_RAISE_EVENT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_157);
            break;

        case MEMORY_FALLING_EVENT_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_158);
            break;
#endif /* #if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
        case CPU_GUARD_CONTROL_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_259);
            break;

        case CPU_GUARD_RELEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_260);
            break;
#endif

#if (SYS_CPNT_ATC_STORM == TRUE)
        case BCAST_STORM_ALARM_FIRE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_160,*(UI32_T *) arg_0);
            break;

        case BCAST_STORM_ALARM_CLEAR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_161,*(UI32_T *) arg_0);
            break;

        case BCAST_STORM_TC_APPLY_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_162,*(UI32_T *) arg_0);
            break;

        case BCAST_STORM_TC_RELEASE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_163,*(UI32_T *) arg_0);
            break;

        case MCAST_STORM_ALARM_FIRE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_164,*(UI32_T *) arg_0);
            break;

        case MCAST_STORM_ALARM_CLEAR_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_165,*(UI32_T *) arg_0);
            break;

        case MCAST_STORM_TC_APPLY_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_166,*(UI32_T *) arg_0);
            break;

        case MCAST_STORM_TC_RELEASE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_167,*(UI32_T *) arg_0);
            break;
#endif

#if(SYS_CPNT_CFM == TRUE)
        case CFM_MEP_UP_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_170, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_MEP_DOWN_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_171, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_CONFIG_FAIL_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_172, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_LOOP_FIND_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_173, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_MEP_UNKNOWN_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_174, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_MEP_MISSING_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_175, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1, (unsigned long)*(UI32_T *) arg_2);
           break;

        case CFM_MA_UP_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_176, (unsigned long)*(UI32_T *) arg_0, (unsigned long)*(UI32_T *) arg_1);
           break;

        case CFM_FAULT_ALARM_MESSAGE_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_177, (unsigned long)*(UI32_T *) arg_0, (char *)arg_1);
           break;
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        case NETWORK_ACCESS_PORT_LINK_DETECTION_INDEX:
           SYSFUN_Snprintf(message, size, MESSAGE_INDEX_168,(long) *(UI32_T *) arg_0, (long)*(UI32_T *) arg_1, (char *) arg_2);
           break;
#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

#if(SYS_CPNT_XFER_AUTO_UPGRADE==TRUE)
        case XFER_AUTO_UPGRADE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_178, (char *) arg_0, (char *) arg_1);
            break;
#endif

        /* DHCP client sends a trap when receiving a packet from a rogue server. */
        case DHCP_ROGUE_SERVER_ATTACK_MESSAGE_INDEX:
			SYSFUN_Snprintf(message, size, MESSAGE_INDEX_186, (UI8_T *) arg_0, (UI8_T *) arg_1, (UI8_T *) arg_2);
            break;

        case XSTP_PORT_STATE_CHANGE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_190, (unsigned long)*(UI32_T*)arg_0, (UI8_T*)arg_1);
            break;

#if (SYS_CPNT_DHCPV6SNP == TRUE)
        case DHCPV6SNP_DROP_BOGUS_SERVER_PACKET_MESSAGE_INDEX:
            sprintf(temp_string1, "%02X%02X%02X%02X%02X%02X", ((UI8_T *)arg_0)[0],
                 ((UI8_T *)arg_0)[1],((UI8_T *)arg_0)[2],((UI8_T *)arg_0)[3],((UI8_T *)arg_0)[4],((UI8_T *)arg_0)[5]);
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_192, temp_string1, (unsigned long)*(UI32_T *)arg_1,(unsigned long)*(UI32_T *)arg_2);
            break;
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
        case DHCP_INFORM_RETRIEVE_FAILURE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_193, (unsigned long)*((UI32_T *) arg_0));
            break;

        case DHCP_INFORM_RETRIEVE_SUCCESS_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_194, (unsigned long)*((UI32_T *) arg_0));
            break;
#endif
        case MAJOR_ALARM_POWER_STATUS_CHANGED_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_195);
            break;
        case MAJOR_ALARM_POWER_STATUS_RECOVER_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_196);
            break;
        case MINOR_ALARM_THERMAL_OVERHEAT_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_197);
            break;
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        case SFP_TX_POWER_HIGH_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_198, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TX_POWER_LOW_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_199, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TX_POWER_HIGH_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_200, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TX_POWER_LOW_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_201, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TX_POWER_ALARMWARN_CEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_234, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_RX_POWER_HIGH_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_202, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_RX_POWER_LOW_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_203, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_RX_POWER_HIGH_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_204, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_RX_POWER_LOW_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_205, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_RX_POWER_ALARMWARN_CEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_235, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TEMP_HIGH_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_206, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TEMP_LOW_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_207, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TEMP_HIGH_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_208, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TEMP_LOW_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_209, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_TEMP_ALARMWARN_CEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_236, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_VOLTAGE_HIGH_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_210, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_VOLTAGE_LOW_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_211, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_VOLTAGE_HIGH_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_212, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_VOLTAGE_LOW_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_213, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_VOLTAGE_ALARMWARN_CEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_237, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_CURRENT_HIGH_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_214, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_CURRENT_LOW_ALARM_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_215, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_CURRENT_HIGH_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_216, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_CURRENT_LOW_WARNING_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_217, (unsigned long)*((UI32_T *) arg_0));
            break;

        case SFP_CURRENT_ALARMWARN_CEASE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_238, (unsigned long)*((UI32_T *) arg_0));
            break;
#endif
        case MAJOR_ALARM_ALL_FAN_FAILURE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_218);
            break;
        case MAJOR_ALARM_FAN_RECOVER_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_219);
            break;
        case MINOR_ALARM_PARTITAL_FAN_FAIL_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_220);
            break;
        case ALARM_INPUT_TYPE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_221, (unsigned long)*((UI32_T *) arg_0));
            break;
        case MAJOR_ALARM_POWER_MODULE_SET_WRONG_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_222);
            break;
        case MAJOR_ALARM_POWER_MODULE_SET_RECOVER_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_223);
            break;

#if (SYS_CPNT_BGP == TRUE)
        case BGP_NEIGHBOR_CHANGE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_224, (UI8_T *) arg_0);
            break;
        case BGP_ESTABLISHED_NOTIFICATION_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_225, (UI8_T *) arg_0, *((UI16_T *) arg_1), (UI8_T *) arg_2);
            break;
        case BGP_BACKWARD_TRANS_NOTIFICATION_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_226, (UI8_T *) arg_0, *((UI16_T *) arg_1), (UI8_T *) arg_2);
            break;
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
        case CRAFT_PORT_LINK_UP_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_228,
                (unsigned long)*((UI32_T *) arg_0));
            break;

        case CRAFT_PORT_LINK_DOWN_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_229,
                (unsigned long)*((UI32_T *) arg_0));
            break;
#endif

        case XSTP_ROOT_BRIDGE_CHANGED_INDEX:
        {
            UI8_T root_bridge_id[6];

            memcpy(root_bridge_id, (UI8_T *)arg_2, 6);
            SYSFUN_Snprintf(temp_string1, 31, "%u.%lu.%02X%02X%02X%02X%02X%02X",
                *((UI16_T *)arg_0), (unsigned long)*((UI32_T *)arg_1),
                root_bridge_id[0], root_bridge_id[1], root_bridge_id[2],
                root_bridge_id[3], root_bridge_id[4], root_bridge_id[5]);
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_232, temp_string1);
        }
            break;
#if(SYS_CPNT_SYNCE == TRUE)
        case SYNCE_SSM_RX_MESSAGE_INDEX:
        {
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_245, (UI8_T *)arg_0,
                *((UI32_T *) arg_1)== VAL_syncEPortStatus_enabled?"Enabled":"Disabled",
                *((UI32_T *) arg_2)== VAL_syncEPortSSMStatus_enabled?"Enabled":"Disabled");
        }
            break;

        case SYNCE_CLOCK_SRC_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_246,
                (UI8_T *)arg_0);
            break;
#endif

        case USERAUTH_AUTHENTICATION_FAILURE_MESSAGE_INDEX:
            {
                char  user_info_str[SYSLOG_MGR_USER_INFO_STR_LEN_MAX+1] = {0};

                if (FALSE == SYSLOG_MGR_GetUserInfoString(
                                 (SYSLOG_MGR_UserInfo_T *) arg_0,
                                 user_info_str,
                                 sizeof(user_info_str)-1))
                {
                    return FALSE;
                }

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_248, user_info_str);
            }
            break;

        case USERAUTH_AUTHENTICATION_SUCCESS_MESSAGE_INDEX:
            {
                char  user_info_str[SYSLOG_MGR_USER_INFO_STR_LEN_MAX+1] = {0};

                if (FALSE == SYSLOG_MGR_GetUserInfoString(
                                 (SYSLOG_MGR_UserInfo_T *) arg_0,
                                 user_info_str,
                                 sizeof(user_info_str)-1))
                {
                    return FALSE;
                }

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_249, user_info_str);
            }
            break;

        case LOGIN_MESSAGE_INDEX:
            {
                char  user_info_str[SYSLOG_MGR_USER_INFO_STR_LEN_MAX+1] = {0};

                if (FALSE == SYSLOG_MGR_GetUserInfoString(
                                 (SYSLOG_MGR_UserInfo_T *) arg_0,
                                 user_info_str,
                                 sizeof(user_info_str)-1))
                {
                    return FALSE;
                }

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_250, user_info_str);
            }
            break;

        case LOGOUT_MESSAGE_INDEX:
            {
                char  user_info_str[SYSLOG_MGR_USER_INFO_STR_LEN_MAX+1] = {0};

                if (FALSE == SYSLOG_MGR_GetUserInfoString(
                                 (SYSLOG_MGR_UserInfo_T *) arg_0,
                                 user_info_str,
                                 sizeof(user_info_str)-1))
                {
                    return FALSE;
                }

                SYSFUN_Snprintf(message, size, MESSAGE_INDEX_251, user_info_str);
            }
            break;

        case XFER_FILE_COPY_MESSAGE_INDEX:
            {
                TRAP_EVENT_XferFileCopyInfo_T  *file_copy_info_p;
                UI32_T  message_len_max;
                UI32_T  file_copy_message_str_len;
                UI32_T  ellipsis_str_len;
                char    user_info_str[SYSLOG_MGR_USER_INFO_STR_LEN_MAX+1] = {0};
                char    server_address_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
                char    file_type_str[SYSLOG_MGR_FILE_COPY_FILE_TYPE_STR_LEN_MAX+1] = {0};
                char    src_oper_str[SYSLOG_MGR_FILE_COPY_SOURCE_OPERATION_STR_LEN_MAX+1] = {0};
                char    dest_oper_str[SYSLOG_MGR_FILE_COPY_DESTINATION_OPERATION_STR_LEN_MAX+1] = {0};
                char    result_str[SYSLOG_MGR_FILE_COPY_RESULTE_STR_LEN_MAX+1] = {0};
                char    *file_copy_message_p;
                char    *display_p;

                file_copy_info_p = (TRAP_EVENT_XferFileCopyInfo_T *) arg_1;

                if (0 != file_copy_info_p->server_address.addrlen)
                {
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&file_copy_info_p->server_address,
                                                                       server_address_str,
                                                                       sizeof(server_address_str)))
                    {
                        return FALSE;
                    }
                }

                switch (file_copy_info_p->file_type)
                {
                    case VAL_fileCopyFileType_opcode:
                        strncpy(file_type_str, "opcode", sizeof(file_type_str)-1);
                        break;

                    case VAL_fileCopyFileType_config:
                        strncpy(file_type_str, "config", sizeof(file_type_str)-1);
                        break;

                    case VAL_fileCopyFileType_bootRom:
                        strncpy(file_type_str, "bootRom", sizeof(file_type_str)-1);
                        break;

                    case VAL_fileCopyFileType_publickey:
                        strncpy(file_type_str, "publickey", sizeof(file_type_str)-1);
                        break;

                    case VAL_fileCopyFileType_certificate:
                        strncpy(file_type_str, "cert/privkey", sizeof(file_type_str)-1);
                        break;

                    case VAL_fileCopyFileType_loader:
                        strncpy(file_type_str, "loader", sizeof(file_type_str)-1);
                        break;

                    default:
                        return FALSE;
                }

                file_type_str[sizeof(file_type_str)-1] = '\0';

                switch (file_copy_info_p->src_oper_type)
                {
                    case VAL_fileCopySrcOperType_startUpCfg:
                        snprintf(src_oper_str, sizeof(src_oper_str),
                            "startup-config(%s)",
                            file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_runningCfg:
                        strncpy(src_oper_str, "running-config",
                            sizeof(src_oper_str)-1);
                        break;

                    case VAL_fileCopySrcOperType_file:
                        snprintf(src_oper_str, sizeof(src_oper_str), "file(%s)",
                            file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_tftp:
                        snprintf(src_oper_str, sizeof(src_oper_str), "TFTP(%s/%s)",
                            server_address_str, file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_unit:
                        snprintf(src_oper_str, sizeof(src_oper_str), "unit(%lu/%s)",
                            (unsigned long)file_copy_info_p->unit, file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_http:
                        strncpy(src_oper_str, "HTTP", sizeof(src_oper_str)-1);
                        break;

                    case VAL_fileCopySrcOperType_ftp:
                        snprintf(src_oper_str, sizeof(src_oper_str), "FTP(%s/%s)",
                            server_address_str, file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_ftps:
                        snprintf(src_oper_str, sizeof(src_oper_str), "FTPS(%s/%s)",
                            server_address_str, file_copy_info_p->src_file_name);
                        break;

                    case VAL_fileCopySrcOperType_usb:
                        snprintf(src_oper_str, sizeof(src_oper_str), "usbdisk(%s)",
                            file_copy_info_p->src_file_name);
                        break;

                    default:
                        return FALSE;
                }

                src_oper_str[sizeof(src_oper_str)-1] = '\0';

                switch (file_copy_info_p->dest_oper_type)
                {
                    case VAL_fileCopyDestOperType_startUpCfg:
                        snprintf(dest_oper_str, sizeof(dest_oper_str),
                            "startup-config(%s)",
                            file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_runningCfg:
                        strncpy(dest_oper_str, "running-config",
                           sizeof(dest_oper_str)-1);
                        break;

                    case VAL_fileCopyDestOperType_file:
                        snprintf(dest_oper_str, sizeof(dest_oper_str), "file(%s)",
                            file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_tftp:
                        snprintf(dest_oper_str, sizeof(dest_oper_str), "TFTP(%s/%s)",
                            server_address_str, file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_unit:
                        snprintf(dest_oper_str, sizeof(dest_oper_str), "unit(%lu/%s)",
                            (unsigned long)file_copy_info_p->unit, file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_http:
                        strncpy(dest_oper_str, "HTTP", sizeof(dest_oper_str)-1);
                        break;

                    case VAL_fileCopyDestOperType_ftp:
                        snprintf(dest_oper_str, sizeof(dest_oper_str), "FTP(%s/%s)",
                            server_address_str, file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_ftps:
                        snprintf(dest_oper_str, sizeof(dest_oper_str), "FTPS(%s/%s)",
                            server_address_str, file_copy_info_p->dest_file_name);
                        break;

                    case VAL_fileCopyDestOperType_publickey:
                        strncpy(dest_oper_str, "publickey", sizeof(dest_oper_str)-1);
                        break;

                    default:
                        return FALSE;
                }

                dest_oper_str[sizeof(dest_oper_str)-1] = '\0';

                if (file_copy_info_p->status == VAL_fileCopyStatus_fileCopySuccess)
                {
                    strncpy(result_str, "succeeded", sizeof(result_str)-1);
                }
                else
                {
                    strncpy(result_str, "failed", sizeof(result_str)-1);
                }

                result_str[sizeof(result_str)-1] = '\0';

                if (NULL == (file_copy_message_p = malloc(SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX+1)))
                {
                    return FALSE;
                }

                if (TRUE == SYSLOG_MGR_GetUserInfoString(
                                 (SYSLOG_MGR_UserInfo_T *) arg_0,
                                 user_info_str,
                                 sizeof(user_info_str)-1))
                {
                    SYSFUN_Snprintf(file_copy_message_p, SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX+1,
                        "%s, copy %s %s to %s %s.",
                        user_info_str, file_type_str, src_oper_str, dest_oper_str, result_str);
                    file_copy_message_p[SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX] = '\0';
                }
                else
                {
                    SYSFUN_Snprintf(file_copy_message_p, SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX+1,
                        "copy %s %s to %s %s.",
                        file_type_str, src_oper_str, dest_oper_str, result_str);
                    file_copy_message_p[SYSLOG_MGR_FILE_COPY_MESSAGE_STR_LEN_MAX] = '\0';
                }

                message_len_max = size-1;
                file_copy_message_str_len = strlen(file_copy_message_p);
                ellipsis_str_len = sizeof(SYSLOG_MGR_ELLIPSIS_STR)-1;

                if (file_copy_message_str_len > message_len_max)
                {
                    display_p = file_copy_message_p + (file_copy_message_str_len - message_len_max + ellipsis_str_len);
                    SYSFUN_Snprintf(message, size, MESSAGE_INDEX_252,
                        SYSLOG_MGR_ELLIPSIS_STR, display_p);
                }
                else
                {
                    SYSFUN_Snprintf(message, size, MESSAGE_INDEX_252,
                        "", file_copy_message_p);
                }

                free(file_copy_message_p);
            }
            break;
        case DHCPSNP_LOG_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_253, (char *)arg_0);
            break;

        case USERAUTH_CREATE_USER_MESSAGE_INDEX:
            {
            	  TRAP_EVENT_UserauthAccount_T *userauth_account_p;

            	  userauth_account_p = (TRAP_EVENT_UserauthAccount_T *) arg_0;

            	  SYSFUN_Snprintf(message,
                                  size,
                                  MESSAGE_INDEX_254,
                                 (char *)userauth_account_p->user_name);
            }
            break;

        case USERAUTH_DELETE_USER_MESSAGE_INDEX:
            {
            	  TRAP_EVENT_UserauthAccount_T *userauth_account_p;

            	  userauth_account_p = (TRAP_EVENT_UserauthAccount_T *) arg_0;

            	  SYSFUN_Snprintf(message,
                                  size,
                                  MESSAGE_INDEX_255,
                                  (char *)userauth_account_p->user_name);
            }
            break;

        case USERAUTH_MODIFY_USER_PRIVILEGE_MESSAGE_INDEX:
            {
            	  TRAP_EVENT_UserauthAccount_T *userauth_account_p;

            	  userauth_account_p = (TRAP_EVENT_UserauthAccount_T *) arg_0;

            	  SYSFUN_Snprintf(message,
                                  size,
                                  MESSAGE_INDEX_256,
                                  (char *)userauth_account_p->user_name,
                                  (unsigned long)userauth_account_p->privilege);
            }
            break;

        case IPSG_LOG_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_264, (char *)arg_0);
            break;
#if (SYS_CPNT_HTTP == TRUE)
        case HTTP_LOG_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_266, (char *)arg_0);
            break;
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
        case DYNAMIC_PRIVISION_CFG_OVERWRITE_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_267, (char *)arg_0);
            break;

        case DYNAMIC_PRIVISION_START_PROVISION_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_268, (char *)arg_0);
            break;

        case DYNAMIC_PRIVISION_INVALID_OPTION_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_269, *(UI32_T *)arg_0);
            break;

        case DYNAMIC_PRIVISION_SERVER_NAME_CANNOT_RESOLVED_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_270);
            break;

        case DYNAMIC_PRIVISION_DOWNLOAD_CFG_ERROR_MESSAGE_INDEX:
            SYSFUN_Snprintf(message, size, MESSAGE_INDEX_271, (char *)arg_0);
            break;
#endif /* #if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE) */

        default:
            printf("message_index=%lu is not found.\r\n", (unsigned long)message_index);
            return FALSE;
    }

    if ('.' != message[size-1])
    {
        /* printf("The buffer for format string is too short. message_idx=%u\r\n", message_index); */
    }

    message[size-1] = '\0';
    return TRUE;
}



/* FUNCTION NAME: SYSLOG_MGR_AddEntrySync
 * PURPOSE: Add a log message to system log module synchrously.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the entry will be written to flash, it
 *          will be done after calling this function.
 */
BOOL_T SYSLOG_MGR_AddEntrySync(SYSLOG_OM_Record_T *syslog_entry)
{
    return __SYSLOG_MGR_AddEntry(syslog_entry, TRUE);
}

/* FUNCTION NAME: SYSLOG_MGR_AddEntry
 * PURPOSE: Add a log message to system log module.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_MGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry)
{
    return __SYSLOG_MGR_AddEntry(syslog_entry, FALSE);
}

/* FUNCTION NAME: __SYSLOG_MGR_AddEntry
 * PURPOSE: Add a log message to system log module.
 * INPUT:   *syslog_entry     -- add this syslog entry
 *          flush_immediately -- TRUE: If the entry will be written to flash, it
 *                                     will be done after calling this function.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
static BOOL_T __SYSLOG_MGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry, BOOL_T flush_immediately)
{
    UI32_T  syslog_status;
    UI32_T  ret = (UI32_T)TRUE;

#if (SYS_CPNT_REMOTELOG == TRUE)
    UI32_T  remotelog_status;
#endif

#if (SYS_CPNT_SMTP == TRUE)
    UI32_T  smtp_status;
#endif

    SYSLOG_OM_GetSyslogStatus(&syslog_status);
    if (syslog_status == SYSLOG_STATUS_ENABLE)
    {
        ret = SYSLOG_MGR_AddEntry_local(syslog_entry, flush_immediately);
    }
#if (SYS_CPNT_REMOTELOG == TRUE)
        if (ret == FALSE)
            return FALSE;

        SYSLOG_MGR_GetRemoteLogStatus(&remotelog_status);

        if (remotelog_status == SYSLOG_STATUS_ENABLE)
        {
            ret = SYSLOG_MGR_AddEntry_Remote((SYSLOG_OM_Remote_Record_T*)syslog_entry);
        }
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_MGR_GetSmtpAdminStatus(&smtp_status);
    if(smtp_status == SMTP_STATUS_ENABLE)
    {
        ret = SYSLOG_MGR_AddEntry_Smtp(syslog_entry);
    }
    #endif
    return  (BOOL_T)ret;

}

/* FUNCTION NAME: SYSLOG_MGR_AddEntry
 * PURPOSE: Add a log message to system log module.
 * INPUT:   *syslog_entry     -- add this syslog entry
 *          flush_immediately -- TRUE: If the entry will be written to flash, it
 *                                     will be done after calling this function.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
static BOOL_T SYSLOG_MGR_AddEntry_local(SYSLOG_OM_Record_T *syslog_entry, BOOL_T flush_immediately)
{
    UI32_T  syslog_status, uc_log_level, flash_log_level;
    UI32_T  log_time;

    if (syslog_entry == 0)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_ADD_ENTRY_LOCAL_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return FALSE;
    }


    SYSLOG_OM_GetSyslogStatus(&syslog_status);
    if (syslog_status != SYSLOG_STATUS_ENABLE)
    {

        return FALSE;
    }
    if (syslog_entry->owner_info.level > SYSLOG_LEVEL_DEBUG)
    {

        return FALSE;
    }
    SYSLOG_OM_GetUcLogLevel(&uc_log_level);
    if (uc_log_level < syslog_entry->owner_info.level)
    {

        return FALSE;
    }
    if (syslog_entry->owner_info.module_no >= SYS_MODULE_UNKNOWN)
    {

        return FALSE;
    }
    SYS_TIME_GetRealTimeBySec(&log_time);
    syslog_entry->log_time = log_time;
    SYSLOG_OM_AddUcNormalEntry(syslog_entry);
    SYSLOG_OM_GetFlashLogLevel(&flash_log_level);
    if (flash_log_level >= syslog_entry->owner_info.level)
    {
        SYSLOG_OM_AddUcFlashEntry(syslog_entry);
    }
    /* if uc_flash_db is full, start to store log entry to flash */
    if (flush_immediately==TRUE || SYSLOG_OM_CheckUcFlashFull() == TRUE)
    {

        SYSLOG_MGR_LogUcFlashDbToLogFile();
    }
    //else


    return TRUE;
} /* End of SYSLOG_MGR_AddEntry */


/* FUNCTION NAME: SYSLOG_MGR_ClearAllRamEntries
 * PURPOSE: Clear all log message from system log module in UC RAM memory.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. This function only clear the UC RAM database, but not clear
 *          the UC Flash and file system syslog file.
 */
BOOL_T
SYSLOG_MGR_ClearAllRamEntries(
   void)
{
    BOOL_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return TRUE;
    }

    ret = SYSLOG_OM_ClearUcRamEntries();

    return ret;
} /* End of SYSLOG_MGR_ClearAllRamEntries */


/* FUNCTION NAME: SYSLOG_MGR_ClearAllFlashEntries
 * PURPOSE: Clear all log message from system log module in flash and UC Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   1. This function clear the file system syslog file.
 *
 */
BOOL_T
SYSLOG_MGR_ClearAllFlashEntries(
    void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return TRUE;
    }

    if ((TRUE != SYSLOG_MGR_CreateLogfile())
        ||(TRUE != SYSLOG_OM_ClearUcFlashEntries()))
    {
        return FALSE;
    }

    return TRUE;
} /* End of SYSLOG_MGR_ClearAllFlashEntries */


/* FUNCTION NAME: SYSLOG_MGR_LogUcFlashDbToLogFile
 * PURPOSE: Log the un-cleared flash database to file system if exist any entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   This function will be active in following 2 events.
 *          -- uc flash database is full.
 *          -- uc flash database is not stroed to file system excess 12 hours.
 *
 */
BOOL_T SYSLOG_MGR_LogUcFlashDbToLogFile(void)
{
    SYSLOG_MGR_Profile_T *profile_ar_p[SYSLOG_OM_MAX_NBR_OF_PROFILE];

    UI32_T  profile_id;
    UI32_T  merge_count;
    UI32_T  read_count;
    UI32_T  write_size_1 = 0;
    UI32_T  resv_size;
    UI32_T  file_size;
    UI32_T  i, j, k;
    BOOL_T  b_result;
    SYSLOG_OM_Header_T uc_flash_header;

    SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);

    if (uc_flash_header.count == 0)
    {
        log_up_time = 0;

        return TRUE;
    }

    /* Debug only.
     * Validates UC header
     */
        {
        UI32_T debug_count = uc_flash_header.count;

        for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
        {
            ASSERT(uc_flash_header.stat[profile_id].count <= debug_count);
            debug_count -= uc_flash_header.stat[profile_id].count;
        }

        ASSERT(0 == debug_count);
    }

    for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
        {
        profile_ar_p[profile_id] = &syslog_mgr_profile[profile_id];

#if (SYS_CPNT_SYSLOG_BACKUP == FALSE)
        ASSERT(1 == profile_ar_p[profile_id]->num_of_logfile);
#else
        ASSERT(2 == profile_ar_p[profile_id]->num_of_logfile);
#endif /* SYS_CPNT_SYSLOG_BACKUP */
        }

    for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
        {
        SYSLOG_MGR_Profile_T *profile_p = profile_ar_p[profile_id];

        if (0 == uc_flash_header.stat[profile_id].count)
        {
            continue;
        }

        b_result = SYSLOG_MGR_GetLastLogfileInfo(profile_p);

        ASSERT(TRUE == b_result);

        if (TRUE != b_result)
        {
            return FALSE;
        }

        ASSERT(profile_p->last_logfile < profile_p->num_of_logfile);

        file_size = profile_p->logfile_total_count * sizeof(profile_p->prepare_db_p->entry[0]) +
                    sizeof(profile_p->prepare_db_p->header);

        if (FS_ReadFile(DUMMY_DRIVE,
                        (UI8_T *)profile_p->logfile_ar[profile_p->last_logfile],
                        (UI8_T*)profile_p->prepare_db_p,
                        file_size,
                        &read_count) != FS_RETURN_OK)
    {
            ASSERT(0);

                log_up_time = 0;

                return FALSE;
            }

        if (0 != read_count)
            {
            ASSERT((read_count/SYSLOG_ADPT_LOG_ENTRY_LENGTH) == profile_p->logfile_total_count+1);

            if ( (read_count/SYSLOG_ADPT_LOG_ENTRY_LENGTH) != profile_p->logfile_total_count+1)
            printf("\r\nError: Count value and data is not matched!!");
        }

        for (i = uc_flash_header.front, j=0, k=profile_p->logfile_total_count;
             j<uc_flash_header.count;
             i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB, j++)
        {
            SYSLOG_OM_Record_T syslog_entry;

            SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);

            if (syslog_entry.profile_id != profile_id)
        {
                continue;
    }

            ASSERT(k < profile_p->max_entries_of_prepare_db);

            profile_p->prepare_db_p->entry[k++] = syslog_entry;
        }

        ASSERT((profile_p->logfile_total_count + uc_flash_header.stat[profile_id].count) == k);

        merge_count = profile_p->logfile_total_count + uc_flash_header.stat[profile_id].count;

        profile_p->prepare_db_p->header.count = (profile_p->logfile_total_count + uc_flash_header.stat[profile_id].count) < profile_p->max_entries_of_logfile ?
                                                (profile_p->logfile_total_count + uc_flash_header.stat[profile_id].count) :
                                                 profile_p->max_entries_of_logfile;

        profile_p->prepare_db_p->header.sequence_no = profile_p->last_sequence_no + 1;

        write_size_1 = profile_p->prepare_db_p->header.count * sizeof(profile_p->prepare_db_p->entry[0]) +
                       sizeof(profile_p->prepare_db_p->header);

        resv_size = profile_p->max_entries_of_logfile * sizeof(profile_p->prepare_db_p->entry[0]) +
                    sizeof(profile_p->prepare_db_p->header);

        if (merge_count <= profile_p->max_entries_of_logfile)
        {
            /* The merge number of flash file and uc_flash_db is less
             * than the max entry number of the flash file.
             */
            #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            SYS_TIME_DisableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

            if (FS_WriteFile(DUMMY_DRIVE,
                             (UI8_T *)profile_p->logfile_ar[profile_p->next_logfile],
                             (UI8_T *)"SysLog",
                             FS_FILE_TYPE_SYSLOG,
                             (UI8_T *) profile_p->prepare_db_p,
                             write_size_1,
                             resv_size) != FS_RETURN_OK)
            {
                log_up_time = 0;

                #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
                SYS_TIME_EnableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

                return FALSE;
            }

            #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            SYS_TIME_EnableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

        }
        else if (merge_count > profile_p->max_entries_of_logfile)
        {
            /* The merge number of flash file and uc_flash_db is grater
             * than the max entry number of the flash file.
             */
            UI32_T  header_1_index;

            header_1_index = merge_count - (profile_p->max_entries_of_logfile + 1);

            memcpy((UI8_T *)profile_p->prepare_db_p + (header_1_index+1) * sizeof(profile_p->prepare_db_p->entry[0]),
                   &profile_p->prepare_db_p->header,
                   sizeof(profile_p->prepare_db_p->header));

            #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            SYS_TIME_DisableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

            if (FS_WriteFile(DUMMY_DRIVE,
                             (UI8_T *)profile_p->logfile_ar[profile_p->next_logfile],
                             (UI8_T *)"SysLog",
                             FS_FILE_TYPE_SYSLOG,
                             (UI8_T *) profile_p->prepare_db_p + (header_1_index+1) * sizeof(profile_p->prepare_db_p->entry[0])/*SYSLOG_ADPT_LOG_ENTRY_LENGTH*/,
                             write_size_1,
                             resv_size) != FS_RETURN_OK)
            {
                log_up_time = 0;

                #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
                SYS_TIME_EnableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

                return FALSE;
            }

            #if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            SYS_TIME_EnableWatchDogTimer();
#endif /* SYS_CPNT_WATCHDOG_TIMER */

    }
    } /* for (profile_id */

    uc_flash_header.count = 0;
    memset(&uc_flash_header.stat, 0, sizeof(uc_flash_header.stat));

    uc_flash_header.front = uc_flash_header.rear;
    SYSLOG_OM_SetUcFlashHeader(&uc_flash_header);

    log_up_time = 0;

    return TRUE;
} /* End of SYSLOG_MGR_LogUcFlashDbToLogFile */


/* FUNCTION NAME: SYSLOG_MGR_NotifyProvisionComplete
 * PURPOSE: STKCTRL notify SYSLOG when STKCTRL know the provision complete..
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. This function only use by STKCTRL.
 *
 */
void SYSLOG_MGR_NotifyProvisionComplete(void)
{
#if (SYS_CPNT_SYSLOG_BACKUP == FALSE)
    SYSLOG_MGR_LogUcFlashDbToLogFile();
    return;
#else
    SYSLOG_MGR_RecoveryLogfileInFileSystem();
    SYSLOG_MGR_LogUcFlashDbToLogFile();
    return;
#endif
} /* End of SYSLOG_MGR_NotifyProvisionComplete */


/************************************************************************/
/* Aaron add the new code for cli to read the flash logfile information */
/************************************************************************/
/* FUNCTION NAME: SYSLOG_MGR_GetNextUcNormalEntries
 * PURPOSE: Get next log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T SYSLOG_MGR_GetNextUcNormalEntries(SYSLOG_MGR_Record_T *mgr_record)
{
    SYSLOG_OM_Header_T  uc_normal_header, uc_flash_header;
    SYSLOG_OM_Record_T  syslog_entry, first_normal_log_entry;
    UI32_T  i, j;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    /* Merge the UC flash/UC normal database together when get first information */
    if (mgr_record->entry_index == (UI32_T) -1)
    {
        uc_database.count = 0;
        SYSLOG_OM_GetUcNormalHeader(&uc_normal_header);
        SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);
        SYSLOG_OM_GetUcNormalEntry(uc_normal_header.front, &first_normal_log_entry);

        if ( (uc_normal_header.front == uc_normal_header.rear) && (uc_normal_header.count != 0) )
        {
            for (i = uc_flash_header.front, j = 0;
                 j < uc_flash_header.count;
                 i = (i + 1) % SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB, j ++)
            {
                SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);
                if (syslog_entry.log_time < first_normal_log_entry.log_time)
                {
                    uc_database.entry[j] = syslog_entry;
                    uc_database.count++;
                }
                else
                    break;
            }

            for (i = uc_normal_header.front, j=0; j<uc_normal_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
            {
                SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
                uc_database.entry[uc_database.count] = syslog_entry;
                uc_database.count++;
            }
        }
        else
        {
            /* copy from uc normal to database */
            uc_database.count = uc_normal_header.count;
            for (i = uc_normal_header.front, j=0; j<uc_normal_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
            {
                SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
                uc_database.entry[j] = syslog_entry; /* structure copy */
            }
        }
    }

    if (uc_database.count == 0)
    {
         //SYSLOG_MGR_GetNextUcNormalEntries
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_GET_NEXT_UC_NORMAL_ENTRIES_FUNC_NO,
            EH_TYPE_MSG_DEB_MSG, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_WARNING));
#endif
        return FALSE;
    }
    if (mgr_record->entry_index == (UI32_T) -1)
    {
        mgr_record->entry_index = (uc_database.count-1);
        mgr_record->owner_info = uc_database.entry[uc_database.count-1].owner_info;
        SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[uc_database.count-1].log_time, &mgr_record->rtc_time.year,
            &mgr_record->rtc_time.month, &mgr_record->rtc_time.day, &mgr_record->rtc_time.hour,
            &mgr_record->rtc_time.minute, &mgr_record->rtc_time.second);
        memcpy(mgr_record->message, uc_database.entry[uc_database.count-1].message, SYSLOG_ADPT_MESSAGE_LENGTH);

        return TRUE;
    }
    else if (mgr_record->entry_index > 0)
    {
        mgr_record->entry_index = (mgr_record->entry_index-1);
        mgr_record->owner_info = uc_database.entry[mgr_record->entry_index].owner_info;
        SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[mgr_record->entry_index].log_time, &mgr_record->rtc_time.year,
            &mgr_record->rtc_time.month, &mgr_record->rtc_time.day, &mgr_record->rtc_time.hour,
            &mgr_record->rtc_time.minute, &mgr_record->rtc_time.second);
        memcpy(mgr_record->message, uc_database.entry[mgr_record->entry_index].message, SYSLOG_ADPT_MESSAGE_LENGTH);

        return TRUE;
    }

    return FALSE;
} /* end of SYSLOG_MGR_GetNextUcNormalEntries */


/* FUNCTION NAME: SYSLOG_MGR_GetNextUcFlashEntry
 * PURPOSE: Get next log message from logfile in file system.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = (UI32_T) (-1) to get first entry.
 *          2. Use *record->entry_index = i to get index(i+1) entry.
 *          3. CLI will call this function.
 */
BOOL_T
SYSLOG_MGR_GetNextUcFlashEntry(
    SYSLOG_MGR_Record_T *mgr_record)
{
    SYSLOG_MGR_Profile_T *profile_ar_p[SYSLOG_OM_MAX_NBR_OF_PROFILE];
    SYSLOG_OM_Record_T  syslog_entry;
    UI32_T  read_count;
    UI32_T  profile_id;
    UI32_T  profile_entry_count;
    UI32_T  i, j;
    BOOL_T b_result;

    /* not in slave mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return TRUE;
    }

    for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
    {
        profile_ar_p[profile_id] = &syslog_mgr_profile[profile_id];

#if (SYS_CPNT_SYSLOG_BACKUP == FALSE)
        ASSERT(1 == profile_ar_p[profile_id]->num_of_logfile);
#else
        ASSERT(2 == profile_ar_p[profile_id]->num_of_logfile);
#endif /* SYS_CPNT_SYSLOG_BACKUP */
    }

    if (mgr_record->entry_index == (UI32_T) -1)
    {
        UI32_T file_size;
        UI32_T max_entries_of_logfile = 0;
        SYSLOG_OM_Header_T uc_flash_header;

        SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);

        /* Copy all records in uc_flash to uc_database
         */
        uc_database.count = 0;
        profile_entry_count = 0;

        for (i = uc_flash_header.front, j = 0;
             j < uc_flash_header.count;
             i = (i + 1) % SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB, j ++)
        {
                SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);
                uc_database.entry[j] = syslog_entry;
                uc_database.count++;
        }

        mgr_record->entry_index = 0;
        mgr_record->internal_max_log_index = 0;
        mgr_record->internal_log_index = 0;

        for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
        {
            SYSLOG_MGR_Profile_T *profile_p = profile_ar_p[profile_id];

            b_result = SYSLOG_MGR_GetLastLogfileInfo(profile_p);

            ASSERT(TRUE == b_result);

            if (TRUE != b_result)
            {
                return FALSE;
            }

            ASSERT(profile_p->last_logfile < profile_p->num_of_logfile);

            file_size = profile_p->logfile_total_count * sizeof(profile_p->prepare_db_p->entry[0]) +
                        sizeof(profile_p->prepare_db_p->header);

            if (FS_ReadFile(DUMMY_DRIVE,
                            (UI8_T *)profile_p->logfile_ar[profile_p->last_logfile],
                            (UI8_T*)profile_p->prepare_db_p,
                            file_size,
                            &read_count) != FS_RETURN_OK)
            {
                ASSERT(0);

                return FALSE;
            }

            if (0 == profile_p->prepare_db_p->header.count)
            {
                mgr_record->index_ar[profile_id].valid = FALSE;
                mgr_record->index_ar[profile_id].entry_index = 0xffffffff;
            }
            else
            {
                mgr_record->index_ar[profile_id].valid = TRUE;
                mgr_record->index_ar[profile_id].entry_index = profile_p->prepare_db_p->header.count - 1;
            }

            profile_entry_count = profile_entry_count + profile_p->prepare_db_p->header.count;
            max_entries_of_logfile += profile_p->max_entries_of_logfile;
        }

        mgr_record->internal_max_log_index = mgr_record->internal_log_index =
            profile_entry_count + uc_database.count;
        mgr_record->entry_index = (mgr_record->internal_max_log_index > max_entries_of_logfile) ?
            max_entries_of_logfile : mgr_record->internal_max_log_index;

        /* Get first record from UC flash entries.
         */
        if (uc_database.count > 0)
        {
            UI32_T entry_index = uc_database.count - 1;

            mgr_record->entry_index -= 1;
            mgr_record->internal_log_index -= 1;
            mgr_record->owner_info = uc_database.entry[entry_index].owner_info;
            SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[entry_index].log_time,
                                              &mgr_record->rtc_time.year,
                                              &mgr_record->rtc_time.month,
                                              &mgr_record->rtc_time.day,
                                              &mgr_record->rtc_time.hour,
                                              &mgr_record->rtc_time.minute,
                                              &mgr_record->rtc_time.second);
            memcpy(mgr_record->message,
                   uc_database.entry[entry_index].message,
                   SYSLOG_ADPT_MESSAGE_LENGTH);

            return TRUE;
        }
    }

    {
        UI32_T last_log_time = 0;
        UI32_T target_profile_id = SYSLOG_OM_MAX_NBR_OF_PROFILE;

        profile_entry_count = 0;

        for (profile_id = 0; profile_id < _countof(profile_ar_p); ++ profile_id)
        {
            SYSLOG_MGR_Profile_T *profile_p = profile_ar_p[profile_id];
            UI32_T idx;

            if (TRUE != mgr_record->index_ar[profile_id].valid)
            {
                continue;
            }

            idx = mgr_record->index_ar[profile_id].entry_index;

            if (last_log_time <= profile_p->prepare_db_p->entry[idx].log_time)
            {
                target_profile_id = profile_id;
                last_log_time = profile_p->prepare_db_p->entry[idx].log_time;
            }

            profile_entry_count = profile_entry_count + profile_ar_p[target_profile_id]->prepare_db_p->header.count;
        }

        /* Get next record from uc database entries.
         */
        if (mgr_record->internal_max_log_index - mgr_record->internal_log_index < uc_database.count)
        {
            UI32_T entry_index;

            entry_index = uc_database.count - (mgr_record->internal_max_log_index - mgr_record->internal_log_index) - 1;
            mgr_record->entry_index -= 1;
            mgr_record->internal_log_index -= 1;

            mgr_record->owner_info = uc_database.entry[entry_index].owner_info;
            SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[entry_index].log_time,
                                              &mgr_record->rtc_time.year,
                                              &mgr_record->rtc_time.month,
                                              &mgr_record->rtc_time.day,
                                              &mgr_record->rtc_time.hour,
                                              &mgr_record->rtc_time.minute,
                                              &mgr_record->rtc_time.second);
            memcpy(mgr_record->message,
                   uc_database.entry[entry_index].message,
                   SYSLOG_ADPT_MESSAGE_LENGTH);

            return TRUE;
        }

        /* No data
         */
        if (SYSLOG_OM_MAX_NBR_OF_PROFILE == target_profile_id)
        {
            return FALSE;
        }

        /* Get next record in flash file
         */
        {
            SYSLOG_MGR_Profile_T *profile_p;
            SYSLOG_OM_Record_T *record_p = NULL;
            UI32_T idx;

            ASSERT(target_profile_id < _countof(profile_ar_p));
            ASSERT(target_profile_id < _countof(mgr_record->index_ar));

            profile_p = profile_ar_p[target_profile_id];

            ASSERT(mgr_record->index_ar[target_profile_id].entry_index < profile_p->prepare_db_p->header.count);
            ASSERT(mgr_record->index_ar[target_profile_id].valid == TRUE);

            idx = mgr_record->index_ar[target_profile_id].entry_index;

            /* Data Occupancy
             */
            if (0 == mgr_record->entry_index)
            {
                return FALSE;
            }

            if (0 == idx)
            {
                mgr_record->index_ar[target_profile_id].valid = FALSE;
                mgr_record->index_ar[target_profile_id].entry_index = 0xffffffff;
            }
            else
            {
                mgr_record->index_ar[target_profile_id].entry_index -= 1;
            }

            mgr_record->entry_index -= 1;
            mgr_record->internal_log_index -= 1;

            record_p = &profile_p->prepare_db_p->entry[idx];

            mgr_record->owner_info = record_p->owner_info;

            SYS_TIME_ConvertSecondsToDateTime(record_p->log_time,
                                              &mgr_record->rtc_time.year,
                                              &mgr_record->rtc_time.month,
                                              &mgr_record->rtc_time.day,
                                              &mgr_record->rtc_time.hour,
                                              &mgr_record->rtc_time.minute,
                                              &mgr_record->rtc_time.second);

            memcpy(mgr_record->message, record_p->message, SYSLOG_ADPT_MESSAGE_LENGTH);
        }
    }

    return TRUE;
} /* End of SYSLOG_MGR_GetNextUcFlashEntry */


/* FUNCTION NAME: SYSLOG_MGR_LogColdStartEntry
 * PURPOSE: Log a cold start entry to uc-memory, and mark a RTC/SysUpTime information.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   SYSMGMT will call this function to notify SYSLOG cold start happen.
 *
 */
BOOL_T SYSLOG_MGR_LogColdStartEntry(void)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    return TRUE;
}


/* FUNCTION NAME: SYSLOG_MGR_LogWarmStartEntry
 * PURPOSE: Log a warm entry to uc-memory, and mark a RTC/SysUpTime information.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   SYSMGMT will call this function to notify SYSLOG warm start happen.
 *
 */
BOOL_T SYSLOG_MGR_LogWarmStartEntry(void)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    return TRUE;
}


/* FUNCTION NAME: SYSLOG_MGR_GetRunningSyslogStatus
 * PURPOSE: This function is used to get the non-default system log status.
 * INPUT:   *syslog_status -- syslog status output buffer.
 * OUTPUT:  *syslog_status -- syslog status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_STATUS_DISABLE.
 *
 */
UI32_T SYSLOG_MGR_GetRunningSyslogStatus(UI32_T *syslog_status)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    /* not in slave mode */

    SYSLOG_OM_GetSyslogStatus(syslog_status);


    if (*syslog_status == SYS_DFLT_SYSLOG_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of SYSLOG_MGR_GetRunningSyslogStatus */


/* FUNCTION NAME: SYSLOG_MGR_GetRunningUcLogLevel
 * PURPOSE: This function is used to get the non-default un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_LEVEL_DEBUG.
 *
 */
UI32_T SYSLOG_MGR_GetRunningUcLogLevel(UI32_T *uc_log_level)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    /* not in slave mode */

    SYSLOG_OM_GetUcLogLevel(uc_log_level);


    if (*uc_log_level == SYS_DFLT_SYSLOG_UC_LOG_LEVEL)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* End of SYSLOG_MGR_GetRunningUcLogLevel */


/* FUNCTION NAME: SYSLOG_MGR_GetRunningFlashLogLevel
 * PURPOSE: This function is used to get the non-default flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Default is SYSLOG_LEVEL_ERR.
 *
 */
UI32_T SYSLOG_MGR_GetRunningFlashLogLevel(UI32_T *flash_log_level)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    /* not in slave mode */

    SYSLOG_OM_GetFlashLogLevel(flash_log_level);


    if (*flash_log_level == SYS_DFLT_SYSLOG_FLASH_LOG_LEVEL)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* End of SYSLOG_MGR_GetRunningFlashLogLevel */

/* FUNCTION NAME: SYSLOG_MGR_ExistLogfileInFileSystem
 * PURPOSE: This function is check the syslog file exist in file system or not.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- exist.
 *          FALSE   -- not exist.
 * NOTES:
 *
 */
static BOOL_T
SYSLOG_MGR_ExistLogfileInFileSystem(
    void)
{
    UI8_T   buf[SYSLOG_ADPT_LOGFILE_HEADER_LENGTH];
    UI32_T  read_count = 0;
    UI32_T  result;

    result = FS_ReadFile(DUMMY_DRIVE,
                               (UI8_T *)SYSLOG_FILE_1,
                               buf,
                               SYSLOG_ADPT_LOGFILE_HEADER_LENGTH,
                               &read_count);

    if ((result == FS_RETURN_OK) ||
        (result == FS_RETURN_FILE_TRUNCATED))
    {
        return TRUE;
    }

#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)

    result = FS_ReadFile(DUMMY_DRIVE,
                               (UI8_T *)SYSLOG_FILE_2,
                               buf,
                               SYSLOG_ADPT_LOGFILE_HEADER_LENGTH,
                               &read_count);

    if ((result == FS_RETURN_OK) ||
        (result == FS_RETURN_FILE_TRUNCATED))
    {
        return TRUE;
    }

#endif

    return FALSE;
} /* End of SYSLOG_MGR_ExistLogfileInFileSystem */

/* FUNCTION NAME: SYSLOG_MGR_CreateLogfile
 * PURPOSE: Create syslog file in file system.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- create file.
 *          FALSE   -- create file failed.
 * NOTES:
 *
 */
static
BOOL_T SYSLOG_MGR_CreateLogfile()
{
    UI32_T profile_id;
    UI32_T i;
    UI32_T result;

    for (profile_id = 0; profile_id < _countof(syslog_mgr_profile); ++ profile_id)
    {
        SYSLOG_MGR_Profile_T *profile_p = &syslog_mgr_profile[profile_id];

        SYSLOG_OM_LogfileHeader_T header;

        for (i = 0; i < profile_p->num_of_logfile; ++ i)
        {
            UI32_T read_count;
            UI32_T resv_size;

            memset(&header, 0, sizeof(header));

            result = FS_ReadFile(DUMMY_DRIVE,
                                 (UI8_T*) profile_p->logfile_ar[i],
                                 (UI8_T *)&header,
                                 sizeof(header),
                                 &read_count);

            if (FS_RETURN_OK == result &&
                sizeof(header) == read_count &&
                sizeof(profile_p->prepare_db_p->header) == header.bytes_of_header &&
                sizeof(profile_p->prepare_db_p->entry[0]) == header.bytes_of_record)
            {
                ASSERT(0);
                continue;
            }

            /* Create / Rebuild logfile
             */
            memset(&header, 0, sizeof(header));

            header.bytes_of_header = sizeof( profile_p->prepare_db_p->header );
            header.bytes_of_record = sizeof( profile_p->prepare_db_p->entry[0] );

            resv_size = profile_p->max_entries_of_logfile * sizeof(profile_p->prepare_db_p->entry[0]) +
                        sizeof(profile_p->prepare_db_p->header);

            result = FS_WriteFile(DUMMY_DRIVE,
                                  (UI8_T*) profile_p->logfile_ar[i],
                                  (UI8_T*) "SysLog",
                                  FS_FILE_TYPE_SYSLOG,
                                  (UI8_T*) &header,
                                  sizeof(header),
                                  resv_size);

            if (FS_RETURN_OK != result)
            {
                ASSERT(0);
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* FUNCTION NAME: SYSLOG_MGR_GetLastLogfileInfo
 * PURPOSE: Get the last syslog file information of syslog profile.
 * INPUT:   none.
 * OUTPUT:  *profile_p    -- syslog profile.
 * RETUEN:  TRUE    -- exist.
 *          FALSE   -- not exist.
 * NOTES:
 *
 */
static BOOL_T
SYSLOG_MGR_GetLastLogfileInfo(
    SYSLOG_MGR_Profile_T *profile_p)
{
    SYSLOG_OM_LogfileHeader_T *header_ar;

    UI32_T i;
    UI32_T read_count;
    UI32_T result;

    UI32_T my_seq_no;

    ASSERT(NULL != profile_p);
    ASSERT(0 != profile_p->num_of_logfile);

    ASSERT(sizeof(*header_ar) == sizeof(SYSLOG_OM_LogfileHeader_T));

    header_ar = calloc(sizeof(*header_ar), profile_p->num_of_logfile);
    ASSERT(NULL != header_ar);

    if (NULL == header_ar)
    {
        return FALSE;
    }

    for (i = 0; i < profile_p->num_of_logfile; ++ i)
    {
        ASSERT(sizeof(*header_ar) == sizeof(SYSLOG_OM_LogfileHeader_T));

        result = FS_ReadFile(DUMMY_DRIVE,
                            (UI8_T *)profile_p->logfile_ar[i],
                            (UI8_T *)&header_ar[i],
                            sizeof(*header_ar),
                            &read_count);

        if (result != FS_RETURN_OK && result != FS_RETURN_FILE_TRUNCATED)
        {
            free(header_ar);
            header_ar = NULL;
            return FALSE;
        }
    }

    my_seq_no = 0;

    for (i = 0; i < profile_p->num_of_logfile; ++ i)
    {
        if (my_seq_no <= header_ar[i].sequence_no)
        {
            profile_p->last_logfile = i;
            profile_p->last_sequence_no = header_ar[i].sequence_no;
            profile_p->logfile_total_count = header_ar[i].count;
            profile_p->next_logfile = ((i+1) % profile_p->num_of_logfile);

            my_seq_no = header_ar[i].sequence_no;
        }
    }

    free(header_ar);
    header_ar = NULL;

    return TRUE;
} /* End of SYSLOG_MGR_GetLastLogfileInfo */


/* FUNCTION NAME: SYSLOG_MGR_TimerExpiry
 * PURPOSE: This routine will be called to check expiry or not.
 *          If Expiry, log entries from uc flash db to file system.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function be called per timer interval (30 sec).
 *
 */
void SYSLOG_MGR_TimerExpiry(void)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return;
    }

    /* not in slave mode */
    log_up_time = log_up_time + SYS_BLD_SYSLOG_LOOKUP_INTERVAL_TICKS;
    if (log_up_time >= (SYS_BLD_SYSLOG_MAX_UPDATE_LOGFILE_TIME_SEC * SYS_BLD_TICKS_PER_SECOND))
        SYSLOG_MGR_LogUcFlashDbToLogFile();

    return;
} /* End of SYSLOG_MGR_TimerExpiry */


#if (SYS_CPNT_SYSLOG_BACKUP == TRUE)
BOOL_T SYSLOG_MGR_RecoveryLogfileInFileSystem(void)
{
    UI32_T  read_count;
    /* SYSLOG_MGR_Prepare_T prepare_db; */


    if (FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_1, (UI8_T *)&prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        if (FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_2, (UI8_T *)&prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
        {

            return FALSE;
        }
        prepare_db.header.sequence_no = prepare_db.header.sequence_no + 1;
        if (FS_WriteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_1, (UI8_T *)"SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *) &prepare_db, (prepare_db.header.count+1)*SYSLOG_ADPT_LOG_ENTRY_LENGTH, SYSLOG_ADPT_LOGFILE_SIZE) != FS_RETURN_OK)
        {

            return FALSE;
        }

        return TRUE;
    }

    if (FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_2, (UI8_T *)&prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        if (FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_1, (UI8_T *)&prepare_db, SYSLOG_ADPT_LOGFILE_SIZE, &read_count) != FS_RETURN_OK)
        {

            return FALSE;
        }
        prepare_db.header.sequence_no = prepare_db.header.sequence_no + 1;
        if (FS_WriteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_FILE_2, (UI8_T *)"SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *) &prepare_db, (prepare_db.header.count+1)*SYSLOG_ADPT_LOG_ENTRY_LENGTH, SYSLOG_ADPT_LOGFILE_SIZE) != FS_RETURN_OK)
        {

            return FALSE;
        }

        return TRUE;
    }


    return TRUE;
} /* End of SYSLOG_MGR_RecoveryLogfileInFileSystem */
#endif

SYSLOG_MGR_Prepare_T *SYSLOG_MGR_GetPrepareDbPointer()
{
    return (&prepare_db);
}

SYSLOG_MGR_UcDatabase_T *SYSLOG_MGR_GetUcDatabasePointer()
{
    return (&uc_database);
}


/* FUNCTION NAME: SYSLOG_MGR_RegisterCallbackFunction
 * PURPOSE: Register Callback Function.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
static BOOL_T SYSLOG_MGR_RegisterCallbackFunction(void)
{
    /* Register a bacldoor engineer debug function */

    return TRUE;
} /*End of SYSLOG_MGR_RegisterCallbackFunction */


#if (SYS_CPNT_REMOTELOG == TRUE)
/* FUNCTION NAME: SYSLOG_MGR_BuildLogMsg
 * PURPOSE: Build syslog remote log message.
 * INPUT:   remotelog_entry -- log data
 *          server_config -- remote server
 * OUTPUT:  *buffer  -- pointer of message
 * RETUEN:
 * NOTES:
 */
static int
SYSLOG_MGR_BuildLogMsg(
    SYSLOG_OM_Remote_Record_T *remotelog_entry,
    SYSLOG_OM_Remote_Server_Config_T server_config,
    char *buffer)
{
    enum
    {
        SYSLOG_TIMESTAMP_BUFFER_LENGTH = sizeof("Mmm dd hh:mm:ss") - 1,
        SYSLOG_HOSTNAME_BUFFER_LENGTH = MAXSIZE_sysName,
        SYSLOG_PRIORITY_BUFFER_LENGTH = sizeof("<xxx>") - 1,
    };
    UI32_T  facility;
    UI32_T  priority;
    UI32_T  out_ifindex;
    UI32_T  module_no;
    int  len;
    int     year, month, day, hour, minute, second;
    char    pri_buffer[SYSLOG_PRIORITY_BUFFER_LENGTH + 1];
    char    time_buffer[SYSLOG_TIMESTAMP_BUFFER_LENGTH + 1];
    char    host_buffer[SYSLOG_HOSTNAME_BUFFER_LENGTH + 1];
    L_INET_AddrIp_T  src_ip, dst_ip, nexthop_ip;

    memset(pri_buffer, 0, sizeof (pri_buffer));
    memset(time_buffer, 0, sizeof (time_buffer));
    memset(host_buffer, 0, sizeof (host_buffer));
    memset(&dst_ip, 0x0, sizeof(dst_ip));
    memset(&src_ip, 0x0, sizeof(src_ip));

    memcpy(&dst_ip, &server_config.ipaddr, sizeof(dst_ip));

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    facility = server_config.facility;
#else
    SYSLOG_OM_GetRemoteLogFacility(&facility);
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */

    /* PRI: Priority, priority value equal facility multiply 8 plus severity
     */
    priority = (facility * 8) + remotelog_entry->owner_info.level;
    snprintf(pri_buffer, sizeof (pri_buffer), "<%3ld>", (long)priority);

   /* HEADER: TimeStamp
    */
    SYS_TIME_ConvertSecondsToDateTime(remotelog_entry->log_time, &year,
                                      &month, &day, &hour, &minute, &second);

    /* Time format : "Mmm dd hh:mm:ss"
       dd is the day of the month.
       If the day of the month is less than 10, then it MUST be represented as
       a space and then the number.
     */
    snprintf(time_buffer, sizeof (time_buffer), "%s %2d %02d:%02d:%02d",
             MONTH_MAPPING_STR[month - 1], day, hour, minute, second);

    /* HEADER:HOST
    */
    MIB2_POM_GetSysName((UI8_T *)host_buffer);

    if (!host_buffer[0])
    {
        /* get IP address
         */
        if (IPAL_RESULT_OK == IPAL_ROUTE_RouteLookup(&dst_ip, &src_ip, &nexthop_ip, &out_ifindex))
        {
            L_INET_InaddrToString((L_INET_Addr_T*)&src_ip, host_buffer,
                                  sizeof(host_buffer));
        }
    }

    module_no = (remotelog_entry->owner_info.module_no >= _countof(SYSLOG_MGR_MODULE_STRING_ARRAY))?
                 SYS_MODULE_UNKNOWN :
                 remotelog_entry->owner_info.module_no;

    /* log message format: "<priority>timestamp host-name module-name: message"
     */
    len = snprintf(buffer, SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE, "%s%s %s %s: %s",
                   pri_buffer, time_buffer, host_buffer,
                   SYSLOG_MGR_MODULE_STRING_ARRAY[module_no],
                   (char *)remotelog_entry->message);
    return len;

} /* End of SYSLOG_MGR_BuildLogMsg */

/* FUNCTION NAME: SYSLOG_MGR_SendLogPacket
 * PURPOSE: Send syslog remote log packet.
 * INPUT:   remotelog_entry -- log data
 * OUTPUT:  *buffer  -- pointer of message
 * RETUEN:
 * NOTES:
 */
static void
SYSLOG_MGR_SendLogPacket(
    SYSLOG_OM_Remote_Record_T *remotelog_entry,
    char *buffer)
{
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    UI32_T  level;
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */
    UI32_T  udp_port;
    int sockfd;
    int sockaddr_len = 0;
    int total_length;
    L_INET_AddrIp_T server_ipaddr;
    SYSLOG_MGR_Remote_Server_Config_T server_config;
    struct  sockaddr *saremote;
    struct  sockaddr_in sin;
    struct  sockaddr_in6 sin6;

    memset(&server_config, 0 ,sizeof(SYSLOG_MGR_Remote_Server_Config_T));

    while (SYSLOG_REMOTE_SUCCESS == SYSLOG_OM_GetNextRemoteLogServer(&server_config))
    {
        memset(&server_ipaddr, 0, sizeof(L_INET_AddrIp_T));
        memcpy(&server_ipaddr, &server_config.ipaddr, sizeof(server_ipaddr));

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
        level = server_config.level;

        if (remotelog_entry->owner_info.level > level)
        {
            continue;
        }
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */

        if (server_ipaddr.addrlen == 0)
        {
            continue;
        }

        udp_port = server_config.udp_port;

        switch(server_ipaddr.type)
        {
            case L_INET_ADDR_TYPE_IPV4:
            case L_INET_ADDR_TYPE_IPV4Z:
                memset(&sin, 0, sizeof(sin));
                sin.sin_family = AF_INET;
                sin.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_ANY);
                sin.sin_port = L_STDLIB_Hton16((unsigned short) udp_port);
                sockaddr_len = sizeof(struct sockaddr_in);
                saremote = (struct sockaddr *)&sin;
                break;

            case L_INET_ADDR_TYPE_IPV6:
            case L_INET_ADDR_TYPE_IPV6Z:
                memset(&sin6, 0, sizeof(sin6));
                sin6.sin6_family = AF_INET6;
                memcpy((void *)&sin6.sin6_addr, (void *)&in6addr_any, sizeof(sin6.sin6_addr));
                sin6.sin6_port = L_STDLIB_Hton16((unsigned short) udp_port);
                sockaddr_len = sizeof(struct sockaddr_in6);
                saremote = (struct sockaddr *)&sin6;
                break;

            default:
                continue;
        }

        sockfd = socket(saremote->sa_family, SOCK_DGRAM, 0);

        if (sockfd < 0)
        {
            continue;
        }

        if (bind(sockfd, saremote, sockaddr_len) < 0)
        {
            s_close (sockfd);
            continue;
        }

        if (FALSE == L_INET_InaddrToSockaddr(&server_ipaddr, udp_port, sockaddr_len, saremote))
        {
            s_close (sockfd);
            continue;
        }

        total_length = SYSLOG_MGR_BuildLogMsg(remotelog_entry, server_config, buffer);

        if (0 < total_length)
        {
            sendto (sockfd,  buffer, total_length,
                    (int) 0, saremote, sockaddr_len);
        }

        s_close (sockfd);
    }
}/* End of SYSLOG_MGR_SendLogPacket */

/* FUNCTION NAME: SYSLOG_MGR_AddFormatMsgEntry_Remote
 * PURPOSE: Add a log message to remote log module using format message.
 * INPUT:   *owner_info -- message information
 *          message_index -- message index
 *          *arg_0 -- argument 0 of message
 *          *arg_1 -- argument 1 of message
 *          *arg_2 -- argument 2 of message
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS -- OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL -- fail
 *          SYSLOG_REMOTE_INVALID -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   Call by SYSLOG_MGR_AddFormatMsgEntry only
 */
static UI32_T
SYSLOG_MGR_AddFormatMsgEntry_Remote(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2)
{
    UI32_T  log_time;
    UI32_T  status;
    UI32_T  level;
    SYSLOG_OM_Remote_Record_T remote_log_entry;
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    UI32_T  server_level;
    int     i;
    SYSLOG_MGR_Remote_Server_Config_T server_config;
#endif

    if (owner_info == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_MGR_ADD_FORMAT_MSG_ENTRY_REMOTE_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    SYSLOG_OM_GetRemoteLogStatus(&status);

    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog AddFormatEntry get status : %ld",(long)status);

    if (status != SYSLOG_STATUS_ENABLE)
    {

        return SYSLOG_REMOTE_INVALID;
    }

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    level = 0;

    for(i = 0; i < SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER; i ++)
    {
        SYSLOG_OM_GetRemoteLogServerByIndex(&server_config, i);

        if ((server_level >= SYSLOG_LEVEL_EMERG) || (server_level <= SYSLOG_LEVEL_DEBUG))
        {
            server_level = server_config.level;

            if (level < server_level)
            {
                level = server_level;
            }
        }
    }

#else
    SYSLOG_OM_GetRemoteLogLevel(&level);
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */

    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog AddFormatEntry get level : %ld",(long)level);

    if((level < SYSLOG_LEVEL_EMERG) ||
       (level > SYSLOG_LEVEL_DEBUG) ||
       (owner_info->level > level) ||
       (owner_info->module_no >= SYS_MODULE_UNKNOWN))
    {

        return SYSLOG_REMOTE_INVALID;
    }

    remote_log_entry.owner_info.level = owner_info->level;
    remote_log_entry.owner_info.module_no = owner_info->module_no;
    remote_log_entry.owner_info.function_no = owner_info->function_no;
    remote_log_entry.owner_info.error_no = owner_info->error_no;

    if (FALSE == SYSLOG_MGR_FormatMessage(message_index, arg_0, arg_1, arg_2,
                                          (char *)remote_log_entry.message, sizeof(remote_log_entry.message)))
    {
        return SYSLOG_REMOTE_INVALID;
    }

    SYS_TIME_GetRealTimeBySec(&log_time);
    remote_log_entry.log_time = log_time;

    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message level : %d",remote_log_entry.owner_info.level);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message module_no : %lu",(unsigned long)remote_log_entry.owner_info.module_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message function_no : %d",remote_log_entry.owner_info.function_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message error_no : %d",remote_log_entry.owner_info.error_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message log_time : %ld",(long)remote_log_entry.log_time);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message : %s",remote_log_entry.message);

    {
        SYSLOG_EVENT_SyslogData_T remotelog_data;

        memcpy(&(remotelog_data.remotelog_entry), &remote_log_entry, sizeof(remotelog_data.remotelog_entry));
        SYSLOG_MGR_QueueEnqueue(&remotelog_data);
        SYSFUN_SendEvent(syslog_task_id, SYSLOG_ADPT_EVENT_TRAP_ARRIVAL);
    }

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_MGR_AddFormatMsgEntry_Remote */


/* FUNCTION NAME: SYSLOG_MGR_AddEntry_Remote
 * PURPOSE: Add a log message to remote log module.
 * INPUT:   *log_entry   -- add this log entry
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS -- OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL -- fail
 *          SYSLOG_REMOTE_INVALID -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:
 *
 */
static
UI32_T SYSLOG_MGR_AddEntry_Remote(
    SYSLOG_OM_Remote_Record_T *log_entry)
{
    UI32_T  log_time;
    UI32_T  status;
    UI32_T  level = 0;
    UI32_T  ret;
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    UI32_T  server_level;
    int     i;
    SYSLOG_MGR_Remote_Server_Config_T server_config;
#endif

    ret = SYSLOG_REMOTE_INVALID;

    if (log_entry == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_MGR_ADD_ENTRY_REMOTE_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    SYSLOG_OM_GetRemoteLogStatus(&status);

    if (status != SYSLOG_STATUS_ENABLE)
    {

        return SYSLOG_REMOTE_INVALID;
    }

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)

    for(i = 0; i < SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER; i ++)
    {
        SYSLOG_OM_GetRemoteLogServerByIndex(&server_config, i);
        server_level = server_config.level;

        if (level < server_level)
        {
            level = server_level;
        }
    }

#else
    SYSLOG_OM_GetRemoteLogLevel(&level);
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */

    if (log_entry->owner_info.level > level)
    {

        return SYSLOG_REMOTE_INVALID;
    }

    if (log_entry->owner_info.module_no >= SYS_MODULE_UNKNOWN)
    {

        return SYSLOG_REMOTE_INVALID;
    }

    SYS_TIME_GetRealTimeBySec(&log_time);
    log_entry->log_time = log_time;

    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message level : %d",log_entry->owner_info.level);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message module_no : %lu",(unsigned long)log_entry->owner_info.module_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message function_no : %d",log_entry->owner_info.function_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message error_no : %d",log_entry->owner_info.error_no);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message log_time : %ld",(long)log_entry->log_time);
    SYSLOG_MGR_DEBUG_LOG("\r\nRemotelog MGR receive message : %s",log_entry->message);

    {
        SYSLOG_EVENT_SyslogData_T remotelog_data;

        memcpy(&(remotelog_data.remotelog_entry), &log_entry, sizeof(remotelog_data.remotelog_entry));
        SYSLOG_MGR_QueueEnqueue(&remotelog_data);
        SYSFUN_SendEvent(syslog_task_id, SYSLOG_ADPT_EVENT_TRAP_ARRIVAL);
    }

    return ret;
} /* End of SYSLOG_MGR_AddEntry_Remote */

/* FUNCTION NAME: SYSLOG_MGR_CreateRemoteLogServer
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_CreateRemoteLogServer(
    L_INET_AddrIp_T *ip_address)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    /* not in slave mode
     */
    if(SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS)
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_CreateRemoteLogServer(ip_address);

    return ret;
}/* End of SYSLOG_MGR_CreateRemoteLogServer */

/* FUNCTION NAME: SYSLOG_MGR_DeleteRemoteLogServer
 * PURPOSE: This function is used to delete the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_DeleteRemoteLogServer(
    L_INET_AddrIp_T *ip_address)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYSLOG_REMOTE_FAIL;
    }

    /* not in slave mode
     */
    if(SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS)
    {

        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_DeleteRemoteLogServer(ip_address);

    return ret;
}/* End of SYSLOG_MGR_DeleteRemoteLogServer */

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerPort
 * PURPOSE: This function is used to add the server ip address and port.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerPort(
    L_INET_AddrIp_T * ip_address,
    UI32_T port)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    /* not in slave mode
     */
    if(SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS)
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    if((port < SYSLOG_ADPT_MIN_REMOTE_HOST_UDP_PORT) ||
       (port > SYSLOG_ADPT_MAX_REMOTE_HOST_UDP_PORT))
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_SetRemoteLogServerPort(ip_address, port);

    return ret;
}/* End of SYSLOG_MGR_SetRemoteLogServerPort */

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogServerPort
 * PURPOSE: This function is used to get the port number of the input server
 *          ip address .
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  *port_p    -- port number of the input server ip address
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_GetRemoteLogServerPort(
    L_INET_AddrIp_T *ip_address,
    UI32_T *port_p)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    /* not in slave mode
     */
    if(SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS)
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_GetRemoteLogServerPort(ip_address,port_p);

    return ret;
}/* End of SYSLOG_MGR_GetRemoteLogServerPort */

/* FUNCTION NAME: SYSLOG_MGR_DeleteAllRemoteLogServer
 * PURPOSE: This function is used to delete all server.
 * INPUT:
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_DeleteAllRemoteLogServer()
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    ret = SYSLOG_OM_DeleteAllRemoteLogServer();

    return ret;
}/* End of SYSLOG_MGR_DeleteAllRemoteLogServer */

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogStatus
 * PURPOSE: This function is used to enable/disable REMOTELOG.
 * INPUT:   status -- remotelog status.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 *
 */
UI32_T
SYSLOG_MGR_SetRemoteLogStatus(
    UI32_T status)
{
    UI32_T  om_status;
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    /* not in slave mode
     */
    if (((int)status != SYSLOG_STATUS_ENABLE) && ((int)status != SYSLOG_STATUS_DISABLE))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "Remote Log status");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_MGR_SET_REMOTELOG_STATUS_FUNC_NO,
                                 EH_TYPE_MSG_IS_INVALID,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 buff1);
#endif

        return SYSLOG_REMOTE_INVALID;
    }

    SYSLOG_OM_GetRemoteLogStatus(&om_status);

    if (status != om_status)
    {
        ret = SYSLOG_OM_SetRemoteLogStatus(status);

        return ret;
    }

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_MGR_SetRemoteLogStatus */

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogStatus
 * PURPOSE: This function is used to get remotelog status.
 * INPUT:   *status -- output buffer of remotelog status.
 * OUTPUT:  *status -- remotelog status.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 */
UI32_T
SYSLOG_MGR_GetRemoteLogStatus(
    UI32_T *status)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    if (status == NULL)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    /* not in slave mode
     */
    ret = SYSLOG_OM_GetRemoteLogStatus(status);

    return ret;
}/* End of SYSLOG_MGR_GetRemoteLogStatus */

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemoteLogStatus
 * PURPOSE: This function is used to get running remotelog status.
 * INPUT:   *status -- output buffer of remotelog status.
 * OUTPUT:  *status -- remotelog status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. status is defined as following:
 *             SYSLOG_STATUS_ENABLE
 *             REMOTELOG_STATUS_DISABLE
 *          2. default status is SYSLOG_STATUS_ENABLE
 */
UI32_T
SYSLOG_MGR_GetRunningRemoteLogStatus(
    UI32_T *status)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (status == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* not in slave mode
     */
    ret = SYSLOG_OM_GetRemoteLogStatus(status);

    if (ret != SYSLOG_REMOTE_SUCCESS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*status == SYS_DFLT_REMOTELOG_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SYSLOG_MGR_GetRunningRemoteLogStatus */

/* FUNCTION NAME: SYSLOG_MGR_IsValidIPAddress
 * PURPOSE: This function is used to check if ip address is valid.
 * INPUT:   ip_addr -- ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 */
static UI32_T
SYSLOG_MGR_IsValidIPAddress(
    L_INET_AddrIp_T *ip_addr)
{
    BOOL_T  is_valid = TRUE;

    if(ip_addr == NULL)
    {
        return FALSE;
    }

    switch(ip_addr->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp(ip_addr->addr))
            {
                is_valid = FALSE;
            }
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            /* IPv6 address should not be
             * IP_LIB_INVALID_IPV6_UNSPECIFIED
             * IP_LIB_INVALID_IPV6_LOOPBACK
             * IP_LIB_INVALID_IPV6_MULTICAST
             */
            if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface(ip_addr->addr, SYS_ADPT_IPV6_ADDR_LEN))
            {
                is_valid = FALSE;
            }
        }
            break;

        default:
            is_valid = FALSE;
    }

    if (FALSE == is_valid)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[48] = {0};

        sprintf((char *)buff1, "remote log server IP address");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_MGR_VALID_IP_FUNC_NO,
                                 EH_TYPE_MSG_INVALID,
                                 (SYSLOG_LEVEL_INFO),
                                 buff1);
#endif

        return SYSLOG_REMOTE_INVALID;
    }
    else
    {

        return SYSLOG_REMOTE_SUCCESS;
    }
}/* End of SYSLOG_MGR_IsValidIPAddress */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_RifUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: this is a callBack function, for Rif to Notify syslog when
 *          Rif get IP sucessfully
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
UI32_T SYSLOG_MGR_RifUp_CallBack(UI32_T ip_address, UI32_T ip_mask)
{
    UI32_T ret;

    ret = SYSFUN_SendEvent(syslog_task_id, SYSLOG_ADPT_EVENT_RIF_UP);

    return ret;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_RifDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: this is a callBack function, for Rif to Notify syslog when
 *          Rif get IP sucessfully
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
UI32_T
SYSLOG_MGR_RifDown_CallBack(
    UI32_T ip_address,
    UI32_T ip_mask)
{
    UI32_T ret;

    ret = SYSFUN_SendEvent(syslog_task_id, SYSLOG_ADPT_EVENT_RIF_DOWN);

    return ret;
}

/* FUNCTION NAME: SYSLOG_MGR_QueueEnqueue
 * PURPOSE: This function is used to enqueue remotelog entry.
 * INPUT:   *p -- remotelog event data pointer.
 *          *q -- remotelog queue pointer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Max queue number is SYSLOG_MGR_MAX_QUE_NBR
 */
static void
SYSLOG_MGR_QueueEnqueue(
    SYSLOG_EVENT_SyslogData_T *p)
{
    /* LOCAL VARIABLE DECLARATIONS
     */

    SYSLOG_EVENT_SyslogData_T             *new_blk;

    /* BODY
     */

    if (p == NULL)
    {
        return;
    } /* End of if */

    {
        UI16_T  userid = L_MM_USER_ID2(SYS_MODULE_SYSLOG, SYSLOG_TYPE_TRACE_ID_SYSLOG_MGR_QUEUEENQUEUE);
        new_blk = (SYSLOG_EVENT_SyslogData_T *)L_MM_Malloc(sizeof(SYSLOG_EVENT_SyslogData_T), userid);
    }

    if (new_blk == NULL)
    {
        return;
    } /* End of if */

    memset(new_blk, 0, sizeof (*new_blk));
    memcpy((void *)&new_blk->remotelog_entry, (void *)&p->remotelog_entry, sizeof (new_blk->remotelog_entry));

    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_OM_QueueEnqueue(new_blk))
    {
        L_MM_Free(new_blk);
    }

    return;
}/* End of SYSLOG_MGR_QueueEnqueue() */

/* FUNCTION NAME: SYSLOG_MGR_QueueDequeue
 * PURPOSE: This function is used to dequeue remotelog entry.
 * INPUT:   *remotelog_queue -- remotelog queue.
 * OUTPUT:  None.
 * RETUEN:  remotelog event data
 * NOTES:   None.
 */
static SYSLOG_EVENT_SyslogData_T
*SYSLOG_MGR_QueueDequeue()
{

    return SYSLOG_OM_QueueDequeue();
}/* End of SYSLOG_MGR_QueueDequeue() */

/* FUNCTION NAME: SYSLOG_MGR_HandleTrapQueue
 * PURPOSE: This function is used to handle remotelog trap queue.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 */
void
SYSLOG_MGR_HandleTrapQueue(
    void)
{
    char *remote_packet_buffer = NULL;
    UI16_T userid;
    SYSLOG_EVENT_SyslogData_T * remotelog_data       = NULL;

    userid = L_MM_USER_ID2(SYS_MODULE_SYSLOG, SYSLOG_TYPE_TRACE_ID_SYSLOG_MGR_HANDLETRAPQUEUE);
    remote_packet_buffer = (char *)L_MM_Malloc(SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE, userid );

    if (remote_packet_buffer == NULL)
    {
        return;
    }

    while ((remotelog_data = SYSLOG_MGR_QueueDequeue()) != NULL)
    {
        /* Initiate message buffer
         */
        remote_packet_buffer[0] = 0;

        SYSLOG_MGR_SendLogPacket(&remotelog_data->remotelog_entry, remote_packet_buffer);
        L_MM_Free((void *)remotelog_data);

        remotelog_data = NULL;

        /* Need to add delay between sending traps.
         * Otherwise, just after start-up, some trap will not be received by the PC.
         */
        SYSFUN_Sleep(SYS_BLD_TICKS_PER_SECOND/10);
    }

    L_MM_Free((void *)remote_packet_buffer);

    return;
} /* end of SYSLOG_MGR_HandleTrapQueue() */

/* FUNCTION NAME: SYSLOG_MGR_SetTaskId
 * PURPOSE: This function is used to keep syslog task id for remotelog send event use.
 * INPUT:   task_id -- task id
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 */
void
SYSLOG_MGR_SetTaskId(
    UI32_T task_id)
{
    syslog_task_id = task_id;

    return;
}/* end of SYSLOG_MGR_SetTaskId() */

/* FUNCTION NAME: SYSLOG_MGR_GetStaState
 * PURPOSE: This function is used to get sta state.
 * INPUT:   *sta_state -- sta state
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None.
 */
void
SYSLOG_MGR_GetStaState(
    UI32_T *sta_state)
{
    *sta_state = spanning_tree_state;

    return;
}/* end of SYSLOG_MGR_GetStaState() */

/* FUNCTION NAME: SYSLOG_MGR_SetStaState
 * PURPOSE: This function is used to set sta state.
 * INPUT:   *sta_state -- sta state
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   This API is for syslog task use only
 */
void
SYSLOG_MGR_SetStaState(
    UI32_T sta_state)
{
    spanning_tree_state = sta_state;

    return;
}/* end of SYSLOG_MGR_SetStaState() */

/* FUNCTION NAME: SYSLOG_MGR_IsQueueEmpty
 * PURPOSE: This function is used to check if queue is empty.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  TRUE -- Queue is empty
 *          FALSE -- Queue is not empty
 * NOTES:   None.
 */
BOOL_T
SYSLOG_MGR_IsQueueEmpty(
    void)
{

    return SYSLOG_OM_IsQueueEmpty();
}/* end of SYSLOG_MGR_IsQueueEmpty() */

/* FUNCTION NAME: SYSLOG_MGR_GetNextRemoteLogServer
 * PURPOSE: This function is used to get the server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *          2. get all server ip address one time
 *          3. index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetNextRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    /* not in slave mode
     */
    if ((server_config->ipaddr.addr == NULL))
    {
        return   SYSLOG_REMOTE_INVALID_BUFFER;
    }

    if((SYSLOG_MGR_IsValidIPAddress(&server_config->ipaddr) != SYSLOG_REMOTE_SUCCESS) &&
       (server_config->ipaddr.addrlen != 0))
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_GetNextRemoteLogServer(server_config);

    return ret;
}/* End of SYSLOG_MGR_GetNextRemoteLogServer */

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogServer
 * PURPOSE: This function is used to get the server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 *          2. get all server ip address one time
 *          3. index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config)
{
    UI32_T  ret;
    UI32_T  index;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    /* not in slave mode
     */
    if ((server_config->ipaddr.addr == NULL))
    {
        return   SYSLOG_REMOTE_INVALID_BUFFER;
    }

    if(SYSLOG_MGR_IsValidIPAddress(&server_config->ipaddr) != SYSLOG_REMOTE_SUCCESS)
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_GetRemoteLogServer(server_config, &index);

    return ret;
}/* End of SYSLOG_MGR_GetRemoteLogServer */

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemoteLogServer
 * PURPOSE: This function is used to get the running server ip address.
 * INPUT:   *ip_address -- output buffer of server ip address.
 *          index       -- index of server ip address.
 * OUTPUT:  *ip_address -- server ip address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   index value from 0 to SYS_ADPT_REMOTELOG_MAX_SERVER_NBR-1
 *
 */
UI32_T
SYSLOG_MGR_GetRunningRemoteLogServer(
    SYSLOG_MGR_Remote_Server_Config_T *server_config,
    UI32_T index)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(server_config == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    ret = SYSLOG_OM_GetRemoteLogServerByIndex(server_config, index);

    if (ret != SYSLOG_REMOTE_SUCCESS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (server_config->ipaddr.addrlen == 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SYSLOG_MGR_GetRunningRemoteLogServer */

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerFacility
 * PURPOSE: This function is used to set the facility of input host.
 * INPUT:   ip_address -- server ip address.
            facility   -- facility level
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerFacility(
    L_INET_AddrIp_T * ip_address,
    UI32_T facility)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    /* not in slave mode
    */
    if(SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS)
    {
        return SYSLOG_REMOTE_INVALID;
    }

    if ((facility < SYSLOG_REMOTE_FACILITY_LOCAL0) ||
        (facility > SYSLOG_REMOTE_FACILITY_LOCAL7))
    {
        return SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_SetRemoteLogServerFacility(ip_address, facility);

    return ret;
}/* SYSLOG_MGR_SetRemoteLogServerFacility */

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogServerLevel
 * PURPOSE: This function is used to add the server ip address.
 * INPUT:   ip_address -- server ip address.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. Max number of server ip address is 5.
 */
UI32_T
SYSLOG_MGR_SetRemoteLogServerLevel(
    L_INET_AddrIp_T * ip_address,
    UI32_T level)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_SUCCESS;
    }

    if((SYSLOG_MGR_IsValidIPAddress(ip_address) != SYSLOG_REMOTE_SUCCESS) ||
       (level < SYSLOG_LEVEL_EMERG ) ||
       (level > SYSLOG_LEVEL_DEBUG ))
    {
        return   SYSLOG_REMOTE_INVALID;
    }

    ret = SYSLOG_OM_SetRemoteLogServerLevel(ip_address, level);

    return ret;
}/* End of SYSLOG_MGR_SetRemoteLogServerLevel */

#else
/* FUNCTION NAME: SYSLOG_MGR_GetRunningFacilityType
 * PURPOSE: This function is used to get running facility type.
 * INPUT:   *facility -- output buffer of facility type.
 * OUTPUT:  *facility -- facility type
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. facility type is defined as following:
 *             SYSLOG_REMOTE_FACILITY_LOCAL0      = 16;
 *             SYSLOG_REMOTE_FACILITY_LOCAL1      = 17;
 *             SYSLOG_REMOTE_FACILITY_LOCAL2      = 18;
 *             SYSLOG_REMOTE_FACILITY_LOCAL3      = 19;
 *             SYSLOG_REMOTE_FACILITY_LOCAL4      = 20;
 *             SYSLOG_REMOTE_FACILITY_LOCAL5      = 21;
 *             SYSLOG_REMOTE_FACILITY_LOCAL6      = 22;
 *             SYSLOG_REMOTE_FACILITY_LOCAL7      = 23;
 *          2. default is SYSLOG_REMOTE_FACILITY_LOCAL7
 */
UI32_T
SYSLOG_MGR_GetRunningFacilityType(
    UI32_T *facility)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(facility == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* not in slave mode
     */
    ret = SYSLOG_OM_GetRemoteLogFacility(facility);

    if (ret != SYSLOG_REMOTE_SUCCESS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*facility == SYS_DFLT_REMOTELOG_FACILITY_TYPE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SYSLOG_MGR_GetRunningFacilityType */

/* FUNCTION NAME: SYSLOG_MGR_GetRunningRemotelogLevel
 * PURPOSE: This function is used to get running remotelog level.
 * INPUT:   *level -- output buffer of remotelog level.
 * OUTPUT:  *level -- remotelog level.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. remotelog level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 */
UI32_T
SYSLOG_MGR_GetRunningRemotelogLevel(
    UI32_T *level)
{
    UI32_T  ret;

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(level == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* not in slave mode
     */
    ret = SYSLOG_OM_GetRemoteLogLevel(level);

    if(ret != SYSLOG_REMOTE_SUCCESS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*level == SYS_DFLT_REMOTELOG_LEVEL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SYSLOG_MGR_GetRunningRemotelogLevel */

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogFacility
 * PURPOSE: This function is used to set facility type.
 * INPUT:   facility -- facility type.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. facility type is defined as following:
 *             SYSLOG_REMOTE_FACILITY_LOCAL0      = 16;
 *             SYSLOG_REMOTE_FACILITY_LOCAL1      = 17;
 *             SYSLOG_REMOTE_FACILITY_LOCAL2      = 18;
 *             SYSLOG_REMOTE_FACILITY_LOCAL3      = 19;
 *             SYSLOG_REMOTE_FACILITY_LOCAL4      = 20;
 *             SYSLOG_REMOTE_FACILITY_LOCAL5      = 21;
 *             SYSLOG_REMOTE_FACILITY_LOCAL6      = 22;
 *             SYSLOG_REMOTE_FACILITY_LOCAL7      = 23;
 *          2. default is SYSLOG_REMOTE_FACILITY_LOCAL7
 */
UI32_T
SYSLOG_MGR_SetRemoteLogFacility(
    UI32_T facility)
{
    UI32_T  om_facility;
    UI32_T  ret;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    if ((facility < SYSLOG_REMOTE_FACILITY_LOCAL0) ||
        (facility > SYSLOG_REMOTE_FACILITY_LOCAL7))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "The input facility type: %lu", facility);
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_MGR_SET_FACILITY_TYPE_FUNC_NO,
                                 EH_TYPE_MSG_IS_INVALID,
                                 (SYSLOG_LEVEL_INFO),
                                 buff1);
#endif

        return SYSLOG_REMOTE_INVALID;
    }

    SYSLOG_OM_GetRemoteLogFacility(&om_facility);

    if (facility != om_facility)
    {
        ret = SYSLOG_OM_SetRemoteLogFacility(facility);

        return ret;
    }

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_MGR_SetRemoteLogFacility */

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogFacility
 * PURPOSE: This function is used to get facility type.
 * INPUT:   *facility_p -- output buffer of facility type.
 * OUTPUT:  *facility_p -- facility type
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. facility type is defined as following:
 *             SYSLOG_REMOTE_FACILITY_LOCAL0      = 16;
 *             SYSLOG_REMOTE_FACILITY_LOCAL1      = 17;
 *             SYSLOG_REMOTE_FACILITY_LOCAL2      = 18;
 *             SYSLOG_REMOTE_FACILITY_LOCAL3      = 19;
 *             SYSLOG_REMOTE_FACILITY_LOCAL4      = 20;
 *             SYSLOG_REMOTE_FACILITY_LOCAL5      = 21;
 *             SYSLOG_REMOTE_FACILITY_LOCAL6      = 22;
 *             SYSLOG_REMOTE_FACILITY_LOCAL7      = 23;
 *          2. default is SYSLOG_REMOTE_FACILITY_LOCAL7
 *
 */
UI32_T
SYSLOG_MGR_GetRemoteLogFacility(
    UI32_T *facility_p)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    if(facility_p == NULL)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    ret = SYSLOG_OM_GetRemoteLogFacility(facility_p);

    return ret;
}/* End of SYSLOG_MGR_GetRemoteLogFacility */

/* FUNCTION NAME: SYSLOG_MGR_SetRemoteLogLevel
 * PURPOSE: This function is used to set remotelog level.
 * INPUT:   level -- remotelog level.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. remotelog level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 */
UI32_T
SYSLOG_MGR_SetRemoteLogLevel(
    UI32_T level)
{
    UI32_T  om_level;
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    if ((level < SYSLOG_LEVEL_EMERG) || (level > SYSLOG_LEVEL_DEBUG))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf((char *)buff1, "Remote log level (0-7): ");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_MGR_SET_REMOTELOG_LEVEL_FUNC_NO,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (SYSLOG_LEVEL_INFO), buff1);
#endif

        return SYSLOG_REMOTE_INVALID;
    }

    SYSLOG_OM_GetRemoteLogLevel(&om_level);

    if (level != om_level)
    {
        ret = SYSLOG_OM_SetRemoteLogLevel(level);

        return ret;
    }

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_MGR_SetRemoteLogLevel */

/* FUNCTION NAME: SYSLOG_MGR_GetRemoteLogLevel
 * PURPOSE: This function is used to get remotelog level.
 * INPUT:   *level_p -- output buffer of remotelog level.
 * OUTPUT:  *level_p -- remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    -- success
 *          SYSLOG_REMOTE_FAIL    -- fail
 *          SYSLOG_REMOTE_INVALID  -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER   -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST  -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL   -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. remotelog level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 */
UI32_T
SYSLOG_MGR_GetRemoteLogLevel(
    UI32_T *level_p)
{
    UI32_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    if(level_p == NULL)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    ret = SYSLOG_OM_GetRemoteLogLevel(level_p);

    return ret;
}/* End of SYSLOG_MGR_GetRemoteLogLevel */
#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif /* end (SYS_CPNT_REMOTELOG == TRUE) */

#if (SYS_CPNT_SMTP == TRUE)
/* FUNCTION NAME: SYSLOG_MGR_AddFormatMsgEntry_Smtp
 * PURPOSE: Add a log message to remote log module using format message.
 * INPUT:   *owner_info -- message information
 *          message_index -- message index
 *          *arg_0 -- argument 0 of message
 *          *arg_1 -- argument 1 of message
 *          *arg_2 -- argument 2 of message
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS -- OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL -- fail
 *          SYSLOG_REMOTE_INVALID -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   Call by SYSLOG_MGR_AddFormatMsgEntry only
 *
 */
static BOOL_T
SYSLOG_MGR_AddFormatMsgEntry_Smtp(
    SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    void    *arg_0,
    void    *arg_1,
    void    *arg_2)
{
    UI32_T  log_time;
    UI32_T  status;
    UI32_T  level;
    SMTP_OM_Record_T smtp_entry;

    if (owner_info == 0)
        return FALSE;

    SMTP_MGR_GetSmtpAdminStatus(&status);

    if (status != SMTP_STATUS_ENABLE)
    {

        return FALSE;
    }

    SMTP_MGR_GetEmailSeverityLevel(&level);

    if (owner_info->level > level)
    {

        return FALSE;
    }

    if (owner_info->module_no >= SYS_MODULE_UNKNOWN)
    {
        return FALSE;
    }

    memset(&smtp_entry, 0, sizeof(smtp_entry));
    smtp_entry.owner_info.level = owner_info->level;
    smtp_entry.owner_info.module_no = owner_info->module_no;
    smtp_entry.owner_info.function_no = owner_info->function_no;
    smtp_entry.owner_info.error_no = owner_info->error_no;

    if (FALSE == SYSLOG_MGR_FormatMessage(message_index, arg_0, arg_1, arg_2,
                                          (char *)smtp_entry.message, sizeof(smtp_entry.message)))
    {
        return FALSE;
    }

    SYS_TIME_GetRealTimeBySec(&log_time);
    smtp_entry.log_time = log_time;

    SMTP_MGR_SendMail(&smtp_entry);

    return TRUE;
} /* End of SYSLOG_MGR_AddFormatMsgEntry_Smtp */


/* FUNCTION NAME: SYSLOG_MGR_AddEntry_Smtp
 * PURPOSE: Add a log message to smtp log module.
 * INPUT:   *log_entry   -- add this log entry
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS -- OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL -- fail
 *          SYSLOG_REMOTE_INVALID -- invalid input value
 *          SYSLOG_REMOTE_INVALID_BUFFER -- invalid input buffer
 *          SYSLOG_REMOTE_INPUT_EXIST -- input value already exist
 *          SYSLOG_REMOTE_CREATE_SOCKET_FAIL -- create socket fail
 *          SYSLOG_REMOTE_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:
 *
 */
static BOOL_T SYSLOG_MGR_AddEntry_Smtp(SYSLOG_OM_Record_T *log_entry)
{
    UI32_T  log_time;
    UI32_T  status;
    UI32_T  level;
    SMTP_OM_Record_T smtp_entry;

    if (log_entry == NULL)
        return FALSE;

    SMTP_MGR_GetSmtpAdminStatus(&status);
    if (status != SMTP_STATUS_ENABLE)
    {
        return FALSE;
    }

    SMTP_MGR_GetEmailSeverityLevel(&level);
    if (log_entry->owner_info.level > level)
    {
        return FALSE;
    }

    if (log_entry->owner_info.module_no >= SYS_MODULE_UNKNOWN)
    {
        return FALSE;
    }
    SYS_TIME_GetRealTimeBySec(&log_time);
    log_entry->log_time = log_time;
    memcpy(&smtp_entry,log_entry,sizeof(SMTP_OM_Record_T));
    SMTP_MGR_SendMail(&smtp_entry);

    return TRUE;
} /* End of SYSLOG_MGR_AddEntry_Smtp */
#endif

/* FUNCTION NAME: SYSLOG_MGR_SnmpGetUcNormalEntry
 * PURPOSE: Get current log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- current entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no current entry.
 * NOTES:   1. Use *record->entry_index = 0 to get first entry.
 *          2. SNMP will call this function.
 */
BOOL_T SYSLOG_MGR_SnmpGetUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    UI32_T index;
    SYSLOG_OM_Header_T  uc_normal_header, uc_flash_header;
    SYSLOG_OM_Record_T  syslog_entry, first_normal_log_entry;
    UI32_T  i, j;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return TRUE;
    }

    /* not in slave mode */

    /* Merge the UC flash/UC normal database together when get first information */
    if ((mgr_record->entry_index == 0) || (mgr_record->entry_index >= uc_database.count))
    {
        uc_database.count = 0;
        SYSLOG_OM_GetUcNormalHeader(&uc_normal_header);
        SYSLOG_OM_GetUcFlashHeader(&uc_flash_header);
        SYSLOG_OM_GetUcNormalEntry(uc_normal_header.front, &first_normal_log_entry);

        if ( (uc_normal_header.front == uc_normal_header.rear) && (uc_normal_header.count != 0) )
        {
            for (i = uc_flash_header.front, j=0; j<uc_flash_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB, j++)
            {
                SYSLOG_OM_GetUcFlashEntry(i, &syslog_entry);
                if (syslog_entry.log_time < first_normal_log_entry.log_time)
                {
                    uc_database.entry[j] = syslog_entry;
                    uc_database.count++;
                }
                else
                    break;
            }

            for (i = uc_normal_header.front, j=0; j<uc_normal_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
            {
                SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
                uc_database.entry[uc_database.count] = syslog_entry;
                uc_database.count++;
            }
        }
        else
        {
            /* copy from uc normal to database */
            uc_database.count = uc_normal_header.count;
            for (i = uc_normal_header.front, j=0; j<uc_normal_header.count; i=(i+1)%SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB, j++)
            {
                SYSLOG_OM_GetUcNormalEntry(i, &syslog_entry);
                uc_database.entry[j] = syslog_entry; /* structure copy */
            }
        }
    }

    if (uc_database.count == 0)
    {
         //SYSLOG_MGR_GetNextUcNormalEntries
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG, SYSLOG_MGR_GET_NEXT_UC_NORMAL_ENTRIES_FUNC_NO,
            EH_TYPE_MSG_DEB_MSG, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_WARNING));
#endif
        return FALSE;
    }

    if (mgr_record->entry_index == (UI32_T) (-1))
        mgr_record->entry_index = 0;

    if((mgr_record->entry_index < 0) || (mgr_record->entry_index >= uc_database.count))
    {

        return FALSE;
    }

    index = mgr_record->entry_index;
    mgr_record->owner_info = uc_database.entry[index].owner_info;
    SYS_TIME_ConvertSecondsToDateTime(uc_database.entry[index].log_time, &mgr_record->rtc_time.year,
        &mgr_record->rtc_time.month, &mgr_record->rtc_time.day, &mgr_record->rtc_time.hour,
        &mgr_record->rtc_time.minute, &mgr_record->rtc_time.second);
    memcpy(mgr_record->message, uc_database.entry[index].message, SYSLOG_ADPT_MESSAGE_LENGTH);

    return TRUE;
}

/* FUNCTION NAME: SYSLOG_MGR_SnmpGetNextUcNormalEntry
 * PURPOSE: Get next log message from un-cleared memory DB.
 * INPUT:   *record->entry_index    -- entry index.
 * OUTPUT:  *record                 -- next entry.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure or no next entry.
 * NOTES:   1. Use *record->entry_index = 0 to get first entry.
 *          2. SNMP will call this function.
 */
BOOL_T SYSLOG_MGR_SnmpGetNextUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    if (mgr_record->entry_index == (UI32_T)(-1))
    {
        mgr_record->entry_index = 0;
        return SYSLOG_MGR_SnmpGetUcNormalEntry(mgr_record);
    }
    else if (mgr_record->entry_index >= 0)
    {
        mgr_record->entry_index = (mgr_record->entry_index+1);
        return SYSLOG_MGR_SnmpGetUcNormalEntry(mgr_record);
    }
    return FALSE;
}

/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_NotifyStaTplgChanged
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that network topology is changed due to the
 *          STA enabled.
 * Parameter:
 * Return: None.
 * Note: When STA enabled, all the ports will go through STA algorithm to
 *       determine its operation state. During this period, the trap management
 *       shall wait until STA becomes stable. Otherwise, the trap message
 *       will be lost if the port is not in forwarding state.
 * -----------------------------------------------------------------------------
 */
void SYSLOG_MGR_NotifyStaTplgChanged (void)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
   /* BODY */


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {

        return;
    }
    spanning_tree_state = SYSLOG_ADPT_STA_UNSTABLED_STATE;

#endif
    return;
} /* End of SYSLOG_MGR_NotifyStaTplgChanged() */



/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SYSLOG_MGR_NotifyStaTplgStabled
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that STA has been enabled, and at least one of the port enters
 *          forwarding state. The network topology shall be stabled after couple seconds.
 * Parameter:
 * Return: None.
 * Note: This notification only informs that at least one of STA port enters forwarding state.
 *       To make sure all the STA ports enters stable state, we shall wait for few more seconds
 *       before we can send trap messages.
 * -----------------------------------------------------------------------------
 */
void SYSLOG_MGR_NotifyStaTplgStabled (void)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    /* BODY */


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {

        return;
    }

    if (spanning_tree_state == SYSLOG_ADPT_STA_UNSTABLED_STATE)
    {
        spanning_tree_state = SYSLOG_ADPT_STA_BECOME_STABLED_STATE;
        SYSFUN_SendEvent (syslog_task_id, SYSLOG_ADPT_EVENT_STA_STATE_CHANGED);
    }

#endif
    return;

} /* End of SYSLOG_MGR_NotifyStaTplgStabled() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleHotInsertion
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
void SYSLOG_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* no port database, do nothing */
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleHotRemoval
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
void SYSLOG_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* no port database, do nothing */
    return;
}

/* FUNCTION NAME: SYSLOG_MGR_AddRawMsgEntry
 * PURPOSE: Add a log message to system log module using raw message.
 * INPUT:   level_no   -- level number
 *          module_no  -- module number,defined in sys_module.h
 *          function_no -- function number,defined by CSC
 *          error_no   -- error number,defined by CSC
 *          message_index -- message index
 *          *arg_0     -- input argument 0
 *          *arg_1     -- input argument 1
 *          *arg_2     -- input argument 2
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_MGR_AddRawMsgEntry(
    UI8_T level_no,
    UI32_T module_no,
    UI8_T function_no,
    UI8_T error_no,
    UI32_T message_index,
    void *arg_0,
    void *arg_1,
    void *arg_2)
{
    SYSLOG_OM_RecordOwnerInfo_T owner_info;
    BOOL_T log_success;

    owner_info.level = level_no;
    owner_info.module_no = module_no;
    owner_info.function_no = function_no;
    owner_info.error_no = error_no;
    log_success = SYSLOG_MGR_AddFormatMsgEntry(
        &owner_info,
        message_index,
        arg_0,
        arg_1,
        arg_2);
    return log_success;

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYSLOG MGR.
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
BOOL_T SYSLOG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    SYSLOG_MGR_IPCMsg_T *msg_p;
    BOOL_T ret = TRUE;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        if((msg_p->type.cmd) > SYSLOG_MGR_FOLLOWING_NON_RESP_CMD)
            ret = FALSE;

        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
        return ret;
    }

    /* dispatch IPC message and call the corresponding VLAN_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case SYSLOG_MGR_IPC_ADD_ENTRY:
            msg_p->type.ret_bool =
                SYSLOG_MGR_AddEntry(&msg_p->data.syslog_entry_data.syslog_entry);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case SYSLOG_MGR_IPC_ADD_ENTRY_SYNC:
            msg_p->type.ret_bool =
                SYSLOG_MGR_AddEntrySync(&msg_p->data.syslog_entry_data.syslog_entry);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case SYSLOG_MGR_IPC_ADD_FORMAT_MSG_ENTRY:
            msg_p->type.ret_bool = SYSLOG_MGR_AddFormatMsgEntry(
                &msg_p->data.syslog_format_msg.owner_info,
                msg_p->data.syslog_format_msg.message_index,
                msg_p->data.syslog_format_msg.arg_0,
                msg_p->data.syslog_format_msg.arg_1,
                msg_p->data.syslog_format_msg.arg_2);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_CREATE_SERVER_IPADDR:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_CreateRemoteLogServer(&msg_p->data.syslog_remote_config.server_config.ipaddr);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SET_SERVER_PORT:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_SetRemoteLogServerPort(&msg_p->data.syslog_remote_config.server_config.ipaddr
                ,msg_p->data.syslog_remote_config.server_config.udp_port);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_GET_SERVER_PORT:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_GetRemoteLogServerPort(&msg_p->data.syslog_remote_config.server_config.ipaddr
                ,&msg_p->data.syslog_remote_config.server_config.udp_port);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_DELETE_SERVER_IPADDR:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_DeleteRemoteLogServer(&msg_p->data.syslog_remote_config.server_config.ipaddr);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_CLEAR_ALL_FLASH_ENTRIES:
            msg_p->type.ret_bool =
                SYSLOG_MGR_ClearAllFlashEntries();
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_CLEAR_ALL_RAM_ENTRIES:
            msg_p->type.ret_bool =
                SYSLOG_MGR_ClearAllRamEntries();
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_DELETE_ALL_SERVER:
            SYSLOG_MGR_DeleteAllRemoteLogServer();
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            ret = FALSE;
            break;

        case SYSLOG_MGR_IPC_GET_NEXT_UC_FLASH_ENTRY:
            msg_p->type.ret_bool =
            SYSLOG_MGR_GetNextUcFlashEntry(
            &msg_p->data.syslog_mgr_record_s.mgr_record);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record_s);
            break;

        case SYSLOG_MGR_IPC_GET_NEXT_UC_NORMAL_ENTRIES:
            msg_p->type.ret_bool =
            SYSLOG_MGR_GetNextUcNormalEntries(
            &msg_p->data.syslog_mgr_record_s.mgr_record);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record_s);
            break;

        case SYSLOG_MGR_IPC_GET_RUNNING_FLASH_LOG_LEVEL:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningFlashLogLevel(
                &msg_p->data.syslog_om_data.flash_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_STATUS:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningRemoteLogStatus(
                &msg_p->data.syslog_remote_config.status);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_GET_RUNNING_SYSLOG_STATUS:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningSyslogStatus(
                &msg_p->data.syslog_om_data.syslog_status);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_GET_RUNNING_UC_LOG_LEVEL:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningUcLogLevel(
                &msg_p->data.syslog_om_data.uc_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_SET_FLASH_LOG_LEVEL:
            msg_p->type.ret_ui32 = SYSLOG_MGR_SetFlashLogLevel(
                msg_p->data.syslog_om_data.flash_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SET_REMOTE_LOG_STATUS:
            msg_p->type.ret_ui32 = SYSLOG_MGR_SetRemoteLogStatus(
                msg_p->data.syslog_remote_config.status);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SET_SYSLOG_STATUS:
            msg_p->type.ret_bool = SYSLOG_MGR_SetSyslogStatus(
                msg_p->data.syslog_om_data.syslog_status);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SET_UC_LOG_LEVEL:
            msg_p->type.ret_ui32 = SYSLOG_MGR_SetUcLogLevel(
                msg_p->data.syslog_om_data.uc_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SNMP_GETNEXT_UC_NORMAL_ENTRY:
            msg_p->type.ret_bool =
                SYSLOG_MGR_SnmpGetNextUcNormalEntry((SYSLOG_MGR_Record_T *)&msg_p->data.syslog_mgr_record);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
            break;

        case SYSLOG_MGR_IPC_SNMP_GET_UC_NORMAL_ENTRY:
            msg_p->type.ret_bool =
                SYSLOG_MGR_SnmpGetUcNormalEntry((SYSLOG_MGR_Record_T *)&msg_p->data.syslog_mgr_record);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
            break;

        case SYSLOG_MGR_IPC_NOTIFYSTATPLGCHANGED:
            SYSLOG_MGR_NotifyStaTplgChanged();
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            ret = FALSE;
            break;

        case SYSLOG_MGR_IPC_NOTIFYSTATPLGSTABLED:
            SYSLOG_MGR_NotifyStaTplgStabled();
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            ret = FALSE;
            break;

        case SYSLOG_MGR_IPC_GETSYSLOGSTATUS:
            msg_p->type.ret_bool =
                SYSLOG_MGR_GetSyslogStatus(&msg_p->data.syslog_om_data.syslog_status);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_GETUCLOGLEVEL:
            msg_p->type.ret_bool =
                SYSLOG_MGR_GetUcLogLevel(&msg_p->data.syslog_om_data.uc_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_GETREMOTELOGSTATUS:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_GetRemoteLogStatus(&msg_p->data.syslog_remote_config.status);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_GETFLASHLOGLEVEL:
            msg_p->type.ret_bool =
                SYSLOG_MGR_GetFlashLogLevel(&msg_p->data.syslog_om_data.flash_log_level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_MGR_IPC_GET_NEXT_SERVER_CONFIG:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetNextRemoteLogServer(
                &msg_p->data.syslog_remote_config.server_config);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_GETSERVERCONFIGINDEXBYIPADDR:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRemoteLogServer(
                &msg_p->data.syslog_remote_config.server_config);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_GET_RUNNING_SERVER_CONFIG:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningRemoteLogServer(
                &msg_p->data.syslog_remote_config.server_config,
                msg_p->data.syslog_remote_config.server_index);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

/*fuzhimin,20090410*/
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)

        case SYSLOG_MGR_IPC_SET_SERVER_FACILITY:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_SetRemoteLogServerFacility(
                    &msg_p->data.syslog_remote_config.server_config.ipaddr,
                    msg_p->data.syslog_remote_config.server_config.facility);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_SET_SERVER_LEVEL:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_SetRemoteLogServerLevel(
				    &msg_p->data.syslog_remote_config.server_config.ipaddr,
                    msg_p->data.syslog_remote_config.server_config.level);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;
#else

        case SYSLOG_MGR_IPC_GET_RUNNING_FACILITY_TYPE:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningFacilityType(
                &msg_p->data.syslog_remote_config.facility);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

	    case SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_LEVEL:
            msg_p->type.ret_ui32 = SYSLOG_MGR_GetRunningRemotelogLevel(
                &msg_p->data.syslog_remote_config.level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

	    case SYSLOG_MGR_IPC_SET_FACILITY:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_SetRemoteLogFacility(msg_p->data.syslog_remote_config.facility);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_GET_FACILITY:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_GetRemoteLogFacility(&msg_p->data.syslog_remote_config.facility);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_MGR_IPC_SET_LEVEL:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_SetRemoteLogLevel(msg_p->data.syslog_remote_config.level);
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
            break;

        case SYSLOG_MGR_IPC_GET_LEVEL:
            msg_p->type.ret_ui32 =
                SYSLOG_MGR_GetRemoteLogLevel(&msg_p->data.syslog_remote_config.level);
            msgbuf_p->msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
            break;

#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */
/*fuzhimin,20090410,end*/

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;
    }

    return ret;
} /* End of SYSLOG_MGR_HandleIPCReqMsg */

/*------------------------------------------------------------------------
 * ROUTINE NAME - SYSLOG_MGR_GetUserInfoString
 *------------------------------------------------------------------------
 * FUNCTION: Get user information string
 * INPUT   : user_info_p      -- user information entry
 *           cb_str           -- count of bytes of str
 * OUTPUT  : str_p            -- display string
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
SYSLOG_MGR_GetUserInfoString(
    SYSLOG_MGR_UserInfo_T  *user_info_p,
    char                   *str_p,
    UI32_T                 cb_str)
{
    enum
    {
        MAC_STR_LEN = sizeof("00-00-00-00-00-00")-1
    };

    char  user_ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    char  user_mac_str[MAC_STR_LEN+1] = {0};

    if (0 != user_info_p->user_ip.addrlen)
    {
        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *) &user_info_p->user_ip,
                                                           user_ip_str,
                                                           sizeof(user_ip_str)))
        {
            return FALSE;
        }

        snprintf(user_mac_str, sizeof(user_mac_str), "%02X-%02X-%02X-%02X-%02X-%02X",
            user_info_p->user_mac[0], user_info_p->user_mac[1],
            user_info_p->user_mac[2], user_info_p->user_mac[3],
            user_info_p->user_mac[4], user_info_p->user_mac[5]);
        user_mac_str[sizeof(user_mac_str)-1] = '\0';
    }

    switch (user_info_p->session_type)
    {
        case VAL_trapVarSessionType_web:
            snprintf(str_p, cb_str, "User(%s/Web) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        case VAL_trapVarSessionType_snmp:
            snprintf(str_p, cb_str, "User(%s/SNMP) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        case VAL_trapVarSessionType_telnet:
            snprintf(str_p, cb_str, "User(%s/Telnet) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        case VAL_trapVarSessionType_console:
            snprintf(str_p, cb_str, "User(%s/Console)",
              user_info_p->user_name);
            break;

        case VAL_trapVarSessionType_ssh:
            snprintf(str_p, cb_str, "User(%s/SSH) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        case VAL_trapVarSessionType_http:
            snprintf(str_p, cb_str, "User(%s/HTTP) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        case VAL_trapVarSessionType_https:
            snprintf(str_p, cb_str, "User(%s/HTTPS) IP(%s) MAC(%s)",
              user_info_p->user_name, user_ip_str, user_mac_str);
            break;

        default:
            return FALSE;
    }

    str_p[cb_str] = '\0';
    return TRUE;
}/* End of SYSLOG_MGR_GetUserInfoString */
/*fuzhimin,20090414*/

/* FUNCTION NAME: SYSLOG_MGR_SaveUcLogsToFlash
 * PURPOSE: Save UC log data on UC memory to Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void SYSLOG_MGR_SaveUcLogsToFlash()
{
    UI32_T ret;
    SYSLOG_UcNormalDb_T *sys_log_ram_db_p;
    SYSLOG_UcFlashDb_T *sys_log_flash_db_p;

    if (FALSE == SYSLOG_OM_GetUcLogPointers(&sys_log_ram_db_p,
        &sys_log_flash_db_p))
    {
        return;
    }

    ret = FS_WriteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_RAM_UC_TMP_FILE,
        (UI8_T *)"SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *)sys_log_ram_db_p,
        sizeof(*sys_log_ram_db_p), 0);
    if (FS_RETURN_OK != ret)
    {
        SYSLOG_MGR_DEBUG_LOG("Failed to write syslog log ram db (ret = %lu).", ret);
    }

    ret = FS_WriteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_FLASH_UC_TMP_FILE,
        (UI8_T *)"SysLog", FS_FILE_TYPE_SYSLOG, (UI8_T *)sys_log_flash_db_p,
        sizeof(*sys_log_flash_db_p), 0);
    if (FS_RETURN_OK != ret)
    {
        SYSLOG_MGR_DEBUG_LOG("Failed to write syslog log flash db (ret = %lu).", ret);
    }
}

/* FUNCTION NAME: SYSLOG_MGR_RestoreUcLogsFromFlash
 * PURPOSE: Restore UC log data on UC memory
 *          from Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void SYSLOG_MGR_RestoreUcLogsFromFlash()
{
    UI32_T ret;
    UI32_T read_count = 0;
    SYSLOG_UcNormalDb_T *sys_log_ram_db_p;
    SYSLOG_UcFlashDb_T *sys_log_flash_db_p;

    if (FALSE == SYSLOG_OM_GetUcLogPointers(&sys_log_ram_db_p,
        &sys_log_flash_db_p))
    {
        return;
    }

    ret = FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_RAM_UC_TMP_FILE,
        (UI8_T *)sys_log_ram_db_p, sizeof(*sys_log_ram_db_p), &read_count);
    if (FS_RETURN_OK != ret)
    {
        SYSLOG_MGR_DEBUG_LOG("Failed to write sys log ram db (ret = %lu).", ret);
    }

    ret = FS_ReadFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_FLASH_UC_TMP_FILE,
        (UI8_T *)sys_log_flash_db_p, sizeof(*sys_log_flash_db_p), &read_count);
    if (FS_RETURN_OK != ret)
    {
        SYSLOG_MGR_DEBUG_LOG("Failed to write sys log flash db (ret = %lu).", ret);
    }
}

/* FUNCTION NAME: SYSLOG_MGR_DeleteUcLogsFromFlash
 * PURPOSE: Delete UC log data on UC memory
 *          from Flash.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
static void SYSLOG_MGR_DeleteUcLogsFromFlash()
{
    FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_RAM_UC_TMP_FILE);
    FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)SYSLOG_ADPT_SYS_LOG_FLASH_UC_TMP_FILE);
}
