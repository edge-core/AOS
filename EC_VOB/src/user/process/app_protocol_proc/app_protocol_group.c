/* MODULE NAME:  app_protocol_group.c
 * PURPOSE:
 *     Files for app protocol group.
 *
 * NOTES:
 *
 * HISTORY
 *    11/13/2007 - Squid Ro, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_callback_mgr.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "l_cmnlib_init.h"

#include "app_protocol_proc_comm.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_SNTP == TRUE)
    #include "sntp_init.h"
    #include "sntp_mgr.h"
#endif

#if (SYS_CPNT_NTP == TRUE)
    #include "ntp_init.h"
    #include "ntp_mgr.h"
#endif

#if (SYS_CPNT_DNS == TRUE)
    #include "dns_init.h"
    #include "dns_mgr.h"
#endif

#if (SYS_CPNT_PING == TRUE)
    #include "ping_init.h"
    #include "ping_mgr.h"
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    #include "traceroute_init.h"
    #include "traceroute_mgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/* NAMING CONSTANT DECLARATIONS
 */
#define APP_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(APP_PROTOCOL_GROUP_MGR_MSG_T)

/* union all data type used for utility Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union APP_PROTOCOL_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_MGR_IPCMsg_T sntp_mgr_ipcmsg;
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_MGR_IPCMsg_T ntp_mgr_ipcmsg;
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_MGR_IPCMsg_T dns_mgr_ipcmsg;
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_MGR_IPCMsg_T ping_mgr_ipcmsg;
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_MGR_IPCMsg_T traceroute_mgr_ipcmsg;
#endif

    struct
    {
        SYS_CALLBACK_MGR_Msg_T sys_callback_msg_header;

        union
        {
            SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T lport_enter_forwarding;
            SYS_CALLBACK_MGR_RifActive_CBData_T rif_active;
        } sys_callback_msg_data;
    } sys_callback_msg;

    BACKDOOR_MGR_Msg_T      backdoor_mgr_ipcmsg;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                     cmgr_ipcmsg;
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
} APP_PROTOCOL_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void APP_PROTOCOL_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void APP_PROTOCOL_GROUP_SetTransitionMode(void);
static void APP_PROTOCOL_GROUP_EnterTransitionMode(void);
static void APP_PROTOCOL_GROUP_EnterMasterMode(void);
static void APP_PROTOCOL_GROUP_EnterSlaveMode(void);
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void APP_PROTOCOL_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
static void APP_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void APP_PROTOCOL_GROUP_RifUpCallbackHandler(
    UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void APP_PROTOCOL_GROUP_LPortEnterForwardingCallbackHandler(
    UI32_T xstp_id, UI32_T lport);
static void APP_PROTOCOL_GROUP_LPortLeaveForwardingCallbackHandler(
    UI32_T xstp_id, UI32_T lport);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for APP_PROTOCOL_GRPUP mgr thread
 */
static UI8_T APP_PROTOCOL_GROUP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(APP_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - APP_PROTOCOL_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for APP_PROTOCOL_GROUP.
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
void APP_PROTOCOL_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_PING == TRUE)
    //PING_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_Create_InterCSC_Relation();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in APP_PROTOCOL_GROUP.
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
 *
 *------------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_Create_Tasks();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_APP_PROTOCOL_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_APP_PROTOCOL_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_APP_PROTOCOL_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          APP_PROTOCOL_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn APP_PROTOCOL_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_APP_PROTOCOL_GROUP, thread_id, SYS_ADPT_APP_PROTOCOL_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will call all CSC in APP_PROTOCOL_GROUP inform that provision complete
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
 *
 *------------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_ProvisionComplete();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init APP_PROTOCOL_GROUP.
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
void APP_PROTOCOL_GROUP_InitiateProcessResource(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_Initiate_System_Resources();
#endif

}


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)APP_PROTOCOL_GROUP_mgrtd_ipc_buf;
    BOOL_T               need_resp;
#if (SYS_CPNT_NTP == TRUE)
    void                 *ntp_timer_id;
