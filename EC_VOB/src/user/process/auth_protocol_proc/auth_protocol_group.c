/* MODULE NAME:  auth_protocol_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of auth_protocol group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
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
#include "auth_protocol_group.h"
#include "auth_protocol_proc_comm.h"
#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_init.h"
#include "tacacs_mgr.h"
#endif
#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#endif
#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_init.h"
#include "radius_mgr.h"
#endif
#if (SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE)
#include "security_backdoor.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union AUTH_PROTOCOL_GROUP_MGR_MSG_U
{

#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_IPCMsg_T aaa_mgr_ipcmsg;
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_MGR_IPCMsg_T radius_mgr_ipcmsg;
#endif

#if (SYS_CPNT_TACACS == TRUE)
    TACACS_MGR_IPCMsg_T tacacs_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} AUTH_PROTOCOL_GROUP_MGR_MSG_T;

#define AUTH_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(AUTH_PROTOCOL_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void AUTH_PROTOCOL_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void AUTH_PROTOCOL_GROUP_SetTransitionMode(void);
static void AUTH_PROTOCOL_GROUP_EnterTransitionMode(void);
static void AUTH_PROTOCOL_GROUP_EnterMasterMode(void);
static void AUTH_PROTOCOL_GROUP_EnterSlaveMode(void);
static void AUTH_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for auth_protocol_group mgr thread
 */
static UI8_T auth_protocol_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(AUTH_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static SYSFUN_MsgQ_T ipc_msgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_InitiateProcessResources
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
void AUTH_PROTOCOL_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_Initiate_System_Resources();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_InitiateProcessResources();
#endif

#if (SYS_CPNT_TACACS == TRUE)
    TACACS_INIT_Initiate_System_Resources();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AUTH_PROTOCOL_GROUP_Create_InterCSC_Relation
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
void AUTH_PROTOCOL_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_TACACS == TRUE)
    TACACS_INIT_Create_InterCSC_Relation();
#endif
#if (SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE)
    SECURITY_Backdoor_Register_SubsysBackdoorFunc();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_Create_All_Threads
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
void AUTH_PROTOCOL_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_AUTH_PROTOCOL_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_AUTH_PROTOCOL_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_AUTH_PROTOCOL_GROUP_MGR_THREAD_NAME,
#ifdef ES3526MA_POE_7LF_LN
                          (SYS_BLD_TASK_COMMON_STACK_SIZE * 8),
#else
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
#endif
                          SYSFUN_TASK_FP,
                          AUTH_PROTOCOL_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn AUTH Protocol group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_AUTH_PROTOCOL_GROUP, thread_id, SYS_ADPT_AUTH_PROTOCOL_GROUP_SW_WATCHDOG_TIMER);
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    TACACS_INIT_Create_Tasks();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_SendAuthCheckResponseMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Sends response message of authentication check request.
 *
 * INPUT:
 *    result    -- Authentication result
 *    privilege -- Privilege of the user
 *    cookie    -- Cookie (message type of the response)
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
void AUTH_PROTOCOL_GROUP_SendAuthCheckResponseMsg(I32_T result, I32_T privilege,
    UI32_T cookie)
{
#if (SYS_CPNT_RADIUS == TRUE)
    UI8_T radius_mgr_ipcmsg[SYSFUN_SIZE_OF_MSG(sizeof(RADIUS_MGR_IPCMsg_T))];
    SYSFUN_Msg_T *msgbuf_p;
    RADIUS_MGR_IPCMsg_T *msg_data_p;

    msgbuf_p = (SYSFUN_Msg_T *)radius_mgr_ipcmsg;
    msgbuf_p->msg_type = cookie;
    msgbuf_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();

    msg_data_p = (RADIUS_MGR_IPCMsg_T *)msgbuf_p->msg_buf;
    msg_data_p->type.result = result;

    if (SYSFUN_OK != SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p))
    {
        printf("%s: SYSFUN_SendResponseMsg fail.\r\n", __FUNCTION__);
    }
#endif /* #if (SYS_CPNT_RADIUS == TRUE) */
}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_Mgr_Thread_Function_Entry
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
static void AUTH_PROTOCOL_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    L_THREADGRP_Handle_T tg_handle =AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)auth_protocol_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_AUTH_PROTOCOL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

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
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            AUTH_PROTOCOL_GROUP_SetTransitionMode();
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
                AUTH_PROTOCOL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_TACACS == TRUE)
                    case SYS_MODULE_TACACS:
                        need_resp=TACACS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_AAA == TRUE)
                    case SYS_MODULE_AAA:
                        need_resp=AAA_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_RADIUS == TRUE)
                    case SYS_MODULE_RADIUS:
                        need_resp=RADIUS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        AUTH_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        AUTH_PROTOCOL_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer auth_protocol group has
                         * entered transition mode but lower layer auth_protocol groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer auth_protocol group. In this case, the IPCFAIL
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
                        AUTH_PROTOCOL_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer auth_protocol group has
                         * entered transition mode but lower layer auth_protocol group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer auth_protocol group. In this case, the IPCFAIL
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
                        AUTH_PROTOCOL_GROUP_EnterTransitionMode();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
#endif
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:
#if (SYS_CPNT_ACCOUNTING == TRUE)
                        RADIUS_MGR_Reload_CallBack();
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
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
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_AUTH_PROTOCOL_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_SetTransitionMode
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
static void AUTH_PROTOCOL_GROUP_SetTransitionMode(void)
{

#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_SetTransitionMode();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_TACACS == TRUE)
    TACACS_INIT_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_EnterTransitionMode
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
static void AUTH_PROTOCOL_GROUP_EnterTransitionMode(void)
{

#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_EnterTransitionMode();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_TACACS== TRUE)
    TACACS_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_EnterMasterMode
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
static void AUTH_PROTOCOL_GROUP_EnterMasterMode(void)
{

#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_EnterMasterMode();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_TACACS == TRUE)
    TACACS_INIT_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_EnterSlaveMode
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
static void AUTH_PROTOCOL_GROUP_EnterSlaveMode(void)
{

#if (SYS_CPNT_AAA == TRUE)
    AAA_MGR_EnterSlaveMode();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_TACACS== TRUE)
    TACACS_INIT_EnterSlaveMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_HandleSysCallbackIPCMsg
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
static void AUTH_PROTOCOL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

