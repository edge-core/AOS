/* MODULE NAME: Driver_PROC.C
 *
 * PURPOSE:
 *    This module implements the functionality of Driver process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/13     --- Echo, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <stdlib.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "sys_pmgr.h"
#include "l_cmnlib_init.h"

#include "driver_proc_comm.h"
/* driver group will handle mode change events, anzhen.zheng, 6/26/2008 */
#if 0 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-12, 18:14:13 */
#include "driver_group_operation.h"
#endif
#include "driver_group.h"
#include "sysrsc_mgr.h"
#include "dev_swdrv_pmgr.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_swdrv.h"
#include "stktplg_pmgr.h"
#include "stktplg_board.h"
#if (SYS_CPNT_L2MCAST == TRUE)
#include "msl_group.h"
#endif
#include "dev_swdrvl4_pmgr.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "dev_rm_pmgr.h"
#if (SYS_CPNT_NICDRV == TRUE)
#include "dev_nicdrv.h"
#endif
#include "uc_mgr.h"
#include "l_mm_backdoor.h"

#if (SYS_HWCFG_I2C_INIT_PHERIPHERAL==TRUE)
#include "i2cdrv.h"
#endif

#if (SYS_CPNT_CPU_CONTROL_PACKET_RATE_LIMIT == TRUE)
#include "netcfg_pom_ip.h"
#endif

#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)
#include "onlpdrv_sfp.h"
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#include "fs_init.h"
#include "mib2_pmgr.h"

/* for a driver_proc to support backdoor via linux shell without using Simba/CLI  backdoor
 */
#include "dev_nicdrv_gateway.h"
#include "dev_nicdrv.h"
#include "lan.h"

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define DBG_PRINT(...) printf(__VA_ARGS__);
#else
#define DBG_PRINT(...)
#endif

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(DRIVER_PROC_OmMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef union
{
#if (SYS_CPNT_NICDRV == TRUE)
        DEV_NICDRV_IPCMsg_T dev_nicdrv_mgr_ipcmsg;
#endif
        DEV_AMTRDRV_PMGR_IPCMSG_T dev_amtrdrv_ipcmsg;
        DEV_SWDRVL3_PMGR_IPCMSG_T dev_swdrvl3_ipcmsg;
        DEV_SWDRVL4_PMGR_IPCMSG_T dev_swdrvl4_ipcmsg;
        DEV_NMTRDRV_PMGR_IPCMSG_T dev_nmtrdrv_ipcmsg;
        DEVRM_PMGR_IPCMSG_T dev_rm_ipcmsg;
        L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;

} DRIVER_PROC_OmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T DRIVER_PROC_Init(void);
static BOOL_T DRIVER_PROC_InitiateProcessResource(void);
static void   DRIVER_PROC_Create_InterCSC_Relation(void);
static void   DRIVER_PROC_Daemonize_Entry(void* arg);
static void   DRIVER_PROC_Create_All_Threads(void);
static void   DRIVER_PROC_Main_Thread_Function_Entry(void);
static void   DRIVER_PROC_InitPeripheral(void);
#if (SYS_CPNT_DRVP_DBG == TRUE)
static void   DRIVER_PROC_DbgThread(void);
#endif

/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for om thread
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* The initialization function in device driver might spawn threads.
 * These threads must be spawned by the forked child process. If these
 * threads are spawned by the parent process, they will be terminated
 * with the parent process. However, the parent process shall not
 * terminate before the initialization is done. Because the other
 * processes which are executed after driver_proc might depend on the
 * initialization of device driver to work propoerly. To ensure the
 * process that are being executed latter will be run after the device
 * driver has done its initialization, we will use a semaphore to hold
 * the parent process of driver_proc until the initialization which is
 * performed in the forked child process has finished.
 */
static UI32_T init_sync_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - main
 *-----------------------------------------------------------------------------
 * PURPOSE : The main entry for Driver process
 *
 * INPUT   : argc --  the size of the argv array
 *           argv --  the array of arguments
 *
 * OUTPUT  : None.
 *
 * RETURN  : 0  -- Success
 *           -1 -- Error
 * NOTES   : This function is the entry point for Driver process.
 *-----------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id, rc;

    if((rc=SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 0, SYSFUN_SEM_FIFO, &init_sync_sem_id))!=SYSFUN_OK)
    {
        printf("\r\n%s:Failed to create init sync semaphore. driver_proc halts!(rc=%lu, semid=%lu)", __FUNCTION__, (unsigned long)rc, (unsigned long)init_sync_sem_id);
        while(1){}
    }

    /* Spawn the Driver process
     */
