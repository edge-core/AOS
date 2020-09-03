/* MODULE NAME:  driver_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mux group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/22/2007 - KH shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "driver_group.h"
#include "driver_proc_comm.h"
#include "dev_amtrdrv.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_swdrv.h"
#include "dev_swdrv_pmgr.h"
#include "dev_swdrvl4.h"
#include "dev_swdrvl4_pmgr.h"
#include "dev_nmtrdrv.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_swdrvl3.h"
#include "dev_swdrvl3_pmgr.h"
#include "sys_time_init.h"

#include "dev_rm_pmgr.h"

#if (SYS_CPNT_NICDRV == TRUE)
#include "nicdrv_init.h"
#include "dev_nicdrv.h"
#endif

#if (SYS_CPNT_ISCDRV == TRUE)
#include "isc_init.h"
#endif

#if (SYS_CPNT_AMTRDRV == TRUE)
#include "amtrdrv_mgr.h"
#endif

#if (SYS_CPNT_SYSDRV == TRUE)
#include "sysdrv_task.h"
#include "sysdrv_init.h" /* anzhen.zheng, 2/2/2007 */
#include "sysdrv.h"
#endif

#if (SYS_CPNT_NMTRDRV == TRUE)
#include "nmtrdrv.h"
#endif

#if (SYS_CPNT_FLASHDRV == TRUE)
#include "fs_init.h"
#endif

#if (SYS_CPNT_SWDRV == TRUE)
#include "swdrv_init.h"
#include "isc.h"
#endif
#if (SYS_CPNT_SWDRVL3 == TRUE)
#include "swdrvl3_init.h"
#include "swdrvl3.h"
#endif
#if (SYS_CPNT_SWDRVL4 == TRUE)
#include "swdrvl4.h"
#endif
#include "isc_agent_init.h"
#include "lan.h"
#include "leddrv.h"
#if (SYS_CPNT_POE == TRUE)
#include "poedrv_init.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYNCE==TRUE)
#include "syncedrv.h"
#endif


#if(SYS_CPNT_PTP_ISR == TRUE)
#include "ptp_isr.h"
#endif

#if (SYS_CPNT_SYS_TIME == TRUE)
#include "sys_time.h"
#endif

#include "stktplg_board.h"

#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)
#include "onlpdrv_sfp.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define DBG_PRINT(...) printf(__VA_ARGS__);
#else
#define DBG_PRINT(...)
#endif

#define DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(DRIVER_GROUP_MGR_MSG_T)

typedef union {
    /* need add sys_callback msg */
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    sys_callback_mgr_authpkt_cbdata;
} DRIVER_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(DRIVER_GROUP_Syscallback_CBData_T))];
} DRIVER_GROUP_Syscallback_Msg_T;

/* union all data type used for CSCGroup1 MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union DRIVER_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_IPCMsg_T       dev_nicdrv_mgr_ipcmsg;
#endif
    DEV_AMTRDRV_PMGR_IPCMSG_T dev_amtrdrv_ipcmsg;
    DEV_SWDRV_PMGR_IPCMSG_T   dev_swdrv_ipcmsg;
    DEV_SWDRVL3_PMGR_IPCMSG_T dev_swdrvl3_ipcmsg;
    DEV_SWDRVL4_PMGR_IPCMSG_T dev_swdrvl4_ipcmsg;
    DEV_NMTRDRV_PMGR_IPCMSG_T dev_nmtrdrv_ipcmsg;
    DEVRM_PMGR_IPCMSG_T       dev_rm_ipcmsg;
    BACKDOOR_MGR_Msg_T        backdoor_mgr_ipcmsg;
    DRIVER_GROUP_Syscallback_Msg_T sys_callback_ipcmsg;
} DRIVER_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T  thread_priority;
    UI32_T  sched_policy;
    char    *thread_name;
    UI32_T  stack_size;
    UI32_T  task_option;
    UI32_T  msgq_key;
    UI8_T   *ipc_buf;
} DRIVER_GROUP_ThreadArg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void DRIVER_GROUP_Mgr_Thread_Function_Entry(void* arg);

/*static void DRIVER_GROUP_SetTransitionMode(void);
static void DRIVER_GROUP_EnterTransitionMode(void);
static void DRIVER_GROUP_EnterMasterMode(void);
static void DRIVER_GROUP_EnterSlaveMode(void);*/
static void DRIVER_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for driver_group mgr thread
 */
