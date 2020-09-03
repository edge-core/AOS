/* MODULE NAME - DCB_GROUP.C
 * PURPOSE : Provides the definitions for DCB thread group functionalities.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "l2_l4_proc_comm.h"
#include "backdoor_mgr.h"
#include "dcb_group.h"
#if (SYS_CPNT_CN == TRUE)
#include "cn_mgr.h"
#endif
#if (SYS_CPNT_DCBX == TRUE)
#include "dcbx_init.h"
#include "dcbx_mgr.h"
#include "dcbx_type.h"
#endif
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_PFC == TRUE)
#include "pfc_init.h"
#include "pfc_mgr.h"
#include "pfc_type.h"
#endif
#if (SYS_CPNT_ETS == TRUE)
#include "ets_init.h"
#include "ets_mgr.h"
#include "ets_type.h"
#endif

#include "swctrl_pom.h"

#if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))
#include "l4_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of DCB group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(DCB_GROUP_MgrMsg_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* union all data types used for DCB group MGR IPC messages to get the maximum
 * required ipc message buffer
 */

typedef union
{
    SYS_CALLBACK_MGR_TrunkMember_CBData_T   trunk_member_cbdata;
#if (SYS_CPNT_CN == TRUE)
    SYS_CALLBACK_MGR_CnRemoteChange_CBData_T    cn_remote_change_cbdata;
#endif
#if (SYS_CPNT_DCBX == TRUE)
    SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T    dcbx_ets_tlv_rx_cbdata;
    SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T    dcbx_pfc_tlv_rx_cbdata;
#endif
#if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))
    SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T  cos_pcfg_chg_cbdata;
#endif
} DCB_GROUP_SysCallbackData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(DCB_GROUP_SysCallbackData_T))];
} DCB_GROUP_SysCallbackMsg_T;

typedef union DCB_GROUP_MgrMsg_U
{
#if (SYS_CPNT_CN == TRUE)
    CN_MGR_IpcMsg_T                 cn_mgr_ipcmsg;
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_MGR_IpcMsg_T               dcbx_mgr_ipcmsg;
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_IpcMsg_T                ets_mgr_ipcmsg;
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_IpcMsg_T                pfc_mgr_ipcmsg;
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_TYPE_HandleHotSwapArg_T     handle_hot_swap_ipcmsg;
#endif
    DCB_GROUP_SysCallbackMsg_T      sys_callback_ipcmsg;
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
} DCB_GROUP_MgrMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void DCB_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void DCB_GROUP_SetTransitionMode(void);
static void DCB_GROUP_EnterTransitionMode(void);
static void DCB_GROUP_EnterMasterMode(void);
static void DCB_GROUP_EnterSlaveMode(void);
static void DCB_GROUP_ProvisionComplete(void);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void DCB_GROUP_HandleHotInertion(SYSFUN_Msg_T *msgbuf_p);
static void DCB_GROUP_HandleHotRemoval(SYSFUN_Msg_T *msgbuf_p);
#endif
static void DCB_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void DCB_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex);
static void DCB_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex);
static void DCB_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex);
static void DCB_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex);
#if (SYS_CPNT_CN == TRUE)
static void DCB_GROUP_CnRemoteChangeCallbackHandler(UI32_T lport,
    UI32_T neighbor_num, UI8_T cnpv_indicators, UI8_T ready_indicators);
#endif
#if (SYS_CPNT_DCBX == TRUE)
static void DCB_GROUP_EtsReceivedCallbackHandler( UI32_T lport,
                                                  BOOL_T is_delete,
                                                  BOOL_T rem_recommend_rcvd,
                                                  BOOL_T rem_willing,
                                                  BOOL_T rem_cbs,
                                                  UI8_T  rem_max_tc,
                                                  UI8_T  *rem_config_pri_assign_table,
                                                  UI8_T  *rem_config_tc_bandwidth_table,
                                                  UI8_T  *rem_config_tsa_assign_table,
                                                  UI8_T  *rem_recommend_pri_assign_table,
                                                  UI8_T  *rem_recommend_tc_bandwidth_table,
                                                  UI8_T  *rem_recommend_tsa_assign_table);
static void DCB_GROUP_PfcReceivedCallbackHandler(UI32_T lport,
                                                 BOOL_T is_delete,
                                                 UI8_T  *rem_mac,
                                                 BOOL_T rem_willing,
                                                 BOOL_T rem_mbc,
                                                 UI8_T  rem_pfc_cap,
                                                 UI8_T  rem_pfc_enable);
