/* MODULE NAME:  stktplg_proc.c
 * PURPOSE:
 *    Stack topology process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/11/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "stktplg_proc_comm.h"
#include "sysrsc_mgr.h"
#include "uc_mgr.h"
#include "stktplg_om.h"
#include "stktplg_group.h"
#include "stktplg_init.h"
#include "dev_swdrv_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "sys_pmgr.h"
#include "snmp_pmgr.h"
#include "stktplg_backdoor.h"
#include "l_mm_backdoor.h"
#include "l_cmnlib_init.h"
#include "cli_pom.h"
#include "mib2_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(STKTPLG_PROC_OM_Msg_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union STKTPLG_PROC_OM_Msg_U
{
    STKTPLG_OM_IPCMsg_T stktplg_om_ipcmsg;
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} STKTPLG_PROC_OM_Msg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T STKTPLG_PROC_Init(void);
static BOOL_T STKTPLG_PROC_InitiateProcessResources(void);
static void   STKTPLG_PROC_Create_InterCSC_Relation(void);
static void   STKTPLG_PROC_Create_All_Threads(void);
static void   STKTPLG_PROC_Main_Thread_Function_Entry(void);
static void   STKTPLG_PROC_Daemonize_Entry(void* arg);

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
 *    the main entry for stktplg process
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
 *    This function is the entry point for stktplg process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(STKTPLG_PROC_Init()==FALSE)
    {
        printf("STKTPLG_PROC_Init fail.\r\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_STKTPLG_PROCESS_NAME,
                            STKTPLG_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("STKTPLG_Process SYSFUN_SpawnProcess error.\r\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for STKTPLG process.
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
static BOOL_T STKTPLG_PROC_Init(void)
{
    /* In STKTPLG_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResources fail\n", __FUNCTION__);
        return FALSE;
    }

    DEV_SWDRV_PMGR_InitiateProcessResource();

    if(STKTPLG_PROC_InitiateProcessResources()==FALSE)
        return FALSE;

    DEV_NICDRV_PMGR_InitiateProcessResource();

    if(FALSE==SYS_PMGR_InitiateProcessResource())
    {
        printf("%s: SYS_PMGR_InitiateProcessResource fail\n", __FUNCTION__);
        return FALSE;
    }

    SNMP_PMGR_Init();

    /* Initiate XFER_PMGR when XFER_PMGR is ready
     */

    if(FALSE==CLI_POM_InitiateProcessResource())
    {
        printf("%s: CLI_POM_InitiateProcessResource fail\n", __FUNCTION__);
        return FALSE;
    }
	
#if(SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_PMGR_Init();
#endif
    /* Register callback fun
     */
    STKTPLG_PROC_Create_InterCSC_Relation();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in STKTPLG process.
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
static BOOL_T STKTPLG_PROC_InitiateProcessResources(void)
{

    /* In STKTPLG_INIT_InitiateProcessResources(), UC_MGR will be called.
     * Need to initiate UC_MGR before STKTPLG.
     */
    if(UC_MGR_InitiateProcessResources()==FALSE)
    {
        printf("\r\n%s:UC_MGR_InitiateProcessResources fail", __FUNCTION__);
        return FALSE;
    }

    if(STKTPLG_PROC_COMM_InitiateProcessResources()==FALSE)
        return FALSE;

    if(STKTPLG_INIT_InitiateProcessResources()==FALSE)
        return FALSE;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_Create_InterCSC_Relation
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
static void STKTPLG_PROC_Create_InterCSC_Relation(void)
{
    STKTPLG_BACKDOOR_Create_InterCSC_Relation();
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in STKTPLG process will be spawned in this function.
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
static void STKTPLG_PROC_Create_All_Threads(void)
{
    STKTPLG_GROUP_Create_All_Threads();

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for STKTPLG process.
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
static void STKTPLG_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    UI32_T rc;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\r\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_STKTPLG_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
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
                case SYS_MODULE_STKTPLG:
                    need_resp = STKTPLG_OM_HandleIPCReqMsg(msgbuf_p);
                    break;

		case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("%s: Invalid IPC req cmd.\r\n", __FUNCTION__);
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

            if((need_resp==TRUE) && ((rc=SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p))!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.(rc=%d)\r\n", __FUNCTION__, rc);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\r\n", __FUNCTION__);
        }
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_Daemonize_Entry
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
static void STKTPLG_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    STKTPLG_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    STKTPLG_PROC_Main_Thread_Function_Entry();

}

