/*-----------------------------------------------------------------------------
 * FILE NAME: poe_task.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implement IEEE802.3at Layer2 classification state machine
 *
 * NOTES:
 *    None.
 *
 * History:                                                               
 *    12/13/2007 - Daniel Chen, Created
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
 

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "poe_task.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "poe_type.h"
#include "poe_mgr.h"
#include "poe_om.h"
#include "poe_backdoor.h"
#include "sys_callback_mgr.h"
#include "poe_engine.h"
#include "poe_init.h"
#include "lldp_pmgr.h"
#include "backdoor_mgr.h"
#include "poedrv_type.h"
#include "leddrv.h"
#include <string.h>
#include "stktplg_pom.h"
#include "phyaddr_access.h"
#include "led_pmgr.h"
#include "uc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(POE_TASK_MgrMsg_T)

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* union all data type used for EH Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union POE_TASK_MgrMsg_U
{
    /* sys_callback data type for ISC
     */
    /* temporarily place here for the above sys_callback type not written
     */   
    POE_MGR_IpcMsg_T ipcmsg;
} POE_TASK_MgrMsg_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void POE_TASK_Main(void);
static void POE_TASK_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void POE_TASK_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void POE_TASK_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);

/* STATIC VARIABLE DECLARATIONS 
 */
static UI32_T POE_TASK_TaskID = 0;
// Eugene temp, static UI32_T POE_TASK_MsgqId;
static BOOL_T POE_TASK_IsTransitionDone = FALSE;
static BOOL_T POE_TASK_IsProvisionComplete = FALSE;
/* the buffer for retrieving ipc request for CFGDB_GROUP mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];



/* EXPORTED SUBPROGRAM BODIES
 */  


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_InitiateSystemResources
 *-------------------------------------------------------------------------
 * FUNCTION: Init the POE System_Resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_InitiateSystemResources()
{
DBG_PRINT();
POE_INIT_InitiateSystemResources();
    return;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_CreateTasks
 * ------------------------------------------------------------------------
 * FUNCTION : Create POE main task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void POE_TASK_CreateTasks(void)
{
    UI32_T thread_id;
	
DBG_PRINT();
    if ( SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,            /* Need to define in Sys_bld.h */
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,        /* Need to define in Sys_bld.h */
                            SYS_BLD_POE_TASK,                         /* Need to define in Sys_bld.h */
                            SYS_BLD_TASK_COMMON_STACK_SIZE,
                            SYSFUN_TASK_NO_FP,
                            POE_TASK_Main,
                            NULL,
                            &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn POE thread fail.\n", __FUNCTION__);
    }
    else
    {
        POE_TASK_TaskID = thread_id;
// Eugene temp,        POE_MGR_SetPoeMgtId(thread_id, POE_TASK_MsgqId);
    }

    return ;
}/* End of POE_TASK_CreateTasks */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_Main
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize POE function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void POE_TASK_Main(void)
{
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    void*  timer_id;
#endif
//    UI32_T msgq_id;
    UI32_T all_events;
//    UI32_T timeout;
	UI32_T received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_MsgQ_T ipc_msgq_handle;
    BOOL_T need_resp;
//    SYSFUN_Msg_T         msg;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)mgrtd_ipc_buf;
//    UI8_T pmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
DBG_PRINT();

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    /* start the timer, trigger every second */
    timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_TICKS_PER_SECOND, POE_TYPE_EVENT_TIMER);
#endif

    all_events = POE_TYPE_EVENT_NONE;

    /* Create POE message queue
     * POE MsgQ: 48 messages of 16 bytes which contain the pointer to the packet buffer
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_POE_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {

        SYSFUN_ReceiveEvent (POE_TYPE_EVENT_ALL |
                             SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;
        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            POE_TASK_SetTransitionMode();
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

            /* wait for the request ipc message
             */
            if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT, PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* global cmd
                     */

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
DBG_PRINT("SYS_MODULE_POE");
#if 0 /* Eugene temp */
                        POE_TASK_SetTransitionMode();
                        POE_TASK_IsProvisionComplete = FALSE;

                        /* When in transition mode, the packet received by POE engine should be invalid.
                         * So we must clear the message queue.
                         */

                        if (POE_TASK_MsgqId != 0)
                        {
                            while(SYSFUN_ReceiveMsg(POE_TASK_MsgqId, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msg), &msg) == SYSFUN_OK)
                            {
                                continue;
                            }
                        }

                        /* receive a event to make sure the task is alive */
                        if (local_events & POE_TYPE_EVENT_ENTER_TRANSITION)
                        {
                            POE_TASK_IsTransitionDone = TRUE;
                        }