static void DCB_GROUP_EtsCfgChangedCallbackHandler(UI32_T lport);
static void DCB_GROUP_PfcCfgChangedCallbackHandler(UI32_T lport);
#endif

#if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))
static void DCB_GROUP_CosPortConfigChangedCallbackHandler(UI32_T lport, UI32_T priority_of_config);
#endif

/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for stagroup mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - DCB_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resources for CN group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_InitiateProcessResources();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_InitiateSystemResources();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_InitiateSystemResources();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_InitiateSystemResources();
#endif
    return;
} /* End of DCB_GROUP_InitiateProcessResources */

/* FUNCTION NAME - DCB_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relations for CN group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_Create_InterCSC_Relation(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_CreateInterCscRelation();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_Create_InterCSC_Relation();
#endif

    return;
} /* End of DCB_GROUP_Create_InterCSC_Relation */

/* FUNCTION NAME - DCB_GROUP_Create_All_Threads
 * PURPOSE : Spawn all threads in CN group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_Create_All_Threads(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T thread_id;

    /* BODY
     */

    if (SYSFUN_SpawnThread(SYS_BLD_DCB_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_DCB_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_DCB_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           DCB_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\r\n%s: fail to spawn DCB_GROUP_MGR thread.\r\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_DCB_GROUP, thread_id,
        SYS_ADPT_DCB_GROUP_SW_WATCHDOG_TIMER);
#endif

    return;
} /* End of DCB_GROUP_Create_All_Threads */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME - DCB_GROUP_Mgr_Thread_Function_Entry
 * PURPOSE : Provide entry point for DCB_GROUP_MGR thread.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    L_THREADGRP_Handle_T    tg_handle;
    UI32_T                  member_id;
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    UI32_T                  local_events, received_events;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG              |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG         |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* BODY
     */

    /* join the thread group of DCB Group
     */
    tg_handle = L2_L4_PROC_COMM_GetDcbGroupTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_DCB_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
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
    local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    received_events = 0;
    while (1)
    {
        SYSFUN_ReceiveEvent(all_events,
                            SYSFUN_EVENT_WAIT_ANY,
                            (local_events == 0) ? SYSFUN_TIMEOUT_WAIT_FOREVER
                                                : SYSFUN_TIMEOUT_NOWAIT,
                            &received_events);

        local_events |= received_events;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_DCB_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            DCB_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

            /* need not to do IPCFAIL recovery in transition mode */
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
            if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
            {
                /* request thread group execution permission */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                        __FUNCTION__);
                }

                /* handle request message based on cmd */
                switch (msgbuf_p->cmd)
                {
                    /* global cmd
                     */

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        DCB_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        DCB_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer csc groups. In this case, the IPCFAIL
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
                        DCB_GROUP_EnterSlaveMode();

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
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        DCB_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        DCB_GROUP_HandleHotInertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        DCB_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
#endif

                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        /* process system reload message sent by stkctrl task
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    /* module id cmd
                     */

#if (SYS_CPNT_CN == TRUE)
                    case SYS_MODULE_CN:
                        need_resp = CN_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
#if (SYS_CPNT_PFC == TRUE)
                    case SYS_MODULE_PFC:
                        need_resp = PFC_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
#if (SYS_CPNT_ETS == TRUE)
                    case SYS_MODULE_ETS:
                        need_resp = ETS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
#if (SYS_CPNT_DCBX == TRUE)
                    case SYS_MODULE_DCBX:
                        need_resp = DCBX_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        DCB_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
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
                        printf("\r\n%s: Invalid IPC req cmd.\r\n", __FUNCTION__);
                        need_resp = FALSE;
                } /* end of switch (msgbuf_p->cmd) */

                /* release thread group execution permission */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                        __FUNCTION__);
                }

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(
                        ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
                {
                    printf("\r\n%s: SYSFUN_SendResponseMsg fail.\r\n",
                        __FUNCTION__);
                }
            } /* end of if (SYSFUN_ReceiveMsg...) */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end of if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

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
    } /* end of while (1) */
} /* End of DCB_GROUP_Mgr_Thread_Function_Entry */

/* FUNCTION NAME - DCB_GROUP_SetTransitionMode
 * PURPOSE : Invoke all SetTransitionMode functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_SetTransitionMode();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_SetTransitionMode();
#endif
   return;
} /* End of DCB_GROUP_SetTransitionMode */

/* FUNCTION NAME - DCB_GROUP_EnterTransitionMode
 * PURPOSE : Invoke all EnterTransitionMode functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_EnterTransitionMode();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_EnterTransitionMode();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_EnterTransitionMode();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_EnterTransitionMode();
#endif
    return;
} /* End of DCB_GROUP_EnterTransitionMode */

