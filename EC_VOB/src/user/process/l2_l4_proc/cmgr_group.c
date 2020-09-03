/*-----------------------------------------------------------------------------
 * MODULE NAME: CMGR_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module implements the fucntionality of CMGR group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2010/11/08     --- Wakka Tu, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2010
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "sys_callback_mgr.h"
#include "sys_callback_om.h"
#include "cmgr_group.h"
#include "cmgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "vlan_om.h"
#include "vlan_type.h"
#include "xstp_om.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define CMGR_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(CMGR_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union
{
    SYS_CALLBACK_MGR_LPort_CBData_T                   sys_callback_mgr_lport_cbdata;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T      sys_callback_mgr_l2mux_receive_packet_cbdata;
}   CMGR_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(CMGR_GROUP_Syscallback_CBData_T))];
}   CMGR_GROUP_Syscallback_Msg_T;

typedef union CMGR_GROUP_MgrMsg_U
{
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
    CMGR_GROUP_Syscallback_Msg_T    syscb_ipcmsg;
} CMGR_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void CMGR_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void CMGR_GROUP_SetTransitionMode(void);
static void CMGR_GROUP_EnterTransitionMode(void);
static void CMGR_GROUP_EnterMasterMode(void);
static void CMGR_GROUP_EnterSlaveMode(void);
static void CMGR_GROUP_ProvisionComplete(void);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void CMGR_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void CMGR_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
static void CMGR_GROUP_HandleSysCallbackIPCMsg(
    SYSFUN_Msg_T    *msgbuf_p);
static void CMGR_GROUP_SetPortStatusCallbackHandler(
    UI32_T ifindex, BOOL_T status, UI32_T reason);
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T CMGR_GROUP_ProcessCallback();
#endif


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for group mgr thread
 */
