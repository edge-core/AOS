/* MODULE NAME:  sflow_proc.c
 * PURPOSE:
 *    This is the main function of sflow process.
 *    The process will be daemonized by spawning a process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    09/08/2009 - Nelson Dai , Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"

#include "sysrsc_mgr.h"
#include "l_cmnlib_init.h"

#include "dev_amtrdrv_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_swdrv_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "dev_swdrvl4_pmgr.h"

#if (SYS_CPNT_SWCTRL == TRUE)
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#endif

#if (SYS_CPNT_NMTR == TRUE)
#include "nmtr_pmgr.h"
#endif

#if(SYS_CPNT_MIB2MGMT == TRUE)
#include "if_pmgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#endif

#include "netcfg_pmgr_route.h"
#include "netcfg_pom_ip.h"
#include "stktplg_pom.h"

#include "sflow_proc_comm.h"
#include "sflow_group.h"
#include "l_mm_backdoor.h"
/* NAMING CONSTANT DECLARATIONS
 */
 
/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define SFLOW_POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(SFLOW_PROC_OM_Msg_T)

/* DATA TYPE DECLARATIONS
 */
 /* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union SFLOW_PROC_OM_Msg_U
{
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} SFLOW_PROC_OM_Msg_T;

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SFLOW_POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SFLOW_PROC_Create_InterCSC_Relation(void); 
static BOOL_T SFLOW_PROC_InitiateProcessResources(void);
static BOOL_T SFLOW_PROC_Init(void);
static void SFLOW_PROC_Daemonize_Entry(void* arg);
static void SFLOW_PROC_Create_All_Threads(void);
static void SFLOW_PROC_Main_Thread_Function_Entry(void);

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  SFLOW_PROC_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource used in SFLOW process.
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
 *
 */
static BOOL_T SFLOW_PROC_InitiateProcessResources(void)
{
    if (SFLOW_PROC_COMM_InitiateProcessResources() == FALSE)
    {
        printf("%s: SFLOW_PROC_COMM_InitiateProcessResources fail\n", __FUNCTION__);
        return FALSE;
    }

    /* After initialize the common resource for process, now
       initialize resource for each CSC */

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

    DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NMTRDRV_PMGR_InitiateProcessResource();
    DEV_SWDRV_PMGR_InitiateProcessResource();
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
    DEV_SWDRVL4_PMGR_InitiateProcessResource();

#if(SYS_CPNT_MIB2MGMT == TRUE)
    IF_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();
#endif

#if (SYS_CPNT_NMTR == TRUE)
	NMTR_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_STKMGMT==TRUE)
    if(STKTPLG_POM_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_POM_InitiateProcessResources fail\n");
       return FALSE;
    }
#endif

    NETCFG_PMGR_ROUTE_InitiateProcessResource();
    NETCFG_POM_IP_InitiateProcessResource();

    SFLOW_GROUP_InitiateProcessResources();

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SFLOW_PROC_Init
 * PURPOSE:
 *    Do initialization procedures for SFLOW process.
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
 * NOTES:`
 *    None
 *
 */
static BOOL_T SFLOW_PROC_Init(void)
{
    /* In SFLOW_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    if (FALSE == SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResources fail\n", __FUNCTION__);
        return FALSE;
    }

    if (SFLOW_PROC_InitiateProcessResources() == FALSE)
    {
        printf("%s: SFLOW_PROC_InitiateProcessResources fail\n", __FUNCTION__);
        return FALSE;
    }

    /* Register callback function
     */
    SFLOW_PROC_Create_InterCSC_Relation();
    
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SFLOW_PROC_Create_InterCSC_Relation
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
static void SFLOW_PROC_Create_InterCSC_Relation(void)
{
    SFLOW_GROUP_Create_InterCSC_Relation();
    return;
}

/* FUNCTION NAME:  SFLOW_PROC_Create_All_Threads
 * PURPOSE:
 *    All of the threads in SFLOW process will be spawned in this function.
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
 *
 */
static void SFLOW_PROC_Create_All_Threads(void)
{
    SFLOW_GROUP_Create_All_Threads();
}
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SFLOW_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for SFLOW process.
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
static void SFLOW_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
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
    if(SYSFUN_CreateMsgQ(SYS_BLD_SFLOW_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER, SFLOW_POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
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

            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\r\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\r\n", __FUNCTION__);
        }
    }

}
/* FUNCTION NAME:  SFLOW_PROC_Daemonize_Entry
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
 *
 */
static void SFLOW_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    SFLOW_PROC_Create_All_Threads();
    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    SFLOW_PROC_Main_Thread_Function_Entry();
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  main
 * PURPOSE:
 *    the main entry for SFLOW process
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
 *    This function is the entry point for SFLOW process.
 *
 */
int main(void)
{
    UI32_T process_id;

	/* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    if (SFLOW_PROC_Init() == FALSE)
    {
        printf("SFLOW_Process_Init fail.\n");
        return -1;
    }

    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_SFLOW_PROCESS_NAME,
                            SFLOW_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("SFLOW_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }
    return 0;
}