/* FUNCTION NAME - DCB_GROUP_EnterMasterMode
 * PURPOSE : Invoke all EnterMasterMode functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_EnterMasterMode();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_EnterMasterMode();
#endif
    return;
} /* End of DCB_GROUP_EnterMasterMode */

/* FUNCTION NAME - DCB_GROUP_EnterSlaveMode
 * PURPOSE : Invoke all EnterSlaveMode functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_EnterSlaveMode();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_EnterSlaveMode();
#endif
    return;
} /* End of DCB_GROUP_EnterSlaveMode */

/* FUNCTION NAME - DCB_GROUP_ProvisionComplete
 * PURPOSE : Invoke all ProvisionComplete functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_ProvisionComplete();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_ProvisionComplete();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_ProvisionComplete();
#endif

    return;
} /* End of DCB_GROUP_ProvisionComplete */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME - DCB_GROUP_HandleHotInertion
 * PURPOSE : Invoke all HandleHotInertion functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_HandleHotInertion(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

} /* End of DCB_GROUP_HandleHotInertion */

/* FUNCTION NAME - DCB_GROUP_HandleHotRemoval
 * PURPOSE : Invoke all HandleHotRemoval functions in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_HandleHotRemoval(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_PFC == TRUE)
    PFC_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

} /* End of DCB_GROUP_HandleHotRemoval */
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/* FUNCTION NAME - DCB_GROUP_HandleSysCallbackIPCMsg
 * PURPOSE : Handle system callback messages to DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p;

    /* BODY
     */

    sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

#if (SYS_CPNT_CN == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CN_REMOTE_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_CnRemoteChangeCallbackHandler);
            break;
#endif

#if (SYS_CPNT_DCBX == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ETS_CFG_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_EtsCfgChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PFC_CFG_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
               DCB_GROUP_PfcCfgChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ETS_TLV:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_EtsReceivedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PFC_TLV:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_PfcReceivedCallbackHandler);
            break;
#endif /* end of #if (SYS_CPNT_DCBX == TRUE) */

#if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_PORT_CONFIG_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DCB_GROUP_CosPortConfigChangedCallbackHandler);
#endif
        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }

    return;
} /* End of DCB_GROUP_HandleSysCallbackIPCMsg */

/* FUNCTION NAME - DCB_GROUP_TrunkMemberAdd1stCallbackHandler
 * PURPOSE : Handle the callback message for adding the first trunk member.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_TrunkMemberAdd1st_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_AddFstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_AddFstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_MGR_TrunkMemberAdd1st_CallBack(trunk_ifindex, member_ifindex);
#endif

    return;
} /* End of DCB_GROUP_TrunkMemberAdd1stCallbackHandler */

/* FUNCTION NAME - DCB_GROUP_TrunkMemberAddCallbackHandler
 * PURPOSE : Handle the callback message for adding a trunk member except the
 *           first.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_TrunkMemberAdd_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_MGR_TrunkMemberAdd_CallBack(trunk_ifindex, member_ifindex);
#endif

    return;
} /* End of DCB_GROUP_TrunkMemberAddCallbackHandler */

/* FUNCTION NAME - DCB_GROUP_TrunkMemberDeleteCallbackHandler
 * PURPOSE : Handle the callback message for deleting a trunk member except the
 *           last.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_TrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_MGR_TrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);
#endif

    return;
} /* End of DCB_GROUP_TrunkMemberDeleteCallbackHandler */

/* FUNCTION NAME - DCB_GROUP_TrunkMemberDeleteLstCallbackHandler
 * PURPOSE : Handle the callback message for deleting the last trunk member.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
    UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

#if (SYS_CPNT_CN == TRUE)
    CN_MGR_TrunkMemberDeleteLst_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_DelLstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_DelLstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_MGR_TrunkMemberDeleteLst_CallBack(trunk_ifindex, member_ifindex);
#endif

    return;
} /* End of DCB_GROUP_TrunkMemberDeleteLstCallbackHandler */

