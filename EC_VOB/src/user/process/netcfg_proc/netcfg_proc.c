/* MODULE NAME:  netcfg_proc.c
 * PURPOSE:
 *    This is the main function of netcfg process.
 *    The process will be daemonized by spawning a process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    05/02/2007 - Charlie Chen , Created
 *    07/10/2007 - Max Chen     , Modified for porting NETCFG
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"

#include "sys_cpnt.h"

#include "sysrsc_mgr.h"

#include "l_cmnlib_init.h"

/* Currently, NETCFG process only have one CSC group -- NETCFG. And It should be always
 * be enabled. If some CSC in NETCFG is disabled, this shall be controlled in CSC Group.
 * If other CSC Group is added, maybe a compile option is needed to separate it if all CSC member in
 * the group are disabled.
 */
#include "netcfg_proc_comm.h"
#include "netcfg_group.h"
#include "netcfg_om_ip.h"
#include "netcfg_om_nd.h"
#include "netcfg_om_route.h"

#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_om_pbr.h"
#endif
#include "l_mm_backdoor.h"

#include "mib2_pmgr.h"
#include "netcfg_pom_main.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define NETCFG_POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(NETCFG_PROC_OM_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union NETCFG_PROC_OM_MSG_U
{
    NETCFG_OM_IP_IPCMsg_T   netcfg_om_ip_ipcmsg;
    NETCFG_OM_ND_IPCMsg_T  netcfg_om_nd_ipcmsg;
    NETCFG_OM_ROUTE_IPCMsg_T netcfg_om_route_ipcmsg;
#if (SYS_CPNT_PBR == TRUE)
    NETCFG_OM_PBR_IPCMsg_T netcfg_om_pbr_ipcmsg;
#endif
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} NETCFG_PROC_OM_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T NETCFG_PROC_Init(void);
static BOOL_T NETCFG_PROC_InitiateProcessResources(void);
static void   NETCFG_PROC_Create_InterCSC_Relation(void);
static void   NETCFG_PROC_Create_All_Threads(void);
static void   NETCFG_PROC_Main_Thread_Function_Entry(void *arg);
static void   NETCFG_PROC_Daemonize_Entry(void* arg);

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T netcfg_omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(NETCFG_POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  main
 * PURPOSE:
 *    the main entry for NETCFG process
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
 *    This function is the entry point for NETCFG process.
 *
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(NETCFG_PROC_Init()==FALSE)
    {
        printf("NETCFG_Process_Init fail.\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_NETCFG_PROCESS_NAME,
                            NETCFG_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("NETCFG_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  NETCFG_PROC_Init
 * PURPOSE:
 *    Do initialization procedures for NETCFG process.
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
static BOOL_T NETCFG_PROC_Init(void)
{
    /* In NETCFG_PROC_Init(), following operations shall be
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

    if(NETCFG_PROC_InitiateProcessResources()==FALSE)
    {
        printf("%s: NETCFG_PROC_InitiateProcessResources fail\n", __FUNCTION__);
        return FALSE;
    }

    /* Register backdoor fun
     */
    NETCFG_PROC_Create_InterCSC_Relation();

    return TRUE;
}

/* FUNCTION NAME:  NETCFG_PROC_InitiateProcessResource
 * PURPOSE:
 *    Initialize the resource used in NETCFG process.
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
static BOOL_T NETCFG_PROC_InitiateProcessResources(void)
{
    if(NETCFG_PROC_COMM_InitiateProcessResources()==FALSE)
        return FALSE;

    /* After initialize the common resource for process, now
       initialize resource for each CSC */
    NETCFG_GROUP_InitiateProcessResources();

#if(SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_PMGR_Init();
#endif

    NETCFG_POM_MAIN_InitiateProcessResource();

    return TRUE;
}

/* FUNCTION NAME:  NETCFG_PROC_Create_InterCSC_Relation
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
 *
 */
static void NETCFG_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

    /* Create relationship between CSCs in each CSC group*/
    NETCFG_GROUP_Create_InterCSC_Relation();

    return;
}

/* FUNCTION NAME:  NETCFG_PROC_Create_All_Threads
 * PURPOSE:
 *    All of the threads in NETCFG process will be spawned in this function.
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
static void NETCFG_PROC_Create_All_Threads(void)
{

    NETCFG_GROUP_Create_All_Threads();

}

/* FUNCTION NAME:  NETCFG_PROC_Main_Thread_Function_Entry
 * PURPOSE:
 *    This is the entry function of the main thread for NETCFG process.
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
static void NETCFG_PROC_Main_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T ipcmsgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)netcfg_omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipcmsgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER,
            NETCFG_POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
#if 0
                case SYS_MODULE_NETCFG:
                    need_resp = NETCFG_OM_MAIN_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
                case SYS_MODULE_IPCFG:
                    need_resp = NETCFG_OM_IP_HandleIPCReqMsg(msgbuf_p);
                    break;

                case SYS_MODULE_NDCFG:
                    need_resp = NETCFG_OM_ND_HandleIPCReqMsg(msgbuf_p);
                    break;

                case SYS_MODULE_ROUTECFG:
                    need_resp = NETCFG_OM_ROUTE_HandleIPCReqMsg(msgbuf_p);
                    break;

#if (SYS_CPNT_PBR == TRUE)
                case SYS_MODULE_PBRCFG:
                    need_resp = NETCFG_OM_PBR_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
                /* invoke corresponding OM IPC Msg handler here
                 */
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

            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipcmsgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

        }
        else
        {
            printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
        }
    }

}

/* FUNCTION NAME:  NETCFG_PROC_Daemonize_Entry
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
static void NETCFG_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    NETCFG_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    NETCFG_PROC_Main_Thread_Function_Entry(arg);

}

