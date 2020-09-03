/*-----------------------------------------------------------------------------
 * FILE NAME: poe_proc.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the process for POE.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "sys_pmgr.h"
#include "poe_pom.h"
#include "poe_pmgr.h"
#include "poe_om.h"
#include "poe_init.h"
#include "poedrv.h"
#include "poedrv_init.h"
#include "poedrv_backdoor.h"
#include "poe_backdoor.h"
#include "l_cmnlib_init.h"
#include "backdoor_mgr.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "led_pmgr.h"
#include "stktplg_pom.h"
#include "phyaddr_access.h"
#include "uc_mgr.h"
#include "dev_swdrv_pmgr.h"
#include "l_mm_backdoor.h"
#include "dev_nicdrv_pmgr.h"

#if (SYS_CPNT_TIME_RANGE == TRUE)
#include "time_range_pmgr.h"
#endif

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(POE_PROC_OmMsg_T)
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union POE_PROC_OmMsg_U
{
    POE_OM_IpcMsg_T poe_om_ipcmsg;
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} POE_PROC_OmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T POE_PROC_Init(void);
static BOOL_T POE_PROC_InitiateProcessResource(void);
static void   POE_PROC_Create_InterCSC_Relation(void);
static void   POE_PROC_Daemonize_Entry(void* arg);

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
 *    the main entry for POE process
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
 *    This function is the entry point for POE process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    DBG_PRINT("\r\nPOE_PROC!!!!\r\n");
    if(POE_PROC_Init()==FALSE)
    {
        printf("POE_PROC_Init fail.\r\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_POE_PROCESS_NAME,
                            POE_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("POE_Process SYSFUN_SpawnProcess error.\r\n");
        return -1;
    }

    return 0;
} /* End of main */

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for POE process.
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
static BOOL_T POE_PROC_Init(void)
{
    /* In STKTPLG_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    DBG_PRINT();

    L_CMNLIB_INIT_InitiateProcessResources();

    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResources fail\n", __FUNCTION__);
        return FALSE;
    }
    DEV_SWDRV_PMGR_InitiateProcessResource();

    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
       return FALSE;
    }

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();
#endif
#if (SYS_CPNT_LLDP == TRUE)
    if(LLDP_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;
    if(LLDP_POM_InitiateProcessResources()==FALSE)
        return FALSE;
#endif
#if (SYS_CPNT_STKMGMT==TRUE)
    if(STKTPLG_POM_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_POM_InitiateProcessResources fail\n");
       return FALSE;
    }
#endif

#if (SYS_CPNT_TIME_RANGE == TRUE)
    if (TIME_RANGE_PMGR_InitiateProcessResource() == FALSE)
    {
        printf("%s(%d): TIME_RANGE_PMGR_InitiateProcessResource failed\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    if (LED_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(POE_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    /* Register callback fun
     */
    POE_PROC_Create_InterCSC_Relation();

    return TRUE;
} /* End of POE_PROC_Init */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in PoE process.
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
static BOOL_T POE_PROC_InitiateProcessResource(void)
{
    DBG_PRINT();

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

    if (POE_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;

    if (POE_POM_InitiateProcessResources()==FALSE)
        return FALSE;

    POE_INIT_InitiateSystemResources();


    return TRUE;
} /* End of POE_PROC_InitiateProcessResource */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_PROC_Create_InterCSC_Relation
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
static void POE_PROC_Create_InterCSC_Relation(void)
{
    /* Back door function : regerister to cli
     */
    DBG_PRINT();

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("POE",
        SYS_BLD_POE_GROUP_IPCMSGQ_KEY, POE_BACKDOOR_Main);

//    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("POE",
//        SYS_BLD_POE_PROC_OM_IPCMSGQ_KEY, DBG_POE_BackdoorInfo_CallBack);
} /* End of POE_PROC_Create_InterCSC_Relation */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for POE process.
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
static void POE_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)omtd_ipc_buf;
    SYSFUN_MsgQ_T ipc_msgq_handle;
    BOOL_T       need_resp;

    DBG_PRINT();

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_POE_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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

                case SYS_MODULE_POE:
                    need_resp = POE_OM_HandleIPCReqMsg(msgbuf_p);
                    break;

                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

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

} /* End of POE_PROC_Main_Thread_Function_Entry */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_PROC_Daemonize_Entry
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
static void POE_PROC_Daemonize_Entry(void* arg)
{

    POE_INIT_CreateTasks();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    POE_PROC_Main_Thread_Function_Entry();

} /* End of POE_PROC_Daemonize_Entry */