#if (SYS_CPNT_CN == TRUE)
/* FUNCTION NAME - DCB_GROUP_CnRemoteChangeCallbackHandler
 * PURPOSE : Handle the callback message for remote CN data change.
 * INPUT   : lport            - the logical port which receives the CN TLV
 *           neighbor_num     - the number of neighbors
 *           cnpv_indicators  - the CNPV indicators in the received CN TLV
 *           ready_indicators - the Ready indicators in the received CN TLV
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_CnRemoteChangeCallbackHandler(UI32_T lport,
    UI32_T neighbor_num, UI8_T cnpv_indicators, UI8_T ready_indicators)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_MGR_RemoteChange_CallBack(lport, neighbor_num, cnpv_indicators,
        ready_indicators);

    return;
} /* End of DCB_GROUP_CnRemoteChangeCallbackHandler */
#endif /* #if (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_DCBX == TRUE)
/* FUNCTION NAME - DCBX_GROUP_EtsReceivedCallbackHandler
 * PURPOSE : Handle the callback message when LLDP receive ETS TLV.
 * INPUT   : lport                            - the logical port which receives the ETS TLV
 *           is_delete                        -
 *           rem_recommend_rcvd               -
 *           rem_willing                      -
 *           rem_cbs                          -
 *           rem_max_tc                       -
 *           rem_config_pri_assign_table      -
 *           rem_config_tc_bandwidth_table    -
 *           rem_config_tsa_assign_table      -
 *           rem_recommend_pri_assign_table   -
 *           rem_recommend_tc_bandwidth_table -
 *           rem_recommend_tsa_assign_table   -
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_EtsReceivedCallbackHandler(UI32_T lport,
                                                 BOOL_T is_delete,
                                                 BOOL_T rem_recommend_rcvd,
                                                 BOOL_T rem_willing,
                                                 BOOL_T rem_cbs,
                                                 UI8_T  rem_max_tc,
                                                 UI8_T  *rem_config_pri_assign_table,
                                                 UI8_T  *rem_config_tc_bandwidth_table,
                                                 UI8_T  *rem_config_tsa_assign_table,
                                                 UI8_T  *rem_recommend_pri_assign_table,
                                                 UI8_T  *rem_recommend_tc_bandwidth_table,
                                                 UI8_T  *rem_recommend_tsa_assign_table)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    DCBX_MGR_EtsRcvd_CallBack(lport, is_delete, rem_recommend_rcvd, rem_willing,
        rem_cbs, rem_max_tc, rem_config_pri_assign_table,
        rem_config_tc_bandwidth_table, rem_config_tsa_assign_table,
        rem_recommend_pri_assign_table, rem_recommend_tc_bandwidth_table,
        rem_recommend_tsa_assign_table);

    return;
} /* End of DCB_GROUP_EtsReceivedCallbackHandler */

/* FUNCTION NAME - DCB_GROUP_PfcReceivedCallbackHandler
 * PURPOSE : Handle the callback message when LLDP receive PFC TLV.
 * INPUT   : lport          - the logical port which receives the PFC TLV
 *           is_delete      -
 *           rem_mac        -
 *           rem_willing    -
 *           rem_mbc        -
 *           rem_pfc_cap    -
 *           rem_pfc_enable -
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void DCB_GROUP_PfcReceivedCallbackHandler(UI32_T lport,
                                                 BOOL_T is_delete,
                                                 UI8_T  *rem_mac,
                                                 BOOL_T rem_willing,
                                                 BOOL_T rem_mbc,
                                                 UI8_T  rem_pfc_cap,
                                                 UI8_T  rem_pfc_enable)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    DCBX_MGR_PfcRcvd_CallBack(lport, is_delete, rem_mac, rem_willing, rem_mbc,
        rem_pfc_cap, rem_pfc_enable);

    return;
} /* End of DCB_GROUP_PfcReceivedCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DCB_GROUP_EtsCfgChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port's ets config has
 *           been changed.
 *
 * INPUT   : lport -- specify the lport whose ets config has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void DCB_GROUP_EtsCfgChangedCallbackHandler(UI32_T lport)
{
    DCBX_MGR_ReRunPortStateMachine(lport, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DCB_GROUP_PfcCfgChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port's pfc config has
 *           been changed.
 *
 * INPUT   : lport -- specify the lport whose pfc config has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void DCB_GROUP_PfcCfgChangedCallbackHandler(UI32_T lport)
{
    DCBX_MGR_ReRunPortStateMachine(lport, FALSE);
}
#endif /* #if (SYS_CPNT_DCBX == TRUE) */

#if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_LocalCheckCosConfig
 *-------------------------------------------------------------------------
 * PURPOSE: To check if the CoS config is ok for PFC/ETS operation.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. trust mode, cos 2 phb 2 queue, must be the same for all port
 *          2. trust cos(l2)
 *-------------------------------------------------------------------------
 */