#if 1
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_DRIVER_PROCESS_NAME,
                            DRIVER_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("\nSYSFUN_SpawnProcess(Driver process) error.\n");
        return -1;
    }


#else
    DRIVER_PROC_Daemonize_Entry(NULL);
    sal_core_init();
    sal_appl_init();
    diag_shell();
#endif
    if((rc=SYSFUN_TakeSem(init_sync_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER))!=SYSFUN_OK)
    {
        printf("\r\n%s:Failed to take init sync semaphore. driver_proc halts!(rc=%lu, semid=%lu)", __FUNCTION__, (unsigned long)rc, (unsigned long)init_sync_sem_id);
        while(1){}
    }
    SYSFUN_DestroySem(init_sync_sem_id);
    return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - Driver_PROC_Init
 *-----------------------------------------------------------------------------
 * PURPOSE : Do initialization procedures for Driver process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DRIVER_PROC_Init(void)
{
    /* In Driver_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
       return FALSE;
    }

    if (SYSRSC_MGR_AttachSystemResourcesWithoutCscRsc() == FALSE)
    {
        printf("\n%s: SYSRSC_MGR_AttachSystemResourcesWithoutCscRsc fail\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_UCMGMT_SHMEM == TRUE)
    UC_MGR_AttachSystemResources();
#endif

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    /* DEV_SWDRV_ChipInit() need FS to read config file.
     */
    FLASHDRV_INIT_AttachSystemResources();
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

    /* For now, STKTPLG_BOARD_InitiateProcessResources() is called after
     * stktplg_proc is forked. However, driver_proc is forked before
     * stktplg_proc. In order to access board info before driver_proc
     * is forked, STKTPLG_BOARD_InitiateProcessResources() will be called here.
     *
     * Note that UC_MGR should be initialized before STKTPLG_BOARD_InitiateProcessResources()
     * is called because STKTPLG_BOARD need to access UC to get board id.
     *
     * TBD: STKTPLG_BOARD should follow the convention to initial
     *      shared memory. Function call to STKTPLG_BOARD_InitiateProcessResources()
     *      can be removed here after STKTPLG_BOARD had refined.
     * charlie_chen 2009/11/4
     */
    STKTPLG_BOARD_InitiateProcessResources();

    /* DEV_SWDRV_ChipInit() need to be called after STKTPLG_BOARD has been initialized
     * the reason is in DEV_SWDRV_ChipInit() will call STKTPLG_BOARD_GetNumberOfSyncEPort()
     */
    DEV_SWDRV_ChipInit();
    DEV_SWDRV_DisableAllPortAdmin();
    /* Note that L_CMNLIB_INIT_InitiateProcessResources() need to be
     * called after DEV_SWDRV_ChipInit() is called. (At least when using
     * Broadcom SDK) Because L_IPCMEM pre-initialization for linux kernel
     * high memory will be done in DEV_SWDRV_ChipInit() and this need to
     * be done before calling L_IPCMEM_Initialize() (Called in L_CMNLIB_INIT_InitiateProcessResources)
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    if (SYSRSC_MGR_AttachSystemResources() == FALSE)
    {
        printf("\n%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if (DRIVER_PROC_InitiateProcessResource() == FALSE)
    {
        return FALSE;
    }

    /* Register callback functions
     */
    DRIVER_PROC_Create_InterCSC_Relation();
    MIB2_PMGR_Init();

    DRIVER_PROC_InitPeripheral();

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - Driver_PROC_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the resource used in Driver process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DRIVER_PROC_InitiateProcessResource(void)
{
    /* Init common resource
     */
    if ( DRIVER_PROC_COMM_InitiateProcessResource() == FALSE)
    {
        return FALSE;
    }

    /* Init resources of each member CSC groups
     */
    DRIVER_GROUP_InitiateProcessResources();

    STKTPLG_PMGR_InitiateProcessResources();
#if (SYS_CPNT_L2MCAST == TRUE)
    MSL_GROUP_InitiateProcessResource();
#endif
    if(SYS_PMGR_InitiateProcessResource()== FALSE)
    {
       printf(" SYS_PMGR_InitiateProcessResource fail\n");
       return FALSE;
    }

#if(SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_PMGR_Init();
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
    if(DHCPSNP_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_CPU_CONTROL_PACKET_RATE_LIMIT == TRUE)
    if (NETCFG_POM_IP_InitiateProcessResource() == FALSE)
        return FALSE;
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DRIVER_PROC_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Relashionships between CSCs are created through function
 *           pointer registrations and callback functions.
 *           At this init phase, all operations related to function pointer
 *           registrations will be processed.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void DRIVER_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();
    DRIVER_GROUP_Create_InterCSC_Relation();
#if (SYS_CPNT_L2MCAST == TRUE)
    MSL_GROUP_Create_InterCSC_Relation();
#endif
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - Driver_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE : After the process has been daemonized, the main thread of the
 *           process will call this function to start the main thread.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *------------------------------------------------------------------------------
 */
static void DRIVER_PROC_Daemonize_Entry(void* arg)
{
    UI32_T rc;

    if (DRIVER_PROC_Init() == FALSE)
    {
        printf("\nDriver_PROC_Process_Init fail.\n");
        return ;
    }

    if((rc=SYSFUN_GiveSem(init_sync_sem_id))!=SYSFUN_OK)
    {
        printf("\r\n%s:Failed to give init sync semaphore. driver_proc halts!(rc=%lu, semid=%lu)", __FUNCTION__, (unsigned long)rc, (unsigned long)init_sync_sem_id);
    }

    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    DRIVER_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    DRIVER_PROC_Main_Thread_Function_Entry();

    return;
}

#if (SYS_CPNT_DRVP_DBG == TRUE)
static void DRIVER_PROC_CreateDbgThread(void)
{
  UI32_T thread_id;

  if (SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                          SYSFUN_SCHED_DEFAULT,
                          "DRVP_DBG",
                          SYS_BLD_CLI_TASK_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          DRIVER_PROC_DbgThread,
                          NULL,
                          &thread_id)!= SYSFUN_OK)
  {
      printf("\r\nDRIVER_PROC_CreateDbgThread fail\n");
  }
  return;
}
#endif
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - Driver_PROC_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : Spaw all threads in Driver process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void DRIVER_PROC_Create_All_Threads(void)
{
    DRIVER_GROUP_Create_All_Threads();
#if (SYS_CPNT_L2MCAST == TRUE)
    MSL_GROUP_Create_All_Threads();
#endif
#if (SYS_CPNT_DRVP_DBG == TRUE)
    DRIVER_PROC_CreateDbgThread();
#endif
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - Driver_PROC_Main_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : The entry function of the main thread for Driver process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : 1. The main thread of the process shall never be terminated before
 *              any other threads spawned in this process. If the main thread
 *              terminates, other threads in the same process will also be
 *              terminated.
 *           2. The main thread is responsible for handling OM IPC request for
 *              read operations of All OMs in this process.
 *-----------------------------------------------------------------------------
 */
static void DRIVER_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T   ipc_msgq_handle;
/* driver group will handle mode change events, anzhen.zheng, 6/26/2008 */
#if 0 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-10, 14:53:44 */
    UI32_T          received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
#endif

    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if (SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY,
            SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);
        return;
    }

    /* create om ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_DRIVER_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while (1)
    {
    /* driver group will handle mode change events, anzhen.zheng, 6/26/2008 */
#if 1 /* Yongxin.Zhao added, use driver process main thread to handle stacking mode change event, 2008-04-10, 14:58:59 */
        /* wait for the request ipc message
         */
        if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER,
                POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch (msgbuf_p->cmd)
            {
                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("\n%s: Invalid IPC req cmd.\n", __FUNCTION__);
                    /* Unknow command. There is no way to idntify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp = FALSE;
            }

            if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle,
                    msgbuf_p) != SYSFUN_OK))
            {
                printf("\n%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);
            }
        }
        else
        {
            SYSFUN_Debug_Printf("\n%s: SYSFUN_ReceiveMsg fail.\n",
                __FUNCTION__);
        }
#else
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;
        DBG_PRINT("%s: receive_events=%lu\n",__FUNCTION__,received_events);

        /* handle set transition mode event
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            printf("%s: receive set transition mode event\n",__FUNCTION__);

            DRIVER_GROUP_SetTransitionMode();

            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;
            /* need not to do IPCFAIL recovery in transition mode
             */
            if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
            {
                /* clear fail info in IPCFAIL
                 */
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
            }

        }

        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
            {
                DBG_PRINT("%s: msgbuf_p->cmd=%lu\n",__FUNCTION__,msgbuf_p->cmd);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        DBG_PRINT("%s: DRIVER_GROUP_EnterMasterMode start\n",__FUNCTION__);
                        DRIVER_GROUP_EnterMasterMode();
                        DBG_PRINT("%s: DRIVER_GROUP_EnterMasterMode suceeed\n",__FUNCTION__);
                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer l2mux group has
                         * entered transition mode but lower layer l2mux groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer l2mux group. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }

                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        DBG_PRINT("%s: DRIVER_GROUP_EnterSlaveMode start\n",__FUNCTION__);
                        DRIVER_GROUP_EnterSlaveMode();
                        DBG_PRINT("%s: DRIVER_GROUP_EnterSlaveMode suceeed\n",__FUNCTION__);
                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer l2mux group has
                         * entered transition mode but lower layer l2mux group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer l2mux group. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        DBG_PRINT("%s: DRIVER_GROUP_EnterTransitionMode start\n",__FUNCTION__);
                        DRIVER_GROUP_EnterTransitionMode();
                        DBG_PRINT("%s: DRIVER_GROUP_EnterTransitionMode suceeed\n",__FUNCTION__);
                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        DBG_PRINT("%s: DRIVER_GROUP_ProvisionComplete start\n",__FUNCTION__);
                        DRIVER_GROUP_ProvisionComplete();
                        DBG_PRINT("%s: DRIVER_GROUP_ProvisionComplete suceeed\n",__FUNCTION__);
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        DRIVER_GROUP_HandleHotInsertion();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        DRIVER_GROUP_HandleHotRemoval();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
 #endif

                    default:
                        /* Unknow command. There is no way to idntify whether this
                         * ipc message need or not need a response. If we response to
                         * a asynchronous msg, then all following synchronous msg will
                         * get wrong responses and that might not easy to debug.
                         * If we do not response to a synchronous msg, the requester
                         * will be blocked forever. It should be easy to debug that
                         * error.
                         */
                        printf("%s: Invalid IPC req cmd. msgbuf_p->cmd=%lu\n", __FUNCTION__,msgbuf_p->cmd);
                        need_resp=FALSE;
                }
                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            }
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        }

        /* handle IPC Async Callback fail when IPC Msgq is empty
         */
        if((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
            !(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG))
        {
            /* read fail info from IPCFAIL
             */

            /* do recovery action
             */
            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
        }
#endif
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DRIVER_PROC_InitPeripheral
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will do initialization to hardware pheripheral
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : This function is called in the context of the main thread of
 *           driver_proc. Some drivers need to be done through the PMGR, which
 *           means the DRIVER_GROUP MGR thread need to be ready. So this function
 *           is called in the beginning of the creation of driver_proc main thread
 *           to ensure DRIVER_GROUP MGR thread is ready at this moment.
 *-----------------------------------------------------------------------------
 */
static void DRIVER_PROC_InitPeripheral(void)
{
#if (SYS_HWCFG_I2C_INIT_PHERIPHERAL==TRUE)
    #if (SYS_CPNT_I2CDRV_BUS0_RUN_IN_DRIVER_PROC_CONTEXT==TRUE)
    /* When I2CDRV_InitPeripheral() is called on the projects that use I2CDRV of
     * MARVELL SDK, the thread to handle the IPC Msg of the I2CDRV operation has not
     * been spawned yet. Thus pass DEV_SWDRV_TwsiDataReadWithBusIdx and
     * DEV_SWDRV_TwsiDataWriteWithBusIdx to I2CDRV_InitPeripheral() to use
     * direct function call.
     */

    I2CDRV_InitPeripheral(DEV_SWDRV_TwsiDataReadWithBusIdx, DEV_SWDRV_TwsiDataWriteWithBusIdx);
    #else
    I2CDRV_InitPeripheral(NULL, NULL);
    #endif
#endif
}

#if (SYS_CPNT_DRVP_DBG == TRUE)

static UI8_T DRIVER_PROC_HextoUI8_T(char *hexstr)
{
    int   len;
    int   i;
    int   onedigit, value = 0;

    len = strlen(hexstr);

    for(i=0; i<len; i++)
    {
        if( *(hexstr+i) <= '9' &&  *(hexstr+i) >= '0')
            onedigit = *(hexstr+i) - 48;
        else if ( *(hexstr+i) <= 'F' &&  *(hexstr+i) >= 'A')
            onedigit = *(hexstr+i) - 65+10;
        else if ( *(hexstr+i) <= 'f' &&  *(hexstr+i) >= 'a')
            onedigit = *(hexstr+i) - 97+10;
        else
            return 0;

        value = ( (value) << 4 ) + onedigit;
    }
    return (UI8_T) value;
}
static int DRIVER_PROC_HexStringTo_UI8T_Values(UI8_T *destBuf, char *HexString)
{
    char *token;
    char *saved_ptr;
    int i = 0;
    token = strtok_r(HexString, "-.", &saved_ptr);
    destBuf[i++] = DRIVER_PROC_HextoUI8_T(token);
    while (NULL != (token = strtok_r(NULL, "-.", &saved_ptr)))
    {
        destBuf[i++] = DRIVER_PROC_HextoUI8_T(token);

    }
    return i;
}
/* move to sysfun when works */
#if 0
static int DRIVER_PROC_GetTTY(int *fd, struct termios *termios_org)
{
#define SYSFUN_UART_SET_RAW_MODE(termios_p)     \
    do {                                        \
        cfmakeraw(termios_p);                   \
        (termios_p)->c_oflag |= (ONLCR | OPOST);\
        (termios_p)->c_lflag |= ISIG;           \
    } while (0)
#define SYSFUN_UART_SET_DEFAULT_MODE(termios_p)         \
    do {                                                \
        (termios_p)->c_iflag |= TTYDEF_IFLAG;           \
        (termios_p)->c_oflag |= TTYDEF_OFLAG;           \
        (termios_p)->c_lflag |= TTYDEF_LFLAG;           \
    } while (0)

    static struct termios  termios;

    *fd = open(SYS_ADPT_DEV_UART_NAME_0, O_RDWR | O_NONBLOCK);
    if (*fd < 0)
    {
        printf("open(SYS_ADPT_DEV_UART_NAME_0, O_RDWR | O_NONBLOCK); fail\n");
        return -1;
    }
    printf("fd=%d\r\n", *fd);
    if (tcgetattr(*fd, &termios) < 0)
    {
        printf("tcgetattr fail\r\n");
        return -1;
    }
    memcpy(termios_org, &termios, sizeof(termios));
    SYSFUN_UART_SET_RAW_MODE(&termios);
    SYSFUN_UART_SET_DEFAULT_MODE(&termios);
//    termios.c_cc[VINTR] = 0x3; /* map signal VINTR back to CTRL+C from CTRL+L */
    if (tcsetattr(*fd, TCSANOW, &termios) < 0)
    {
        printf("tcsetattr fail\r\n");
        return -1;
    }
    if (ioctl(*fd, TIOCSCTTY, (char *)1) < 0)
    {
        printf("ioctl UART TIOCSCTTY error\r\n");
        return -1;
    }
    return 0;
}
static void DRIVER_PROC_GiveTTY(int fd, struct termios *termios_org)
{
    tcsetattr(fd, TCSANOW, termios_org);
    close(fd);
    #if 0 /* how to release the tty control ? */
    if (ioctl(0, TIOCSCTTY, (char *)1) < 0)
    {
        printf("ioctl UART TIOCSCTTY error\n");
        return -1;
    }
    #endif
}
#endif
static void DRIVER_PROC_DbgThreadCmd()
{
   printf("\r\n--Usage:-----\r\n");
   printf("drvdbg_proc b xxx : to execute BCM Diag command\r\n");
   printf("drvdbg_proc B     : to spawn BCM Diag shell\r\n");
   printf("drvdbg_proc p o   : to execute OF Packet-out to pipeline test\r\n");
   printf("drvdbg_proc n 1   : to execute dev_nicdrv_gateway backdoor\r\n");
   printf("drvdbg_proc n 2   : to execute dev_nicdrv backdoor\r\n");
   printf("drvdbg_proc n 3   : to execute lan backdoor\r\n");
   return;
}
static void DRIVER_PROC_DbgThread(void)
{
#define BUF_LEN 600
#define CLIENT_ADDR "/var/run/drvdbg_socket_client" /* need to match with drvdbg_proc.c */
#define SERVER_ADDR "/var/run/drvdbg_socket_server"


    int rc, i;
    int sockfd;
    int recv_bytes, read_len;
    char *rxbuf, *packet;
    char *token[5];
    char *saved_ptr;
    FILE *fd;

    rxbuf = malloc(BUF_LEN);
    packet = malloc(BUF_LEN);
    if (rxbuf == NULL || packet == NULL)
    {
        if (rxbuf != NULL)
            free(rxbuf);
        printf("\r\n DRIVER_PROC_DbgThread malloc fail\r\n");
        return;
    }

    if (SYSFUN_OK != SYSFUN_CreateIPCSocketClient(CLIENT_ADDR, &sockfd, 50 * 1024))
    {
        printf("\r\nDRIVER_PROC_DbgThread: Failed on creating Unix Domain Socket !\r\n\r\n");
        free(rxbuf);
        free(packet);
        return;
    }
    while (1)
    {
      recv_bytes = SYSFUN_RecvFromIPCSocket(sockfd, rxbuf, BUF_LEN);
      /*
      printf("\r\n DrvP_Dbg: rxbug(%d)=%s\r\n", recv_bytes, rxbuf);
      */
      if (recv_bytes <= 0)
      {
        continue;
      }
      token[0] = strtok_r(rxbuf, " ", &saved_ptr);
      switch ((char)(*token[0]))
        {
        case 'p': /* OF Packet Out*/
            token[1] = strtok_r(NULL, " ", &saved_ptr);
            if (*token[1]=='o') /* OF Packet Out*/
            {
                printf("\r\nDrvP: Received Packet Out command = \"%s\".\r\n\r\n", saved_ptr);

        /* OF packet out tests, reads the packet content from a file
         * src_port,DA-DA-DA-DA-DA-DA-SA-SA-SA-SA-SA-SA-XX-XX-XX-XX-XX-XX-XX-XX-XX-XX.
                 * Note: end with a period "."
                 */
                fd=fopen("/flash/packet_out", "r");
                if (fd == NULL)
                {
                    printf("\r\nCan't open /flash/packet_out file!\r\n");
                }
                else
                {
                    read_len = fread(rxbuf, 1, BUF_LEN, fd);
                    printf("r\nTest OF packet out to pipeline. reads %d bytes from /flash/packet_out\r\n", read_len);
                    token[0] = strtok_r(rxbuf, ",", &saved_ptr);
                    token[1] = strtok_r(NULL, ".", &saved_ptr);
                    printf("src port=%s, pkt=\"%s\"\r\n", token[0], token[1]);
                    read_len = DRIVER_PROC_HexStringTo_UI8T_Values((UI8_T *) packet, token[1]);
                    printf("Packet content after HexString To UI8T conversion: (length=%d)\r\n", read_len);
                    for (i=0; i<read_len; i++)
                    {
                        printf("0x%02x-", (UI8_T) packet[i]);
                        if ( ((i+1)%8) == 0 )
                        {
                            printf("\r\n");
                        }
                    }
                    printf("------------------------------------------------------\r\n");
                    printf("------DEV_NICDRV_GATEWAY_SendPacketToPipeline---------\r\n");
                    DEV_NICDRV_GATEWAY_SendPacketToPipeline(atoi(token[0]), (UI8_T *) packet, read_len, NULL);
                    printf("------------------------------------------------------\r\n");
                    fclose(fd);
                }
            }
            else
            {
                printf("\r\nUnknown command!\r\n");
                DRIVER_PROC_DbgThreadCmd();
            }
            break;
        case 'n': /* NIC backdoors */
            printf("\r\nDrvP: Received NIC backdoor command = \"%s\".\r\n\r\n", saved_ptr);
            if (strlen(saved_ptr)>0)
            {
                token[1] = strtok_r(NULL, " ", &saved_ptr);
                switch ((char)(*token[1]))
                {
                    case '1': /* dev_nicdrv_gateway backdoor */
                        DEV_NICDRV_GATEWAY_BackDoor_Menu();
                        break;
                    case '2': /* dev_nicdrv backdoor */
                        DEV_NICDRV_Backdoor();
                        break;
                    case '3': /* lan backdoor */
                        LAN_BackdoorEntrance();
                        break;
                    default:
                        printf("\r\nUnknown command!\r\n");
                        DRIVER_PROC_DbgThreadCmd();
                        break;
                }
            }
            break;
        default:
            printf("\r\nUnknown command!\r\n");
            DRIVER_PROC_DbgThreadCmd();
            break;
       }
       SYSFUN_SendToIPCSocket(sockfd, SERVER_ADDR, "done",  5);
    }
    free(rxbuf);
    free(packet);
    return;
}

#endif