#endif

    /* join the thread group of APP_PROTOCOL_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_APP_PROTOCOL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_NTP == TRUE)
    ntp_timer_id = SYSFUN_PeriodicTimer_Create();

    if (SYSFUN_PeriodicTimer_Start(ntp_timer_id, NTP_TYPE_EVENT_TIMER_INTERVAL, NTP_TYPE_EVENT_TIMER) == FALSE)
    {
        printf("\r\n%s: SYSFUN_PeriodicTimer_Start fail.\n", __FUNCTION__);
    }
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

        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_NTP == TRUE)
                             NTP_TYPE_EVENT_TIMER |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                            |SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
                            ,SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            APP_PROTOCOL_GROUP_SetTransitionMode();
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
            if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                APP_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_SNTP == TRUE)
                    case SYS_MODULE_SNTP:
                        need_resp=SNTP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_NTP == TRUE)
                    case SYS_MODULE_NTP:
                        need_resp=NTP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_DNS == TRUE)
                    case SYS_MODULE_DNS:
                        need_resp=DNS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_PING == TRUE)
                    case SYS_MODULE_PING:
                        need_resp=PING_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
                    case SYS_MODULE_TRACEROUTE:
                        need_resp=TRACEROUTE_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        APP_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        APP_PROTOCOL_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                        /* global cmd
                         */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        APP_PROTOCOL_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
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
                        APP_PROTOCOL_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
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
                        APP_PROTOCOL_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        APP_PROTOCOL_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
#endif
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

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

#if (SYS_CPNT_NTP == TRUE)
        /* handle NTP timer evnet
         */
        if (local_events & NTP_TYPE_EVENT_TIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n", __FUNCTION__);
            }

            NTP_MGR_Alarming();

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n", __FUNCTION__);
            }

            local_events ^= NTP_TYPE_EVENT_TIMER;
        }
#endif /* end of #if (SYS_CPNT_NTP == TRUE) */

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
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_APP_PROTOCOL_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_SetTransitionMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_EnterTransitionMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_EnterMasterMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SNTP == TRUE)
    SNTP_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_INIT_EnterSlaveMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in APP_PROTOCOL_GROUP.
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
static void APP_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &APP_PROTOCOL_GROUP_RifUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &APP_PROTOCOL_GROUP_LPortEnterForwardingCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_HandleCmgrIpcMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    CMGR IPC handler function.
 *
 * INPUT:
 *    ipc_msg_p -- CMGR IPC message.
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
static void APP_PROTOCOL_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
{
    CMGR_IpcMsg_T *cmgr_msg_p;

    if (ipc_msg_p == NULL)
    {
        return;
    }

    cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;

    switch (cmgr_msg_p->type.cmd)
    {
        case CMGR_IPC_XSTP_PORT_STATE_CHANGE:
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
            {
                if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
                {
                    APP_PROTOCOL_GROUP_LPortLeaveForwardingCallbackHandler(
                        cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                        cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
                }
                APP_PROTOCOL_GROUP_LPortEnterForwardingCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            else
            {
                APP_PROTOCOL_GROUP_LPortLeaveForwardingCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            break;

        default:
            printf("%s: Invalid CMGR IPC message.\n", __FUNCTION__);
            break;
    }
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_RifUpCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback handler for the event of rif up.
 *
 * INPUT:
 *    ifindex   -- The ifindex of active rif.
 *    addr_p    -- The IP address of active rif.
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
static void APP_PROTOCOL_GROUP_RifUpCallbackHandler(
    UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
#if (SYS_CPNT_NTP == TRUE)
    NTP_MGR_RifUp_Callback(ifindex, addr_p);
#endif /* #if (SYS_CPNT_NTP == TRUE) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_LPortEnterForwardingCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback handler for the event of a lport entering forwarding state.
 *
 * INPUT:
 *    xstid -- Index of the spanning tree.
 *    lport -- Logical port number.
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
static void APP_PROTOCOL_GROUP_LPortEnterForwardingCallbackHandler(
    UI32_T xstp_id, UI32_T lport)
{
#if (SYS_CPNT_NTP == TRUE)
    NTP_MGR_LPortEnterForwarding_Callback(xstp_id, lport);
#endif /* #if (SYS_CPNT_NTP == TRUE) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_LPortLeaveForwardingCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback handler for the event of a lport leaving forwarding state.
 *
 * INPUT:
 *    xstid -- Index of the spanning tree.
 *    lport -- Logical port number.
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
static void APP_PROTOCOL_GROUP_LPortLeaveForwardingCallbackHandler(
    UI32_T xstp_id, UI32_T lport)
{
#if (SYS_CPNT_NTP == TRUE)
    NTP_MGR_LPortLeaveForwarding_Callback(xstp_id, lport);
#endif /* #if (SYS_CPNT_NTP == TRUE) */
}

