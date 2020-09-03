/* MODULE NAME:  CORE_UTIL_proc.c
 * PURPOSE:
 *    for core util process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/6/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "core_util_proc_comm.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "utility_group.h"
#include "stkctrl_pmgr.h"
#include "stktplg_pom.h"
#include "stktplg_pmgr.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "l_cmnlib_init.h"
#include "uc_mgr.h"

#include "snmp_pmgr.h"
#include "mib2_pom.h"
#include "iml_pmgr.h"

#if (SYS_CPNT_CFGDB == TRUE)
#include "cfgdb_group.h"
#endif
/* cfgdb group */
#if (SYS_CPNT_CFGDB == TRUE)
#include "cfgdb_init.h"
#endif


#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_init.h"
#include "syslog_om.h"
#endif
#if (SYS_CPNT_NICDRV == TRUE)
#include "dev_nicdrv_pmgr.h"
#endif

#include "netcfg_pmgr_main.h"
#include "netcfg_pom_main.h"

#if(SYS_CPNT_MIB2MGMT==TRUE)
#include "mib2_pmgr.h"
#endif

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif

#include "dev_swdrv_pmgr.h"
#include "lldp_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for MGR thread to receive and response
 * PMGR ipc. The size of this buffer should pick the maximum of size required
 * for PMGR ipc request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
/* EH group
 */
#define CORE_UTIL_PROC_UTILITY_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(CORE_UTIL_PROC_UTILITY_GROUP_MGR_MSG_T)

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(CORE_UTIL_PROC_OM_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union CORE_UTIL_PROC_OM_MSG_U
{
/* EH group
 */
#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_OM_IPCMsg_T syslog_om_ipcmsg;
#endif

} CORE_UTIL_PROC_OM_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T CORE_UTIL_PROC_Init(void);
static BOOL_T CORE_UTIL_PROC_InitiateProcessResource(void);
static void   CORE_UTIL_PROC_Create_InterCSC_Relation(void);
static void   CORE_UTIL_PROC_Create_All_Threads(void);
static void   CORE_UTIL_PROC_Daemonize_Entry(void* arg);

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for CORE_UTIL process
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for CORE_UTIL process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(CORE_UTIL_PROC_Init()==FALSE)
    {
        printf("CORE_UTIL_Process_Init fail.\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_CORE_UTIL_PROCESS_NAME,
                            CORE_UTIL_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("CORE_UTIL_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CORE_UTIL process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T CORE_UTIL_PROC_Init(void)
{
    /* In CORE_UTIL_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */

    L_CMNLIB_INIT_InitiateProcessResources();

    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if(CORE_UTIL_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    /* Register callback fun
     */
#if (SYS_CPNT_NICDRV == TRUE)
      DEV_NICDRV_PMGR_InitiateProcessResource();
#endif
    CORE_UTIL_PROC_Create_InterCSC_Relation();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in CORE_UTIL process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T CORE_UTIL_PROC_InitiateProcessResource(void)
{
    if(CORE_UTIL_PROC_COMM_InitiateProcessResource()==FALSE)
        return FALSE;

    UTILITY_GROUP_InitiateProcessResource();

    /* for SMTP */
    SNMP_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
    IML_PMGR_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();

    if(NETCFG_PMGR_MAIN_InitiateProcessResource()==FALSE)
    {
        return FALSE;
    }
    /* for SMTP */

    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();

#if (SYS_CPNT_STKMGMT==TRUE)
    if(STKTPLG_POM_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_POM_InitiateProcessResources fail\n");
       return FALSE;
    }

    if(STKTPLG_PMGR_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_PMGR_InitiateProcessResources fail\n");
       return FALSE;
    }

    if(STKCTRL_PMGR_InitiateProcessResources()==FALSE)
    {
       printf(" STKCTRL_PMGR_InitiateProcessResources fail\n");
       return FALSE;
    }
#endif

/* for aluc: CMGR_SetPortStatus*/
#if(SYS_CPNT_CFM == TRUE)
    CFM_PMGR_InitiateProcessResources();
#endif
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_PMGR_InitiateProcessResources();
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    EFM_OAM_PMGR_InitiateProcessResource();
#endif
/* end for aluc: CMGR_SetPortStatus */


    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf(" UC_MGR_InitiateProcessResources fail\n");
       return FALSE;
    }

    DEV_SWDRV_PMGR_InitiateProcessResource();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Relashionships between CSCs are created through function
 *    pointer registrations and callback functions.
 *    At this init phase, all operations related to function pointer
 *    registrations will be processed.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void CORE_UTIL_PROC_Create_InterCSC_Relation(void)
{
    UTILITY_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_CFGDB == TRUE)
    CFGDB_INIT_Create_InterCSC_Relation();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in CORE_UTIL process will be spawned in this function.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void CORE_UTIL_PROC_Create_All_Threads(void)
{
    UTILITY_GROUP_Create_All_Threads();

#if (SYS_CPNT_CFGDB == TRUE)
    CFGDB_GROUP_Create_All_Threads();
#endif

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for CORE_UTIL process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    The main thread of the process shall never be terminated before any other
 *    threads spawned in this process. If the main thread terminates, other
 *    threads in the same process will also be terminated.
 *
 *    The main thread is responsible for handling OM IPC request for read
 *    operations of All OMs in this process.
 *------------------------------------------------------------------------------
 */
static void CORE_UTIL_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_CORE_UTIL_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER, POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
#if (SYS_CPNT_SYSLOG == TRUE)
                case SYS_MODULE_SYSLOG:
                    need_resp = SYSLOG_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
                default:
                    printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                    /* Unknow command. There is no way to idntify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp=FALSE;
            }

            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
        }
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    After the process has been daemonized, the main thread of the process
 *    will call this function to start the main thread.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void CORE_UTIL_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    CORE_UTIL_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    CORE_UTIL_PROC_Main_Thread_Function_Entry();

}