static UI8_T driver_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static UI8_T driver_group_mgrtd_ipc_sendpacket_buf[SYSFUN_SIZE_OF_MSG(DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static UI8_T driver_group_mgrtd_ipc_freepacket_buf[SYSFUN_SIZE_OF_MSG(DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static UI8_T driver_group_mgrtd_ipc_dispatchpkt_buf[SYSFUN_SIZE_OF_MSG(DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

DRIVER_GROUP_ThreadArg_T driver_group_thread_arg[] =
{
    { /* MGR */
        /* thread_priority  */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_PRIORITY,
        /* sched_policy     */ SYS_BLD_DRIVER_GROUP_MGR_SCHED_POLICY,
        /* thread_name      */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_NAME,
        /* stack_size       */ 24 * SYS_TYPE_1K_BYTES,
        /* task_option      */ SYSFUN_TASK_FP,
        /* msgq_key         */ SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
        /* ipc_buf          */ driver_group_mgrtd_ipc_buf,
    },
    { /* NICDRV send packet */
        /* thread_priority  */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_PRIORITY,
        /* sched_policy     */ SYS_BLD_DRIVER_GROUP_MGR_SCHED_POLICY,
        /* thread_name      */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_NAME "_TX",
        /* stack_size       */ 32 * SYS_TYPE_1K_BYTES,
        /* task_option      */ SYSFUN_TASK_FP,
        /* msgq_key         */ SYS_BLD_DRIVER_GROUP_SENDPACKET_IPCMSGQ_KEY,
        /* ipc_buf          */ driver_group_mgrtd_ipc_sendpacket_buf,
    },
    { /* NICDRV free packet */
        /* thread_priority  */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_PRIORITY,
        /* sched_policy     */ SYS_BLD_DRIVER_GROUP_MGR_SCHED_POLICY,
        /* thread_name      */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_NAME "_FREE",
        /* stack_size       */ SYS_BLD_TASK_COMMON_STACK_SIZE,
        /* task_option      */ SYSFUN_TASK_FP,
        /* msgq_key         */ SYS_BLD_DRIVER_GROUP_FREEPACKET_IPCMSGQ_KEY,
        /* ipc_buf          */ driver_group_mgrtd_ipc_freepacket_buf,
    },
    { /* LAN dispatch packet */
        /* thread_priority  */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_PRIORITY,
        /* sched_policy     */ SYS_BLD_DRIVER_GROUP_MGR_SCHED_POLICY,
        /* thread_name      */ SYS_BLD_DRIVER_GROUP_MGR_THREAD_NAME "_DISPATCH",
        /* stack_size       */ SYS_BLD_TASK_COMMON_STACK_SIZE,
        /* task_option      */ SYSFUN_TASK_FP,
        /* msgq_key         */ SYS_BLD_DRIVER_GROUP_DISPATCHPKT_IPCMSGQ_KEY,
        /* ipc_buf          */ driver_group_mgrtd_ipc_dispatchpkt_buf,
    },
};

BOOL_T DEV_SWDRV_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init CSC Group.
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
void DRIVER_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)
    {
        I16_T aos_uport_to_onlp_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];

        STKTPLG_BOARD_GetONLPSFPPortMapping(aos_uport_to_onlp_port);
        if (ONLPDRV_SFP_Init(aos_uport_to_onlp_port)==FALSE)
        {
            printf("%s(%d)ONLPDRV_SFP_Init error.\r\n", __FUNCTION__, __LINE__);
        }
    }
#endif

    DEV_SWDRV_Init();
#if (SYS_CPNT_NICDRV == TRUE)
    //NICDRV_INIT_InitiateProcessResources();
#endif
    DEV_NICDRV_Init();
#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_Initiate_System_Resources();
    ISC_Init();
#endif
    DEV_SWDRVL4_Init();
    DEVRM_Initial();
    DEV_AMTRDRV_Init();

#if (SYS_CPNT_I2CDRV_BUS0_RUN_IN_DRIVER_PROC_CONTEXT==TRUE)
    SYS_TIME_Init_InitiateProcessResources(DEV_SWDRV_TwsiDataRead, DEV_SWDRV_TwsiDataWrite);
#else
    SYS_TIME_Init_InitiateProcessResources(NULL, NULL);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DRIVER_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for L2MUX Group.
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
void DRIVER_GROUP_Create_InterCSC_Relation(void)
{
    DEV_SWDRV_Create_InterCSC_Relation();
    /* register dev_swdrvl3 backdoor */
    DEV_SWDRVL3_Create_InterCSC_Relation();
#if (SYS_CPNT_NICDRV == TRUE)
    LAN_Create_InterCSC_Relation();
    DEV_NICDRV_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_Create_InterCSC_Relation();
    ISC_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_AMTRDRV == TRUE)
    DEV_AMTRDRV_Create_InterCSC_Relation();
    AMTRDRV_MGR_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_INIT_Create_InterCSC_Relation();
#endif

/* anzhen.zheng, 2/2/2007 */
#if (SYS_CPNT_SYSDRV == TRUE)
    /* SYSDRV_INIT_Create_InterCSC_Relation is defined in the module that will
     * be put in a shared library "libsysdrv.so", and SYSDRV_Create_InterCSC_Relation
     * is defined in a static lib(i.e. libsysdrv_private.a). To avoid making
     * dependency between libsysdrv.so and libsysdrv_private.a,
     * SYSDRV_Create_InterI2C_Relation should be called directly here.
   SYSDRV_INIT_Create_InterCSC_Relation();
     */
   SYSDRV_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SWDRVL4 == TRUE)
    DEV_SWDRVL4_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_NMTRDRV == TRUE)
    DEV_NMTRDRV_Create_InterCSC_Relation();
    NMTRDRV_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_LEDDRV == TRUE)
    LEDDRV_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_FLASHDRV==TRUE)
    FLASHDRV_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_SYNCE==TRUE)
    SYNCEDRV_Create_InterCSC_Relation();
