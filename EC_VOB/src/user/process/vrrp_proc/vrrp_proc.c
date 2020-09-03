/* MODULE NAME:  vrrp_proc.c
 * PURPOSE:
 *    This is the main function of vrrp process.
 *    The process will be daemonized by spawning a process.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009     

 *
 * Copyright(C)      Accton Corporation, 2009
 */
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
     
#include "sys_cpnt.h"
     
#include "sysrsc_mgr.h"
     
#include "l_cmnlib_init.h"
     
#include "vrrp_proc_comm.h"
#include "vrrp_group.h"
#include "vrrp_om.h"
#include "l_mm_backdoor.h"

#if (SYS_CPNT_NSM == TRUE)
    #include "nsm_pmgr.h"
#endif


#if (SYS_CPNT_NICDRV == TRUE)
#include "dev_nicdrv_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define VRRP_POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(VRRP_PROC_OM_MSG_T)


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union VRRP_PROC_OM_MSG_U
{
    VRRP_OM_IPCMsg_T vrrp_om_ipcmsg;
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} VRRP_PROC_OM_MSG_T;

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T vrrp_omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T VRRP_PROC_InitiateProcessResources(void);
static BOOL_T VRRP_PROC_Init(void);
static void VRRP_PROC_Daemonize_Entry(void* arg);
static void VRRP_PROC_Create_All_Threads(void);
static void VRRP_PROC_Main_Thread_Function_Entry(void* arg);
static void VRRP_PROC_Create_InterCSC_Relation(void);

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  VRRP_PROC_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource used in VRRP process.
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
static BOOL_T VRRP_PROC_InitiateProcessResources(void)
{
   
    if(VRRP_PROC_COMM_InitiateProcessResources()==FALSE)
        return FALSE;

	/* Init PMGR
     */
#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_NSM == TRUE)
    NSM_PMGR_InitiateProcessResource();
#endif

    /* After initialize the common resource for process, now
       initialize resource for each CSC */
    VRRP_GROUP_InitiateProcessResources();

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  VRRP_PROC_Init
 * PURPOSE:
 *    Do initialization procedures for VRRP process.
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
static BOOL_T VRRP_PROC_Init(void)
{
    /* In VRRP_PROC_Init(), following operations shall be
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
    
    VRRP_PROC_Create_InterCSC_Relation();
    if(VRRP_PROC_InitiateProcessResources()==FALSE)
        return FALSE;

    return TRUE;
}

/* FUNCTION NAME:  VRRP_PROC_Create_All_Threads
 * PURPOSE:
 *    All of the threads in VRRP process will be spawned in this function.
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
static void VRRP_PROC_Create_All_Threads(void)
{
  
    VRRP_GROUP_Create_All_Threads();
}

/* FUNCTION NAME:  VRRP_PROC_Main_Thread_Function_Entry
 * PURPOSE:
 *    This is the entry function of the main thread for VRRP process.
 *
 * INPUT:
 *    arg -- not used.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    Main thread is responsible for handling IPC request for read operations of
 *    OM in this process.
 *
 */
static void VRRP_PROC_Main_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T ipcmsgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)vrrp_omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_VRRP_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipcmsgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER,
                VRRP_POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

#if (SYS_CPNT_VRRP == TRUE)
                case SYS_MODULE_VRRP:
                    need_resp = VRRP_OM_HandleIPCReqMsg(msgbuf_p);
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
            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipcmsgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
        }
    }
    
    return;
}

/* FUNCTION NAME:  VRRP_PROC_Daemonize_Entry
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
static void VRRP_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
  
    VRRP_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    VRRP_PROC_Main_Thread_Function_Entry(arg);

}
/* FUNCTION NAME:  VRRP_PROC_Create_InterCSC_Relation
 * PURPOSE:
 *    Register call back functions.
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
static void VRRP_PROC_Create_InterCSC_Relation(void)
{
    VRRP_GROUP_Create_InterCSC_Relation();
    return;
}





/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  main
 * PURPOSE:
 *    the main entry for VRRP process
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
 *    This function is the entry point for OSPF process.
 *
 */
int main(void)
{
    UI32_T process_id;


    if(VRRP_PROC_Init()==FALSE)
    {
        printf("VRRP_PROC_Init fail.\n");
        return -1;
    }

    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_VRRP_PROCESS_NAME,
                            VRRP_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("VRRP_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

  

      return 0;
}


