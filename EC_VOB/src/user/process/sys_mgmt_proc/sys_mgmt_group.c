/* MODULE NAME:  sysmgmt_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of sysmgmt group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/17/2007 - Echo Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "sys_mgmt_group.h"
#include "sys_mgmt_proc_comm.h"

#include "sys_mgr.h"
#if (SYS_CPNT_SYSMGMT == TRUE)
#include "sysmgmt_init.h"
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
#include "if_mgr.h"
#include "mib2_mgr.h"
#include "mib2_pom.h"
#endif

#include "userauth.h"
#include "snmp_pmgr.h"
#include "netcfg_pmgr_main.h"

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif


#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_group.h"
#endif

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#endif /* SYS_CPNT_DEBUG */

#define SYS_MGMT_GROUP_DEBUG_ENABLE TRUE

#if (SYS_MGMT_GROUP_DEBUG_ENABLE == TRUE)

#define SYS_MGMT_GROUP_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define SYS_MGMT_GROUP_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}
#else
#define SYS_MGMT_GROUP_DEBUG_LINE()
#define SYS_MGMT_GROUP_DEBUG_MSG(a,b...)
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union SYSMGMT_GROUP_MGR_MSG_U
{

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_IPCMsg_T mib2_msg;
    IF_MGR_IPCMsg_T if_msg;
#endif

    USERAUTH_IPCMsg_T userauth_msg;

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
   // MGMT_IP_FLT_MGR_IPCMsg_T mgmt_ip_flt_msg;
#endif

    SYS_MGR_IPCMsg_T    sys_msg;
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} SYSMGMT_GROUP_MGR_MSG_T;

#define SYSMGMT_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SYSMGMT_GROUP_MGR_MSG_T)

#define SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_TICKS (1 * SYS_BLD_TICKS_PER_SECOND)
#define SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_EVENT BIT_0

#define SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_TIMER_SEC 1
#define SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_EVENT BIT_1

/*maggie liu for RADIUS authentication ansync*/
SYSFUN_MsgQ_T    sys_mgmt_group_ipc_msgq_handle;


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYS_MGMT_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void SYS_MGMT_GROUP_SetTransitionMode(void);
static void SYS_MGMT_GROUP_EnterTransitionMode(void);
static void SYS_MGMT_GROUP_EnterMasterMode(void);
static void SYS_MGMT_GROUP_EnterSlaveMode(void);
static void SYS_MGMT_GROUP_ProvisionComplete(void);
static void SYS_MGMT_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for sysmgmt_group mgr thread
 */
static UI8_T sysmgmt_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SYSMGMT_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
static void *sysmgmt_mon_timer_id;
#endif
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
static void *sysmgmt_reload_timer_id;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_GROUP_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  CSC Group.
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
 *------------------------------------------------------------------------------
 */
void SYS_MGMT_GROUP_InitiateProcessResources(void)
{

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_InitiateSystemResources();
    MIB2_POM_Init();
    IF_MGR_InitiateSystemResources();
#endif

#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_InitiateProcessResource();
#endif

    USERAUTH_Init();
    SNMP_PMGR_Init();
    NETCFG_PMGR_MAIN_InitiateProcessResource();

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#endif

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGMT_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SWCTRL Group.
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
void SYS_MGMT_GROUP_Create_InterCSC_Relation(void)
{

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_Create_InterCSC_Relation();
    IF_MGR_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_Create_InterCSC_Relation();
#endif

    USERAUTH_Create_InterCSC_Relation();

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    MGMT_IP_FLT_INIT_Create_InterCSC_Relation();
#endif

}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup1.
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
 *    All threads in the same CSC group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void SYS_MGMT_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_SYSMGMT_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_SYSMGMT_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_SYSMGMT_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SYS_MGMT_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CSCGroup1 MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SYS_MGMT_GROUP, thread_id, SYS_ADPT_SYS_MGMT_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSMGMT_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of CSCGroup1.
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
static void SYS_MGMT_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    L_THREADGRP_Handle_T tg_handle =SYS_MGMT_PROC_COMM_Get_SYSMGMT_MGR_GROUPTGHandle();
    UI32_T               member_id,wait_events,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)sysmgmt_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    wait_events = SYSFUN_SYSTEM_EVENT_IPCFAIL |
                  SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_SYSMGMT_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &sys_mgmt_group_ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    if ((sysmgmt_mon_timer_id = SYSFUN_PeriodicTimer_Create()) == NULL)
    {
        printf("%s: SYSFUN_PeriodicTimer_Create fail.\n", __FUNCTION__);
        return;
    }

    SYSFUN_PeriodicTimer_Start(sysmgmt_mon_timer_id,
                               SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_TICKS,
                               SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_EVENT);

    wait_events |= SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_EVENT;
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    if ((sysmgmt_reload_timer_id = SYSFUN_PeriodicTimer_Create()) == NULL)
    {
        printf("%s: SYSFUN_PeriodicTimer_Create fail.\n", __FUNCTION__);
        return;
    }

    SYSFUN_PeriodicTimer_Start(sysmgmt_reload_timer_id,
                               SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_TIMER_SEC * SYS_BLD_TICKS_PER_SECOND,
                               SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_EVENT);

    wait_events |= SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_EVENT;