static UI8_T cmgr_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(CMGR_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static UI32_T vlan_change_msgqkey_list[] =
{
#if (SYS_CPNT_BRIDGE == TRUE)
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_CFM == TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static UI32_T vlan_member_change_msgqkey_list[] =
{
#if (SYS_CPNT_BRIDGE == TRUE)
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_CFM == TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static UI32_T gvrp_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T l3_vlan_destroy_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
    0
};

static UI32_T port_vlan_change_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T port_vlan_mode_change_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static UI32_T pvid_change_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T l3_if_oper_status_change_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
    0
};

static UI32_T vlan_name_change_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T protocol_vlan_change_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T vlan_member_tag_change_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

static UI32_T xstp_port_state_msgqkey_list[] =
{
#if (SYS_CPNT_CFM == TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_NTP == TRUE)
    SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
#endif /* #if (SYS_CPNT_NTP == TRUE) */
    0
};

static UI32_T xstp_version_msgqkey_list[] =
{
    0
};

static UI32_T xstp_port_topo_msgqkey_list[] =
{
    0
};

static UI32_T vid = 0;
static UI32_T gvrp_vid = 0;
static UI32_T l3_vid = 0;
static UI32_T port_vlan_lport = 0;
static UI32_T port_vlan_mode_lport = 0;
static UI32_T pvid_lport = 0;
static UI32_T vlan_name_vid = 0;
static UI32_T protocol_vlan_lport = 0;
static UI32_T vlan_member_tag_ar[2] = {0}; /* [0]: vlan_index; [1]: lport */
static UI32_T xstp_port_state_ar[2] = {0};
static UI32_T xstp_port_topo_ar[2] = {0};
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */


/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate process resource.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_InitiateProcessResources(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: Create inter-CSC relationships.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_Create_InterCSC_Relation(void)
{
    CMGR_Create_InterCSC_Relation();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will spawn all threads.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_CMGR_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_CMGR_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_CMGR_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           CMGR_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn CMGR Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_CMGR_GROUP, thread_id, SYS_ADPT_CMGR_GROUP_SW_WATCHDOG_TIMER);
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    SYS_CALLBACK_OM_SetCmgrThreadId(thread_id);
#endif
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE: This is the entry function for the MGR thread.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = NULL; /* never take care protection of transaction */
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)cmgr_mgrtd_ipc_buf;
    UI32_T                  all_events =
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                                CMGR_EVENT_CALLBACK |
#endif
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;
    BOOL_T                  need_resp;

    /* join the thread group
     */
    if (tg_handle != NULL)
    {
        if (L_THREADGRP_Join(tg_handle, SYS_BLD_CMGR_GROUP_MGR_THREAD_PRIORITY,
                &member_id) == FALSE)
        {
            printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
            return;
        }
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CMGR_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    /* main loop:
     *    while(1)
     *    {
     *        Wait event
     *            Handle SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE event if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCMSG if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCFAIL if any
     *    }
     */
    while (1)
    {
        SYSFUN_ReceiveEvent(all_events,
                            SYSFUN_EVENT_WAIT_ANY,
                            (local_events == 0) ? SYSFUN_TIMEOUT_WAIT_FOREVER
                                                : SYSFUN_TIMEOUT_NOWAIT,
                            &received_events);
        local_events |= received_events;

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            CMGR_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

            /* need not to do IPCFAIL recovery in transition mode
             */
            if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
            {
                /* clear fail info in IPCFAIL
                 */
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
            }
        }

        /* handle IPCMSG
         */
        if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            UI32_T  ret;

            ret = SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    CMGR_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p);

            if (ret == SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (tg_handle != NULL)
                {
                    if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    {
                        printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                            __FUNCTION__);
                    }
                }

                /* handle request message based on cmd
                 */
                switch (msgbuf_p->cmd)
                {
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        CMGR_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        CMGR_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        CMGR_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        CMGR_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        CMGR_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        CMGR_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        msgbuf_p->msg_size=0;
                        need_resp = TRUE;
                        break;

                    /* module id cmd
                     */
                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        CMGR_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
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
                        printf("\n%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp = FALSE;
                } /* end of switch (msgbuf_p->cmd) */

                /* release thread group execution permission
                 */
                if (tg_handle != NULL)
                {
                    if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    {
                        printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                            __FUNCTION__);
                    }
                }

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(
                        ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
                {
                    printf("\n%s: SYSFUN_SendResponseMsg fail.\n",
                        __FUNCTION__);
                }
            } /* end of if (SYSFUN_ReceiveMsg... */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end of if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
        if (local_events & CMGR_EVENT_CALLBACK)
        {
            if (CMGR_GROUP_ProcessCallback() == FALSE)
            {
                local_events ^= CMGR_EVENT_CALLBACK;
            }
        }
#endif

        /* handle IPC Async Callback fail when IPC Msgq is empty
         */
        if ((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_CMGR_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while (1) */
} /* End of CMGR_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all set transition mode function.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_SetTransitionMode(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all enter transition mode function.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_EnterTransitionMode(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all set master mode function.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_EnterMasterMode(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all enter slave mode function.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_EnterSlaveMode(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all provision complete function.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_ProvisionComplete(void)
{
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_HandleHotInsertion
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke a new dut insertion.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

    /* XXX_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default); */
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_HandleHotRemoval
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke a dut removal.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

    /* XXX_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default); */
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will handle all callbacks from IPC messages.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SET_PORT_STATUS:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                CMGR_GROUP_SetPortStatusCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_SetPortStatusCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: call-back function, when set the port admin status
 * INPUT  : ifindex -- which logical port
 *          status
 *          reason
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void CMGR_GROUP_SetPortStatusCallbackHandler(
    UI32_T ifindex, BOOL_T status, UI32_T reason)
{
    CMGR_SetPortStatus(ifindex, status, reason);
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T CMGR_GROUP_HandleNormalVlan()
{
    SYS_CALLBACK_OM_Vlan_T  vlan_db;
    UI32_T                  vlan_ifindex;
    UI32_T                  i, j;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_VLAN, &vid,
            &vlan_db) == FALSE)
    {
        return FALSE;
    }

    vlan_ifindex = vid + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;

    if (vlan_db.vlan_change == TRUE)
    {
        CMGR_NotifyVlanChange(vlan_change_msgqkey_list,
            CMGR_IPC_VLAN_CHANGE, vlan_ifindex, VLAN_TYPE_VLAN_STATUS_NONE,
            vlan_db.vlan_state, vlan_db.vlan_merge);

        if (vlan_db.vlan_state == FALSE)
        {
            return TRUE;
        }
    }

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if (vlan_db.member_change[i] == 0)
        {
            continue;
        }
        for (j = 0; j < 8; j++)
        {
            if (vlan_db.member_change[i] & (1 << (7 - j)))
            {
                BOOL_T  existed, merged;

                if (vlan_db.member_state[i] & (1 << (7 - j)))
                {
                    existed = TRUE;
                }
                else
                {
                    existed = FALSE;
                }
                if (vlan_db.member_merge[i] & (1 << (7 - j)))
                {
                    merged = TRUE;
                }
                else
                {
                    merged = FALSE;
                }

                CMGR_NotifyVlanMemberChange(vlan_member_change_msgqkey_list,
                    CMGR_IPC_VLAN_MEMBER_CHANGE, vlan_ifindex, i*8+j+1,
                    VLAN_TYPE_VLAN_STATUS_NONE, existed, merged);
            }
        } /* end for j */
    } /* end for i */

    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleGvrpVlan()
{
    SYS_CALLBACK_OM_Vlan_T  vlan_db;
    UI32_T                  vlan_ifindex;
    UI32_T                  i, j;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_GVRP_VLAN,
            &gvrp_vid, &vlan_db) == FALSE)
    {
        return FALSE;
    }

    vlan_ifindex = gvrp_vid + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;

    if (vlan_db.vlan_change == TRUE)
    {
        CMGR_NotifyVlanChange(gvrp_msgqkey_list, CMGR_IPC_GVRP_VLAN_CHANGE,
            vlan_ifindex, VLAN_TYPE_VLAN_STATUS_NONE, vlan_db.vlan_state,
            vlan_db.vlan_merge);

        if (vlan_db.vlan_state == FALSE)
        {
            return TRUE;
        }
    }

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if (vlan_db.member_change[i] == 0)
        {
            continue;
        }
        for (j = 0; j < 8; j++)
        {
            if (vlan_db.member_change[i] & (1 << (7 - j)))
            {
                BOOL_T  existed, merged;

                if (vlan_db.member_state[i] & (1 << (7 - j)))
                {
                    existed = TRUE;
                }
                else
                {
                    existed = FALSE;
                }
                if (vlan_db.member_merge[i] & (1 << (7 - j)))
                {
                    merged = TRUE;
                }
                else
                {
                    merged = FALSE;
                }

                CMGR_NotifyVlanMemberChange(gvrp_msgqkey_list,
                    CMGR_IPC_GVRP_VLAN_MEMBER_CHANGE, vlan_ifindex, i*8+j+1,
                    VLAN_TYPE_VLAN_STATUS_NONE, existed, merged);
            }
        } /* end for j */
    } /* end for i */

    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleL3Vlan()
{
    SYS_CALLBACK_OM_L3Vlan_T    l3_db;
    UI32_T                      vlan_ifindex;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_L3_VLAN,
            &l3_vid, &l3_db) == FALSE)
    {
        return FALSE;
    }

    vlan_ifindex = l3_vid + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;
    if (l3_db.vlan_destroy == TRUE)
    {
        CMGR_NotifyL3VlanDestroy(l3_vlan_destroy_msgqkey_list, vlan_ifindex,
            VLAN_TYPE_VLAN_STATUS_NONE);
    }
    if (l3_db.oper_status_change == TRUE)
    {
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;

        vlan_entry.dot1q_vlan_index = vlan_ifindex;
        if (VLAN_OM_GetVlanEntry(&vlan_entry) == TRUE)
        {
            CMGR_NotifyL3IfOperStatusChange(
                l3_if_oper_status_change_msgqkey_list, vlan_ifindex,
                vlan_entry.if_entry.vlan_operation_status);
        }
    }

    return TRUE;
}