#endif

    SYS_TIME_INIT_Create_InterCSC_Relation();
}

/* This is a temporary code for ease of display status change of
 * SyncE Chip when debug message level is changed to a value
 * larger than SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG
 */
#if (SYS_CPNT_SYNCE==TRUE) && (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_IDT82V3399)
extern void SYNCEDRV_CreateTask(void);
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_Create_All_Threads
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
void DRIVER_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_NICDRV == TRUE)
    LAN_CreateTask();
    DEV_NICDRV_CreateTask();
#endif

#if (SYS_CPNT_ISCDRV == TRUE)
    ISC_AGENT_INIT_CreateTasks();
#endif

#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    extern void SWDRV_MONITOR_CreateTask(void);
    SWDRV_MONITOR_CreateTask();
#endif

#if (SYS_CPNT_SWDRVL3 == TRUE)
    SWDRVL3_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_AMTRDRV == TRUE)
    AMTRDRV_TASK_CreateTask();
#endif

#if (SYS_CPNT_SYSDRV == TRUE)
    SYSDRV_TASK_CreateTask();
#endif

#if ((SYS_CPNT_SYS_TIME == TRUE) && (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE))
    SYS_TIME_TASK_CreateTask();
#endif

#if (SYS_CPNT_NMTRDRV == TRUE)
    NMTRDRV_TASK_CreateTask();
#endif

#if (SYS_CPNT_FLASHDRV == TRUE)
    FLASHDRV_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_POE == TRUE)
    POEDRV_INIT_CreateTasks();
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE ) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
    VLAN_NET_CreateTask();
#endif

    {
        int i;

        for (i = 0; i < sizeof(driver_group_thread_arg)/sizeof(*driver_group_thread_arg); i++)
        {
            UI32_T ret;

            if ((ret = SYSFUN_SpawnThread(
                    driver_group_thread_arg[i].thread_priority,
                    driver_group_thread_arg[i].sched_policy,
                    driver_group_thread_arg[i].thread_name,
                    driver_group_thread_arg[i].stack_size,
                    driver_group_thread_arg[i].task_option,
                    DRIVER_GROUP_Mgr_Thread_Function_Entry,
                    &driver_group_thread_arg[i],
                    &thread_id)) != SYSFUN_OK)
            {
                printf("%s:Spawn thread %s fail.\r\n", __FUNCTION__, driver_group_thread_arg[i].thread_name);
            }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            if (ret == SYSFUN_OK)
            {
                if (driver_group_thread_arg[i].msgq_key == SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY)
                {
                    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_DRIVER_GROUP, thread_id, SYS_ADPT_DRIVER_GROUP_SW_WATCHDOG_TIMER);
                }
            }
#endif
        }
    }

#if(SYS_CPNT_PTP_ISR == TRUE)
    PTP_ISR_CreateTask();
#endif

/* This is a temporary code for ease of display status change of
 * SyncE Chip when debug message level is changed to a value
 * larger than SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG
 */