#endif

    /* main loop:
     *    while(1)
     *    {
     *
     *        Wait event
     *            Handle SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE event if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCMSG if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCFAIL if any
     *    }
     */
    while(1)
    {

        SYSFUN_ReceiveEvent (wait_events,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            SYS_MGMT_GROUP_SetTransitionMode();
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
            if(SYSFUN_ReceiveMsg(sys_mgmt_group_ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                SYSMGMT_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        SYS_MGMT_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_MIB2MGMT==TRUE)
                    case SYS_MODULE_MIB2MGMT:
                        need_resp=MIB2_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_IFMGR:
                        need_resp=IF_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_USERAUTH:
                        need_resp=USERAUTH_HandleIPCReqMsg(msgbuf_p);
                        break;


                    case SYS_MODULE_SYSMGMT:
                        need_resp= SYS_MGR_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        SYS_MGMT_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer sysmgmt group has
                         * entered transition mode but lower layer sysmgmt groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer sysmgmt group. In this case, the IPCFAIL
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
                        SYS_MGMT_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer sysmgmt group has
                         * entered transition mode but lower layer sysmgmt group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer sysmgmt group. In this case, the IPCFAIL
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
                        SYS_MGMT_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        SYS_MGMT_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
#endif
                    /*add by fen.wang ,to process system reload msg,it is sent by stkctrl task*/
                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    default:
                        /* Unknow command. There is no way to idntify whether this
                         * ipc message need or not need a response. If we response to
                         * a asynchronous msg, then all following synchronous msg will
                         * get wrong responses and that might not easy to debug.
                         * If we do not response to a synchronous msg, the requester
                         * will be blocked forever. It should be easy to debug that
                         * error.
                         */
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp=FALSE;
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(sys_mgmt_group_ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
        if (local_events & SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_EVENT)
        {
            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

            #if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
            SYS_MGR_CpuUtilizationMonitoringProcess();
            #endif

            #if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
            SYS_MGR_MemoryUtilizationMonitoringProcess();
            #endif


            /* release thread group execution permission
             */
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

            local_events ^= SYSMGMT_GROUP_SYSTEM_RESOURCE_REFRESH_EVENT;
        }
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
        if (local_events & SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_EVENT)
        {
            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

            SYS_RELOAD_MGR_TimeHandler(SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_TIMER_SEC);

            /* release thread group execution permission
             */
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

            local_events ^= SYSMGMT_GROUP_RELOAD_SYSTEM_COUNTDOWN_EVENT;
        }
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */

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

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SYS_MGMT_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in CSCGroup1.
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
static void SYS_MGMT_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_SetTransitionMode();
    IF_MGR_SetTransitionMode();
#endif
    USERAUTH_SetTransitionMode();
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    MGMT_IP_FLT_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_DEBUG == TRUE)
	DEBUG_MGR_SetTransitionMode();
#endif /* SYS_CPNT_DEBUG */

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in CSCGroup1.
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
static void SYS_MGMT_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_EnterTransitionMode();
    IF_MGR_EnterTransitionMode();
#endif
    USERAUTH_EnterTransitionMode();
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    MGMT_IP_FLT_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_DEBUG == TRUE)
	DEBUG_MGR_EnterTransitionMode();
#endif /* SYS_CPNT_DEBUG */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in CSCGroup1.
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
static void SYS_MGMT_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_EnterMasterMode();
    IF_MGR_EnterMasterMode();
#endif
    USERAUTH_EnterMasterMode();
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    MGMT_IP_FLT_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_DEBUG == TRUE)
	DEBUG_MGR_EnterMasterMode();
#endif /* SYS_CPNT_DEBUG */
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set ProvisionComplete mode function in CSCGroup1.
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
static void SYS_MGMT_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_ProvisionComplete();
    IF_MGR_ProvisionComplete();
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_GROUP_ProvisionComplete();
#endif

    USERAUTH_ProvisionComplete();
}
/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CSCGroup1.
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
static void SYS_MGMT_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SYSMGMT == TRUE)
    SYSMGMT_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_MGR_EnterSlaveMode();
    IF_MGR_EnterSlaveMode();