#endif
//                        local_events &= ~POE_TYPE_EVENT_ALL;
						
                        POE_TASK_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
DBG_PRINT("SYS_MODULE_POE");
                        POE_TASK_EnterMasterMode();

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
DBG_PRINT("SYS_MODULE_POE");
                        POE_TASK_EnterSlaveMode();

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
// Eugene mark,                        SYSFUN_Sleep(200);
                        break;

                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        POE_TASK_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_MODULE_POE:
                        DBG_PRINT("SYS_MODULE_POE");
                        need_resp = POE_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        DBG_PRINT("SYS_MODULE_BACKDOOR");
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        DBG_PRINT("SYS_MODULE_SYS_CALLBACK");
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        POE_TASK_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        POE_TASK_HandleHotInertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        POE_TASK_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif
                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
						msgbuf_p->msg_size=0;
						need_resp=TRUE;
						break;
						
                    default:
//                        printf("%s: Invalid IPC req cmd %d.\n", __FUNCTION__, msgbuf_p->cmd);
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
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
                SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
            }
       	}

        if (local_events & POE_TYPE_EVENT_ALL)
        {
            if ((POE_MGR_GetOperationMode() == SYS_TYPE_STACKING_MASTER_MODE) &&
				POE_TASK_IsProvisionComplete)
            {
#if 0 /* Eugene mark, POE_MGR do it without passing through POE_TASK */
                msgq_id = POE_TASK_MsgqId;

                /* 2008/4/30, for force high power */
                if ((local_events & POE_TYPE_EVENT_FORCE_HIGH_POWER) ||
					(local_events & POE_TYPE_EVENT_LLDP_RECVD))
                {
//                    if((SYSFUN_ReceiveMsgQ((UI32_T)msgq_id, (UI32_T*)&msg, SYSFUN_TIMEOUT_NOWAIT) != SYSFUN_RESULT_NO_MESSAGE) ||
//						(msg.type == POE_TASK_MSG_TYPE_HIGHPOWER))
                    if((SYSFUN_ReceiveMsg((UI32_T)msgq_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msg), &msg) != SYSFUN_RESULT_NO_MESSAGE) &&
#if 0 /* Eugene marked */
                    {
                        if (POE_BACKDOOR_IsDebugMsgOn()==TRUE)
                        {
                            printf("(port %d) Force high-power (state %d)\n", msg.lport, msg.force_high_power_state); 
                        }

                        if (msg.force_high_power_state == 1)
                        {

                            /* Diable LLDP tx/rx per port */
                            LLDP_PMGR_SetPortConfigAdminStatus(msg.lport, LLDP_TYPE_ADMIN_STATUS_DISABLE);

                            /* Disabe state machine */
                            POE_ENGINE_DISABLE_PORT_DOT3AT(msg.lport);

                            /* Enable Force mode */
                            POE_MGR_SetPortForceHighPowerMode(msg.lport, 1);

                            /* Reset this port */
                            POE_MGR_ResetPort(msg.lport);
                        }
                        else
                        {
                            /* Disable Force mode */
                            POE_MGR_SetPortForceHighPowerMode(msg.lport, 0);

                            /* Reset this port */
                            POE_MGR_ResetPort(msg.lport);

                            /* Enable LLDP tx/rx per port */
                            LLDP_PMGR_SetPortConfigAdminStatus(msg.lport, LLDP_TYPE_ADMIN_STATUS_TX_RX);

                            /* Enable state machine */
                            POE_ENGINE_ENABLE_PORT_DOT3AT(msg.lport);
                        }
                    }
					else if (msg.type == POE_TASK_MSG_TYPE_RECEIVELLDP)
#else
                    (msg.type == POE_TASK_MSG_TYPE_RECEIVELLDP))
#endif
					{
                        POE_MGR_DOT3at_PowerMode_T info;

                        if (POE_BACKDOOR_IsDebugMsgOn()==TRUE)
                        {
                            printf("\nReceive 3at frame: port : %d , ttl: %d\n", msg.lport, msg.ttl);
                            printf("status type :%d, source: %d, priority: %d, value: %d\n",
                                    msg.status_powerType, msg.status_powerSource, msg.status_powerPriority, msg.status_powerValue);
                            printf("state control type :%d, source: %d, priority: %d, value: %d, ack: %d\n",
                                    msg.state_control_powerType, msg.state_control_powerSource, msg.state_control_powerPriority,
                                    msg.state_control_portValue, msg.state_control_acknowledge);
                        }

                        /* refresh the state machine timer */
                        POE_ENGINE_RefreshTimer(msg.lport, msg.ttl);

                        memset(&info, 0, sizeof(POE_MGR_DOT3at_PowerMode_T));
                        info.state_control_acknowledge = msg.state_control_acknowledge;
                        info.state_control_portValue= msg.state_control_portValue;
                        info.status_powerValue = msg.status_powerValue;

                        /* handled state control TLV in state machine */
                        POE_ENGINE_ProcessLLDPInfo(msg.lport, &info);
					}
                    else /* no more message */
                    {
                        local_events &= ~POE_TYPE_EVENT_FORCE_HIGH_POWER;
                        local_events &= ~POE_TYPE_EVENT_LLDP_RECVD;
                    }
                }
                if (local_events & POE_TYPE_EVENT_PORT_POE_STATE_CHANGE)
                {
                    I32_T i;

                    POE_MGR_GetPortDetectionStatusPortMask(pmask);
                    for (i=0;i<SYS_ADPT_TOTAL_NBR_OF_LPORT;i++)
                    {
                        if (pmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1-(i/8)] & (0x1<<(7-(i%8))))
                        {
                            POE_ENGINE_ProcessPortStateChange(i+1);
                        }
                    }

                    local_events &= ~POE_TYPE_EVENT_PORT_POE_STATE_CHANGE;
                }
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                if (local_events & POE_TYPE_EVENT_TIMER)
                {
                    POE_MGR_ProcessTimerEvent();
                    local_events &= ~ POE_TYPE_EVENT_TIMER;
                }