#if (SYS_CPNT_SYNCE==TRUE) && (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_IDT82V3399)
    SYNCEDRV_CreateTask();
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_Mgr_Thread_Function_Entry
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
static void DRIVER_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    DRIVER_GROUP_ThreadArg_T *thread_arg_p = arg;

    SYSFUN_MsgQ_T   ipc_msgq_handle;
    UI32_T          received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T    *msgbuf_p = (SYSFUN_Msg_T *)thread_arg_p->ipc_buf;
    BOOL_T          need_resp;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
#endif


    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(thread_arg_p->msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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
        DBG_PRINT("%s: receive_events=%lu\n",__FUNCTION__,received_events);

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            printf("%s: receive set transition mode event\n",__FUNCTION__);
            //DRIVER_GROUP_SetTransitionMode();
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
                DRIVER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                /*kh_shi if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
                 */

                DBG_PRINT("%s: msgbuf_p->cmd=%lu\n",__FUNCTION__,msgbuf_p->cmd);

                /* handle request message based on cmd
                 */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                start_time = SYSFUN_GetSysTick();
#endif
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
#if (SYS_CPNT_NICDRV == TRUE)
                    case SYS_MODULE_NIC:
                        need_resp=DEV_NICDRV_HandleIPCReqMsg(msgbuf_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_NIC,end_time);
#endif
                        break;
#endif
                    case SYS_MODULE_DEV_NMTRDRV:
                        need_resp = DEV_NMTRDRV_HandleIPCReqMsg(msgbuf_p);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_NMTRDRV,end_time);
#endif
                        break;
                    case SYS_MODULE_DEV_AMTRDRV:
                        need_resp = DEV_AMTRDRV_HandleIPCReqMsg(msgbuf_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_AMTRDRV,end_time);
#endif
                       break;
                    case SYS_MODULE_DEV_SWDRV:
                        need_resp = DEV_SWDRV_HandleIPCReqMsg(msgbuf_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_SWDRV,end_time);
#endif
                        break;
                    case SYS_MODULE_DEV_SWDRVL3:
                        need_resp = DEV_SWDRVL3_HandleIPCReqMsg(msgbuf_p);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_SWDRVL3,end_time);
#endif
                        break;
                    case SYS_MODULE_DEV_SWDRVL4:
                        need_resp = DEV_SWDRVL4_HandleIPCReqMsg(msgbuf_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_SWDRVL4,end_time);
#endif
                        break;
                    case SYS_MODULE_DEV_RM:
                        need_resp = DEVRM_HandleIPCReqMsg(msgbuf_p);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_DEV_RM,end_time);
#endif
                        break;
                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        DRIVER_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_SYS_CALLBACK,end_time);
#endif
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time =SYSFUN_GetSysTick()-start_time;
                        DEV_NICDRV_SetDriverGroupModuleProcessTime(SYS_MODULE_BACKDOOR,end_time);
#endif
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        //DRIVER_GROUP_EnterMasterMode();

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
                        //DRIVER_GROUP_EnterSlaveMode();

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
                        //DRIVER_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if 0 /* hot insertion/removal of driver group called directly by stkctrl_task */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        DRIVER_GROUP_HandleHotRemoval();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
 #endif
 #endif
 /*add by fen.wang ,to process system reload msg*/
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
                        printf("%s: Invalid IPC req cmd. msgbuf_p->cmd=%d\n", __FUNCTION__,msgbuf_p->cmd);
                        need_resp=FALSE;
                }

                /* release thread group execution permission
                 */
                /*kh_shi if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                 */
                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
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
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_DRIVER_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}


/* LOCAL SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DRIVER_GROUP_DispatchPacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, to dispatch a packet.
 * INPUT   : ...
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void DRIVER_GROUP_DispatchPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T    tag_info,
    UI16_T    type,
    UI32_T    pkt_length,
    UI32_T    unit_no,
    UI32_T    port_no,
    UI32_T    packet_class)
{
    LAN_DispatchPacket(
        mref_handle_p,
        dst_mac,
        src_mac,
        tag_info,
        type,
        pkt_length,
        unit_no,
        port_no,
        packet_class);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_HandleSysCallbackIPCMsg
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
static void DRIVER_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        /*kh_shi case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_STA_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &DRIVER_GROUP_L2muxReceiveSTAPacketCallbackHandler);
            break;
         */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &DRIVER_GROUP_DispatchPacketCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}