#endif
    USERAUTH_EnterSlaveMode();
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    MGMT_IP_FLT_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_DEBUG == TRUE)
	DEBUG_MGR_EnterSlaveMode();
#endif /* SYS_CPNT_DEBUG */
}

void SYS_MGMT_GROUP_HandleRadiusAuthenResult(I32_T result, I32_T privilege, UI32_T cookie)
{
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)sysmgmt_group_mgrtd_ipc_buf;
    USERAUTH_IPCMsg_T *msg_data_p;

    msg_data_p = (USERAUTH_IPCMsg_T *)msgbuf_p->msg_buf;

    msgbuf_p->msg_type = cookie;
    msgbuf_p->msg_size = (sizeof(((USERAUTH_IPCMsg_T *)0)->data.name_password_priv)
                + USERAUTH_MSGBUF_TYPE_SIZE);
    msg_data_p->type.result_i32 = result;
    msg_data_p->data.name_password_priv.priv = privilege;

    if(SYSFUN_SendResponseMsg(sys_mgmt_group_ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK)
        printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

    return;
}

void SYS_MGMT_GROUP_HandleRadiusAuthenResult_2(
    I32_T result,
    UI32_T privilege,
    void *cookie,
    UI32_T cookie_size)
{
    SYSFUN_Msg_T        *msgbuf_p=(SYSFUN_Msg_T*)sysmgmt_group_mgrtd_ipc_buf;
    USERAUTH_IPCMsg_T   *msg_data_p;

    USERAUTH_AsyncAuthParam_T *param_p = (USERAUTH_AsyncAuthParam_T *)cookie;
    USERAUTH_ReturnValue_T    ret = USERAUTH_REJECT;

    if (cookie_size != sizeof(*param_p))
    {
        printf("Unknown result. Size(%lu)", cookie_size);
        return;
    }

    param_p->result.status = result;
    param_p->result.privilege = privilege;

    ret = USERAUTH_ProcessAuthResult(param_p);

    if (USERAUTH_PROCESSING == ret)
    {
        return;
    }

    msg_data_p = (USERAUTH_IPCMsg_T *)msgbuf_p->msg_buf;

    msgbuf_p->msg_type = param_p->cookie;
    msgbuf_p->msg_size = (sizeof(((USERAUTH_IPCMsg_T *)0)->data.auth_result)
                + USERAUTH_MSGBUF_TYPE_SIZE);
    msg_data_p->type.result_bool = (ret == USERAUTH_PASS) ? TRUE : FALSE;

    msg_data_p->data.auth_result.privilege = param_p->result.privilege;
    msg_data_p->data.auth_result.authen_by_whom = param_p->result.authen_by_whom;

    if (SYSFUN_SendResponseMsg(sys_mgmt_group_ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK)
        printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : sysmgmt_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in CSCGroup1.
 *
 * INPUT:
 *    msgbuf_p  --  SYS_CALLBACK IPC message
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
static void SYS_MGMT_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        /*maggie liu for RADIUS authentication ansync*/
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT:
             SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGMT_GROUP_HandleRadiusAuthenResult);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT_2:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGMT_GROUP_HandleRadiusAuthenResult_2);
            break;

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
		case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_ALARM_INPUT_STATUS_CHANGED:
			 SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_AlarmInputStatusChanged_CallBack);
            break;
#endif
#if (SYS_CPNT_ALARM_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MAJOR_ALARM_OUTPUT_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_MajorAlarmOutputStatusChanged_CallBack);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MINOR_ALARM_OUTPUT_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_MinorAlarmOutputStatusChanged_CallBack);
            break;
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
		case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED:
			 SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_PowerStatusChanged_CallBack);
            break;
#endif
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
		case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_TYPE_CHANGED:
			 SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_PowerTypeChanged_CallBack);
            break;
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_FanStatusChanged_CallBack);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_SPEED_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_FanSpeedChanged_CallBack);
            break;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_THERMAL_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYS_MGR_ThermalStatusChanged_CallBack);
            break;
#endif
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}