#endif
            }
            else /* if (!POE_TASK_IsProvisionComplete) */
            {
                local_events &= ~POE_TYPE_EVENT_ALL;
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterMasterMode(void)
{
DBG_PRINT();
    POE_INIT_EnterMasterMode();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterSlaveMode(void) 
{
DBG_PRINT();
    POE_INIT_EnterSlaveMode();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_SetTransitionMode()
{
DBG_PRINT();
    POE_INIT_SetTransitionMode();
    POE_TASK_IsTransitionDone = TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterTransitionMode()
{
DBG_PRINT();
    POE_INIT_EnterTransitionMode();
    SYSFUN_TASK_ENTER_TRANSITION_MODE(POE_TASK_IsTransitionDone);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell POE that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_ProvisionComplete()
{
DBG_PRINT();
    POE_INIT_ProvisionComplete();
    POE_TASK_IsProvisionComplete = TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortDetectionStatusChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortDetectionStatusChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T status)
{
DBG_PRINT();
    POE_MGR_SetPsePortDetectionStatus_callback(unit,port,status);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortStatusChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortStatusChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T status)
{
DBG_PRINT("LED:(%lu, %lu, %lu)",unit,port,status);
//    LED_MGR_SetPOELed(unit,port,status);
    LED_PMGR_SetPOELed_callback(unit,port,status);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvIsMainPowerReachMaximumCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvIsMainPowerReachMaximumCallbackHandler(UI32_T unit, UI32_T status)
{
DBG_PRINT();
        LED_PMGR_SetPoeLED(unit, status);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortOverloadStatusChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortOverloadStatusChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T status)
{
DBG_PRINT();
    POE_MGR_Notify_PortOverloadStatusChange(unit,port,status);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortPowerConsumptionChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           value -- which value
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortPowerConsumptionChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T value)
{
DBG_PRINT();
    POE_MGR_SetPsePortPoweConsumption_callback(unit,port,value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortPowerClassificationChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           value -- which value
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortPowerClassificationChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T value)
{
DBG_PRINT();
    POE_MGR_SetPsePortPowerClassification_callback(unit,port,value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvMainPseConsumptionChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           value -- which value
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvMainPseConsumptionChangeCallbackHandler(UI32_T unit, UI32_T value)
{
DBG_PRINT();
    POE_MGR_SetMainPseConsumptionPower_callback(unit,value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPseOperStatusChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPseOperStatusChangeCallbackHandler(UI32_T unit, UI32_T status)
{
DBG_PRINT();
    POE_MGR_SetMainPseOperStatus_callback(unit,status);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPowerDeniedOccurFrequentlyCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPowerDeniedOccurFrequentlyCallbackHandler(UI32_T unit, UI32_T port)
{
DBG_PRINT();
    POE_MGR_Notify_PowerDeniedOccurFrequently(unit,port);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPortStatusChangeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 *           status -- which status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_PoedrvPortFailureStatusChangeCallbackHandler(UI32_T unit, UI32_T port, UI32_T status)
{
DBG_PRINT();

}

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_LldpDot3atInfoCallbackHandlerReceived
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_LldpDot3atInfoCallbackHandlerReceived(UI32_T unit,
                                                        UI32_T port,
                                                        UI8_T  power_type,
                                                        UI8_T  power_source,
                                                        UI8_T  power_priority,
                                                        UI16_T power_value,
                                                        UI8_T  requested_power_type,
                                                        UI8_T  requested_power_source,
                                                        UI8_T  requested_power_priority,
                                                        UI16_T requested_power_value,
                                                        UI8_T  acknowledge)
{
DBG_PRINT();
    POE_TYPE_Dot3atPowerInfo_T info;

	memset(&info, 0, sizeof(POE_TYPE_Dot3atPowerInfo_T));
	
	info.power_type = power_type;
	info.power_source = power_source;
	info.power_priority = power_priority;
	info.power_value = power_value;
	info.requested_power_type = requested_power_type;
	info.requested_power_source = requested_power_source;
	info.requested_power_priority = requested_power_priority;
	info.requested_power_value = requested_power_value;
	info.acknowledge = acknowledge;

    POE_MGR_NotifyLldpFameReceived_Callback(unit,port,&info);
}
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_LldpDot3atInfoCallbackHandlerReceived
 * -------------------------------------------------------------------------
 * FUNCTION: Port detection status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           port   -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_LldpDot3atInfoCallbackHandlerReceived(UI32_T unit,
                                                        UI32_T port,
                                                        UI8_T  power_type,
                                                        UI8_T  power_source,
                                                        UI8_T  power_priority,
                                                        UI16_T pd_requested_power,
                                                        UI16_T pse_allocated_power)
{
DBG_PRINT();
    POE_TYPE_Dot3atPowerInfo_T info;

	memset(&info, 0, sizeof(POE_TYPE_Dot3atPowerInfo_T));
	
	info.power_type = power_type;
	info.power_source = power_source;
	info.power_priority = power_priority;
	info.pd_requested_power = pd_requested_power;
	info.pse_allocated_power = pse_allocated_power;

    POE_MGR_NotifyLldpFameReceived_Callback(unit,port,&info);
}
#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_SysdrvPowerStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Power status change callback function, register to poedrv
 * INPUT   : unit   -- which unit
 *           power  -- VAL_swIndivPowerIndex_externalPower
 *                     VAL_swIndivPowerIndex_internalPower
 *           status -- VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 *                     VAL_swIndivPowerStatus_notPresent
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_SysdrvPowerStatusChangedCallbackHandler(UI32_T unit, UI32_T power, UI32_T status)
{
DBG_PRINT();
    POE_MGR_PowerStatusChanged_CallBack(unit, power, status);
}
#endif

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_TASK_PoedrvPowerDeniedOccurFrequentlyCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: time range status change callback function, register to time_range
 * INPUT   : isChanged_list -- if status changed by time range index list
 *           status_list    -- status by time range index list
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_TASK_TimeRangeStatusChangeCallbackHandler(UI8_T *isChanged_list, UI8_T *status_list)
{
DBG_PRINT();
    POE_MGR_TimeRangeStatusChange_callback(isChanged_list, status_list);
}
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_TASK_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE: This function will handle all callbacks from IPC messages in CSCGroup1.
 * INPUT:   msgbuf_p  --  SYS_CALLBACK IPC message
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 *------------------------------------------------------------------------------
 */
static void POE_TASK_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;


    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_DETECTION_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortDetectionStatusChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortStatusChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_IS_MAIN_POWER_REACH_MAXIMUM:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvIsMainPowerReachMaximumCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_OVERLOAD_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortOverloadStatusChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CONSUMPTION_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortPowerConsumptionChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CLASSIFICATION_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortPowerClassificationChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_MAIN_PSE_CONSUMPTION_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvMainPseConsumptionChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PSE_OPER_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPseOperStatusChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_POWER_DENIED_OCCUR_FRENQUENTLY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPowerDeniedOccurFrequentlyCallbackHandler);
            break;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_LldpDot3atInfoCallbackHandlerReceived);
            break;
#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_SysdrvPowerStatusChangedCallbackHandler);
            break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_FAILURE_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_PoedrvPortFailureStatusChangeCallbackHandler);
            break;

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TIME_RANGE_STATUS_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &POE_TASK_TimeRangeStatusChangeCallbackHandler);
            break;
#endif

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_TASK_HandleHotInertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in CSCGroup1.
 *
 * INPUT:
 *    starting_port_ifindex.
      number_of_port
      use_default
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
static void POE_TASK_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    POE_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);      
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : POE_TASK_HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut removal in CSCGroup1.
 *
 * INPUT:
 *    starting_port_ifindex.
      number_of_port
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
static void POE_TASK_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    POE_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);      
}
#endif

