/* MODULE NAME:  UTILITY_GROUP.c
 * PURPOSE:
 *     Files for utility group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/06/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#if 0 /* turn on for doing EH unit test, also need to turn on EH_UNIT_TEST in eh_mgr.c */
#define EH_UNIT_TEST 1
#endif

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "utility_group.h"
#include "core_util_proc_comm.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l_cmnlib_init.h"
#include "uc_mgr.h"

#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_init.h"
#include "smtp_mgr.h"
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_init.h"
#include "syslog_mgr.h"
#endif

#if (SYS_CPNT_RESETMGMT == TRUE)
#include "resetmgmt_mgr.h"
#define UTITLITY_GROUP_RESETMGMT_TIMER_INTERVAL SYS_BLD_TICKS_PER_SECOND
#endif

#include "sysfun_backdoor.h"
#include "l_ipcmem_backdoor.h"
#include "l_mm_backdoor.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define UTILITY_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(UTILITY_GROUP_MGR_MSG_T)

/* union all data type used for utility Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union UTILITY_GROUP_MGR_MSG_U
{

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_MGR_IPCMsg_T syslog_mgr_ipcmsg;
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_MGR_IPCMsg_T   smtp_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_ipcmsg;
} UTILITY_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define UTITLITY_GROUP_RESETMGMT_EVENT_TIMER  BIT_1
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void UTILITY_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void UTILITY_GROUP_SetTransitionMode(void);
static void UTILITY_GROUP_EnterTransitionMode(void);
static void UTILITY_GROUP_EnterMasterMode(void);
static void UTILITY_GROUP_EnterSlaveMode(void);
static void UTILITY_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);


/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for UTILITY_GROUP mgr thread
 */
static UI8_T UTILITY_GROUP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(UTILITY_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
#if defined(EH_UNIT_TEST)
#include "eh_mgr.h"
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UTILITY_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for UTILITY_Group.
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
void UTILITY_GROUP_Create_InterCSC_Relation(void)
{
    /* register backdoor func for CSCs
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sysfun", SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, SYSFUN_BACKDOOR_BackDoorMenu);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("k_ipcmem", SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, L_IPCMEM_BACKDOOR_BackDoorMenu);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("k_mm", SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, K_L_MM_BACKDOOR_BackDoorMenu);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("l_mm", SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, L_MM_BACKDOOR_BackDoorMenu);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sys_callback", SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, SYS_CALLBACK_MGR_BackDoorMenu);

#if defined(EH_UNIT_TEST)
    eh_unit_test_register_backdoor(SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY);
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_Create_InterCSC_Relation();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in UTILITY_GROUP.
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
void UTILITY_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_Create_Tasks();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_UTILITY_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_UTILITY_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_UTILITY_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          UTILITY_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn UTILITY_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_UTILITY_GROUP, thread_id, SYS_ADPT_UTILITY_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will call all CSC in UTILITY_GROUP inform that provision complete
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
void UTILITY_GROUP_ProvisionComplete(void)
{

    SYSLOG_MGR_NotifyProvisionComplete();

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_InitiateProcessResource
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
void UTILITY_GROUP_InitiateProcessResource(void)
{
#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_InitiateSystemResources();
#endif
}


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of UTILITY_GROUP.
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
static void UTILITY_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = CORE_UTIL_PROC_COMM_GetUTILITY_GROUPTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)UTILITY_GROUP_mgrtd_ipc_buf;
    BOOL_T               need_resp;
#if (SYS_CPNT_RESETMGMT == TRUE)
    void                 *timer_id;
#endif
    /* join the thread group of UTILITY_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_UTILITY_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }
#if (SYS_CPNT_RESETMGMT == TRUE)
    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, UTITLITY_GROUP_RESETMGMT_TIMER_INTERVAL,
            UTITLITY_GROUP_RESETMGMT_EVENT_TIMER) == FALSE)
    {
        printf("\r\n%s: Start timer failed!\r\n", __FUNCTION__);
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
#if (SYS_CPNT_RESETMGMT == TRUE)
                             UTITLITY_GROUP_RESETMGMT_EVENT_TIMER |
#endif
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            UTILITY_GROUP_SetTransitionMode();
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
                UTILITY_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_SMTP == TRUE)
                    case SYS_MODULE_SMTP:
                        need_resp=SMTP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
                    case SYS_MODULE_SYSLOG:
                        need_resp=SYSLOG_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        UTILITY_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        UTILITY_GROUP_EnterMasterMode();

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
                        UTILITY_GROUP_EnterSlaveMode();

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
                        UTILITY_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        UTILITY_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
#endif
                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:
                        SYSLOG_MGR_SaveUcLogsToFlash();

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
#if (SYS_CPNT_RESETMGMT == TRUE)
        if((local_events & UTITLITY_GROUP_RESETMGMT_EVENT_TIMER))
        {
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

            RESETMGMT_MGR_HandleTimerEvents();
            local_events ^= UTITLITY_GROUP_RESETMGMT_EVENT_TIMER;
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
        }
#endif
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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_UTILITY_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in UTILITY_GROUP.
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
static void UTILITY_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_RESETMGMT == TRUE)
    RESETMGMT_MGR_SetTransitionMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in UTILITY_GROUP.
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
static void UTILITY_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_RESETMGMT == TRUE)
    RESETMGMT_MGR_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in UTILITY_GROUP.
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
static void UTILITY_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_RESETMGMT == TRUE)
    RESETMGMT_MGR_EnterMasterMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in UTILITY_GROUP.
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
static void UTILITY_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SMTP == TRUE)
    SMTP_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_RESETMGMT == TRUE)
    RESETMGMT_MGR_EnterSlaveMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in UTILITY_GROUP.
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
static void UTILITY_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
       case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYSLOG_MGR_RifUp_CallBack);
            break;

       case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SYSLOG_MGR_RifDown_CallBack);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}