static BOOL_T DCB_GROUP_LocalCheckCosConfig(void)
{
    typedef struct
            {
                UI32_T  phb;
                UI32_T  color;
            } _PFC_MGR_PhbColorRec_T;

    UI32_T                  lport, cos, cfi, old_map_mode, new_map_mode, priority;
    _PFC_MGR_PhbColorRec_T  old_cos2phb[MAX_COS_VAL+1][MAX_CFI_VAL+1];
    _PFC_MGR_PhbColorRec_T  new_cos2phb[MAX_COS_VAL+1][MAX_CFI_VAL+1];
    BOOL_T                  ret = TRUE, is_1st = TRUE;
    UI8_T                   old_phb2q[MAX_PHB_VAL + 1];
    UI8_T                   new_phb2q[MAX_PHB_VAL + 1];

    priority = COS_TYPE_PRIORITY_USER;

    /* trust mode, cos 2 phb 2 queue, must be the same for all port
     */
    for (lport =1; lport <=SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (TRUE == is_1st)
        {
            if (FALSE == L4_MGR_QOS_GetPortPriorityTrustMode(lport, priority, &old_map_mode))
                continue;

            if (old_map_mode != COS_MAPPING_MODE)
            {
                ret = FALSE;
                break;
            }

            for (cos = MIN_COS_VAL; cos <= MAX_COS_VAL; cos++)
                for (cfi = MIN_CFI_VAL; cfi <= MAX_CFI_VAL; cfi++)
                    if (COS_TYPE_E_NONE != L4_MGR_QOS_GetPortCos2InternalDscp(
                                               lport, priority, cos, cfi,
                                               &old_cos2phb[cos][cfi].phb,
                                               &old_cos2phb[cos][cfi].color))
                        continue;

#if (TRUE == SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT)
            if (FALSE == L4_MGR_QOS_GetPortPriorityIngressDscp2Queue(lport, priority, old_phb2q))
#else
            if (FALSE == L4_MGR_QOS_GetPriorityIngressDscp2Queue(priority, old_phb2q))
#endif /* #if (TRUE == SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT) */
            {
                continue;
            }

            is_1st = FALSE;
        }
        else
        {
            if (  (TRUE == L4_MGR_QOS_GetPortPriorityTrustMode(lport, priority, &new_map_mode))
                &&(new_map_mode != old_map_mode)
               )
            {
                ret = FALSE;
                break;
            }

            for (cos = MIN_COS_VAL; cos <= MAX_COS_VAL; cos++)
                for (cfi = MIN_CFI_VAL; cfi <= MAX_CFI_VAL; cfi++)
                    if (COS_TYPE_E_NONE != L4_MGR_QOS_GetPortCos2InternalDscp(
                                               lport, priority, cos, cfi,
                                               &new_cos2phb[cos][cfi].phb,
                                               &new_cos2phb[cos][cfi].color))
                        continue;

            if (0 != memcmp(&old_cos2phb, &new_cos2phb, sizeof(old_cos2phb)))
            {
                ret = FALSE;
                break;
            }

#if (TRUE == SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT)
            if (  (TRUE == L4_MGR_QOS_GetPortPriorityIngressDscp2Queue(lport, priority, new_phb2q))
#else
            if (  (TRUE == L4_MGR_QOS_GetPriorityIngressDscp2Queue(priority, new_phb2q))
#endif /* #if (TRUE == SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT) */
                &&(0 != memcmp(&old_phb2q, &new_phb2q, sizeof(old_phb2q)))
               )
            {
                ret = FALSE;
                break;
            }
        }
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DCB_GROUP_CosPortConfigChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when COS config of
 *            a port changed
 * INPUT   : lport              -- lport
 *           priority_of_config -- configured by which priority
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void DCB_GROUP_CosPortConfigChangedCallbackHandler(UI32_T lport, UI32_T priority_of_config)
{
    BOOL_T  is_cos_ok;

    if (COS_TYPE_PRIORITY_USER != priority_of_config)
    {
        return;
    }

    /* no one care which port is changed, just make lport valid here
     */
    if (lport < 1 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        lport = 0;
        SWCTRL_POM_GetNextLogicalPort(&lport);
    }

    is_cos_ok = DCB_GROUP_LocalCheckCosConfig();

#if (SYS_CPNT_PFC == TRUE)
    PFC_MGR_CosConfigChanged_CallBack(lport, is_cos_ok);
#endif

#if (SYS_CPNT_ETS == TRUE)
    ETS_MGR_CosConfigChanged_CallBack(lport, is_cos_ok);
#endif

}
#endif /* #if ((SYS_CPNT_COS == TRUE) && ((SYS_CPNT_PFC == TRUE) || (SYS_CPNT_ETS == TRUE)))  */