static BOOL_T CMGR_GROUP_HandlePortVlan()
{
    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_PORT_VLAN,
            &port_vlan_lport, NULL) == FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyPortVlanChange(port_vlan_change_msgqkey_list, port_vlan_lport);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandlePortVlanMode()
{
    VLAN_OM_VlanPortEntry_T port_entry;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(
            SYS_CALLBACK_OM_KIND_PORT_VLAN_MODE, &port_vlan_mode_lport, NULL) ==
            FALSE)
    {
        return FALSE;
    }

    if (VLAN_OM_GetVlanPortEntryByIfindex(port_vlan_mode_lport, &port_entry)
            == TRUE)
    {
        CMGR_NotifyPortVlanModeChange(port_vlan_mode_change_msgqkey_list,
            CMGR_IPC_PORT_VLAN_MODE_CHANGE, port_vlan_mode_lport,
            port_entry.vlan_port_mode);
    }
    else
    {
        printf("%s: failed to get port vlan mode of lport %lu\r\n",
            __func__, (unsigned long) port_vlan_mode_lport);
    }

    return TRUE;
}

static BOOL_T CMGR_GROUP_HandlePvid()
{
    SYS_CALLBACK_OM_Pvid_T  pvid_db;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_PVID,
            &pvid_lport, &pvid_db) == FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyPvidChange(pvid_change_msgqkey_list, pvid_lport,
        pvid_db.old_pvid, pvid_db.new_pvid);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleVlanName()
{
    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_VLAN_NAME,
            &vlan_name_vid, NULL) == FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyVlanNameChange(vlan_name_change_msgqkey_list, vlan_name_vid);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleProtocolVlan()
{
    if (SYS_CALLBACK_OM_GetAndResetNextChange(
            SYS_CALLBACK_OM_KIND_PROTOCOL_VLAN, &protocol_vlan_lport, NULL) ==
            FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyProtocolVlanChange(protocol_vlan_change_msgqkey_list,
        protocol_vlan_lport);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleVlanMemberTag()
{
    if (SYS_CALLBACK_OM_GetAndResetNextChange(
            SYS_CALLBACK_OM_KIND_VLAN_MEMBER_TAG, vlan_member_tag_ar, NULL) ==
            FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyVlanMemberTagChange(vlan_member_tag_change_msgqkey_list,
        vlan_member_tag_ar[0] + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1,
        vlan_member_tag_ar[1]);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleXstpPortState()
{
    SYS_CALLBACK_OM_Common_T    xstp_port_db;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(
            SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE, xstp_port_state_ar,
            &xstp_port_db) == FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyXstpPortStateChange(xstp_port_state_msgqkey_list,
        XSTP_OM_GetMstidByEntryId(xstp_port_state_ar[0]), xstp_port_state_ar[1],
        xstp_port_db.state, xstp_port_db.merge);
    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleXstpVersion()
{
    UI32_T  mode, status;

    if (SYS_CALLBACK_OM_GetAndResetNextChange(SYS_CALLBACK_OM_KIND_XSTP_VERSION,
            NULL, NULL) == FALSE)
    {
        return FALSE;
    }

    if (XSTP_OM_GetSystemSpanningTreeVersion(&mode) == TRUE)
    {
        if (XSTP_OM_GetSystemSpanningTreeStatus(&status) == TRUE)
        {
            CMGR_NotifyXstpVersionChange(xstp_version_msgqkey_list, mode,
                status);
        }
        else
        {
            printf("%s: failed to get stp status\r\n", __func__);
        }
    }
    else
    {
        printf("%s: failed to get stp mode\r\n", __func__);
    }

    return TRUE;
}

static BOOL_T CMGR_GROUP_HandleXstpPortTopo()
{
    if (SYS_CALLBACK_OM_GetAndResetNextChange(
            SYS_CALLBACK_OM_KIND_XSTP_PORT_TOPO, xstp_port_topo_ar, NULL) ==
            FALSE)
    {
        return FALSE;
    }

    CMGR_NotifyXstpPortTopoChange(xstp_port_topo_msgqkey_list,
        XSTP_OM_GetMstidByEntryId(xstp_port_topo_ar[0]), xstp_port_topo_ar[1]);
    return TRUE;
}

static BOOL_T CMGR_GROUP_ProcessCallback()
{
    BOOL_T  result = FALSE;

    result |= CMGR_GROUP_HandleNormalVlan();
    result |= CMGR_GROUP_HandleGvrpVlan();
    result |= CMGR_GROUP_HandleL3Vlan();
    result |= CMGR_GROUP_HandlePortVlan();
    result |= CMGR_GROUP_HandlePortVlanMode();
    result |= CMGR_GROUP_HandlePvid();
    result |= CMGR_GROUP_HandleVlanName();
    result |= CMGR_GROUP_HandleProtocolVlan();
    result |= CMGR_GROUP_HandleVlanMemberTag();
    result |= CMGR_GROUP_HandleXstpPortState();
    result |= CMGR_GROUP_HandleXstpVersion();
    result |= CMGR_GROUP_HandleXstpPortTopo();

    return result;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
