/* MODULE NAME:  vrrp_group.c
 * PURPOSE:
 *     This file handles the related work for VRRP csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "l_threadgrp.h"

#include "vrrp_group.h"
#include "vrrp_proc_comm.h"

#include "vrrp_init.h"
#include "vrrp_mgr.h"
#include "vrrp_pmgr.h"
#include "syslog_om.h"
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "vrrp_task.h"
#include "vrrp_vm.h"
#include "vrrp_mgr.h"
#include "swctrl.h"
#include "l_mm_type.h"
#include "backdoor_mgr.h"
#include "vlan_pom.h"
#include "ip_lib.h"
#include "netcfg_pom_ip.h"
#include "iml_pmgr.h"
#include "amtrl3_pmgr.h"
#include "netcfg_pmgr_nd.h"
#include "vlan_pmgr.h"
#include "sys_cpnt.h"
#include "sys_callback_mgr.h"
#include "ipal_kernel.h"
#include "swctrl_pmgr.h"


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void   VRRP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void   VRRP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define VRRP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(VRRP_GROUP_MgrMsg_T)
static void VRRP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);


/* MACRO FUNCTION DECLARATIONS
 */

static BOOL_T  vrrpTaskErrorLogFlag[8];


/* DATA TYPE DECLARATIONS
 */
/* union all data type used for MGR IPC message to get the maximum required
 * ipc message buffer
 */

typedef union VRRP_GROUP_MgrMsg_U
{
    
    BACKDOOR_MGR_Msg_T                  backdoor_mgr_ipcmsg;
    VRRP_MGR_IPCMsg_T vrrp_mgr_ipcmsg;
    
} VRRP_GROUP_MgrMsg_T;


#define VRRP_GROUP_TIMER_EVENT              0x0001L
#define VRRP_GROUP_PROVISION_COMPLETE_EVENT 0x0008L

/* STATIC VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VRRP_GROUP_EnterTransitionMode(void);
static void VRRP_GROUP_SetTransitionMode(void);
static void VRRP_GROUP_EnterMasterMode(void);
static void VRRP_GROUP_EnterSlaveMode(void);
static void VRRP_GROUP_BackdoorCallback(void);
static void VRRP_GROUP_ProvisionComplete(void);
static void VRRP_GROUP_RifActive_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VRRP_GROUP_RifDown_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VRRP_GROUP_L3IfDestroy_CallBack(UI32_T ifindex);
static void VRRP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for VRRP group.
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
void VRRP_GROUP_InitiateProcessResources(void)
{
    /* Initial PMGR used in VRRP Group */
   VRRP_PMGR_InitiateProcessResource();

   VLAN_POM_InitiateProcessResource();
   NETCFG_POM_IP_InitiateProcessResource();
   IML_PMGR_InitiateProcessResource();
   AMTRL3_PMGR_InitiateProcessResource();
   NETCFG_PMGR_ND_InitiateProcessResource();
   VLAN_PMGR_InitiateProcessResource();
   IPAL_Kernel_Init();
   SWCTRL_PMGR_Init();
    /* Initial VRRP Group */
   VRRP_MGR_Initiate_System_Resources();
}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_BackdoorCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : inter-CSC relationships' callback for VRRP group.
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
static void VRRP_GROUP_BackdoorCallback(void)
{
    L_THREADGRP_Handle_T tg_handle = VRRP_PROC_COMM_GetVrrpTGHandle();
    UI32_T  member_id;

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    VRRP_MGR_BACKDOOR_MainMenu();
    L_THREADGRP_Leave(tg_handle, member_id);
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for VRRP group.
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
void VRRP_GROUP_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("VRRP",
        SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY, VRRP_GROUP_BackdoorCallback);

}

/* FUNCTION NAME:  VRRP_GROUP_EnterTransitionMode
 * PURPOSE:
 *    This function will invoke all enter transition mode function in VRRP GROUP.
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
static void VRRP_GROUP_EnterTransitionMode(void)
{
    VRRP_INIT_EnterTransitionMode();
}

/* FUNCTION NAME:  VRRP_GROUP_SetTransitionMode
 * PURPOSE:
 *    This function will invoke all set transition mode function in VRRP GROUP.
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
static void VRRP_GROUP_SetTransitionMode(void)
{
    VRRP_INIT_SetTransitionMode();
}

/* FUNCTION NAME:  VRRP_GROUP_EnterMasterMode
 * PURPOSE:
 *    This function will invoke all enter master mode function in VRRP GROUP.
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
static void VRRP_GROUP_EnterMasterMode(void)
{
    VRRP_INIT_EnterMasterMode();
}

/* FUNCTION NAME:  VRRP_GROUP_EnterSlaveMode
 * PURPOSE:
 *    This function will invoke all enter slave mode function in VRRP Group.
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
static void VRRP_GROUP_EnterSlaveMode(void)
{
    VRRP_INIT_EnterSlaveMode();
}

/* FUNCTION NAME:  VRRP_GROUP_ProvisionComplete
 * PURPOSE:
 *    This function will invoke all provision complete function in VRRP Group.
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

static void VRRP_GROUP_ProvisionComplete(void)
{
    //VRRP_INIT_ProvisionComplete();
}

/* FUNCTION NAME:  VRRP_GROUP_Mgr_Thread_Function_Entry
 * PURPOSE:
 *    This is the entry function for the MGR thread of VRRP GROUP.
 *
 * INPUT:
 *    arg   --  not used.
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
static void VRRP_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipcmsgq_handle;
    L_THREADGRP_Handle_T tg_handle = VRRP_PROC_COMM_GetVrrpTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    UI8_T                msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(VRRP_MGR_IPCMsg_T))];
    BOOL_T               need_resp;
    SYSFUN_Msg_T         *msgbuf_p;
    SYS_TYPE_Stacking_Mode_T  stacking_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
    UI32_T  provision_completed;
    void             *vrrp_timer_id;
    UI32_T               system_ticks = SYSFUN_GetSysTick();
    SYSLOG_OM_RecordOwnerInfo_T vrrpErrorLogS;

    msgbuf_p = (SYSFUN_Msg_T*)msgbuf;

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_VRRP_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    vrrp_timer_id = SYSFUN_PeriodicTimer_Create();    
    if(SYSFUN_PeriodicTimer_Start(vrrp_timer_id, VRRP_TYPE_DFLT_TIMER_TICKS, VRRP_GROUP_TIMER_EVENT) != TRUE)
    {       
        if (vrrpTaskErrorLogFlag[0] != 1){
            vrrpErrorLogS.level = SYSLOG_LEVEL_ERR;
            vrrpErrorLogS.module_no = SYS_MODULE_VRRP;
            vrrpErrorLogS.function_no = 3;
            vrrpErrorLogS.error_no = 1;
            vrrpTaskErrorLogFlag[0] = 1;
        }
        return;
    }

    while(1)
    {
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE|
                             VRRP_GROUP_TIMER_EVENT|
                             VRRP_GROUP_PROVISION_COMPLETE_EVENT,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;
        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            VRRP_GROUP_SetTransitionMode();
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

        if (local_events & VRRP_GROUP_TIMER_EVENT)
        {
            if(stacking_mode == SYS_TYPE_STACKING_MASTER_MODE)
            {
                if (provision_completed)
                {
                    UI32_T pass_time;
                    UI32_T min_expire_time;
                    UI32_T prev_time_interval;
                    UI32_T current_ticks = SYSFUN_GetSysTick();

                    /* pass time is declared as unsigned integer, 
                     * we don't have to handle systick wrap around case.
                     */
                    pass_time = current_ticks - system_ticks;
                    system_ticks = current_ticks;
                    VRRP_MGR_ProcessTimerEvent(pass_time, &min_expire_time);

                    /* If minimum expire time is shorter than previos time interval,
                     * restart periodic time interval to minimum expire time;
                     * If minimum expire time is default periodic time interval and 
                     * different from previous time interval, 
                     * reset it to default time interval
                     */
                    if(!SYSFUN_PeriodicTimer_Get(vrrp_timer_id, &prev_time_interval))
                        VRRP_BD(TIMER, "Failed to get periodic timer");

                    if(min_expire_time != prev_time_interval)
                    {   
                        VRRP_BD(TIMER, "Restart periodic timer to %lu ticks", min_expire_time);
                        SYSFUN_PeriodicTimer_Restart(vrrp_timer_id, min_expire_time);
                    }
                }
            }
            
            local_events ^= VRRP_GROUP_TIMER_EVENT;
            
        }


        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            if(SYSFUN_ReceiveMsg(ipcmsgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                VRRP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
                }

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
                    case SYS_MODULE_VRRP:
                        need_resp=VRRP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        VRRP_GROUP_EnterMasterMode();

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

                        /* store timer id 
                         */
                        VRRP_MGR_SetTimerId(vrrp_timer_id);
                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        VRRP_GROUP_EnterSlaveMode();

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
                        VRRP_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;


                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        VRRP_GROUP_ProvisionComplete(); 
                                                
                        stacking_mode = SYS_TYPE_STACKING_MASTER_MODE;
                        provision_completed = TRUE;


                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                    
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        VRRP_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case     SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                      VRRP_GROUP_HandleHotInertion(msgbuf_p);
                      
                       msgbuf_p->msg_size=0;
                       need_resp=TRUE;
                       break;
                       
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                      VRRP_GROUP_HandleHotRemoval(msgbuf_p);
                      
                       msgbuf_p->msg_size=0;
                       need_resp=TRUE;
                        break;
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
                        need_resp=FALSE;
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                }

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipcmsgq_handle, msgbuf_p)!=SYSFUN_OK))
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

    } /* end of while(1) */
}

/* FUNCTION NAME:  VRRP_GROUP_Create_All_Threads
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup.
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
 *
 */
void VRRP_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;   

    /* Create Thread to handle the MGR message */
    if(SYSFUN_SpawnThread(SYS_BLD_VRRP_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_VRRP_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_VRRP_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          VRRP_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CSCGroup2 MGR thread fail.\n", __FUNCTION__);
    }

}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : VRRP_GROUP_HandleHotInertion
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
static void VRRP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return ;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

  
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : VRRP_GROUP_HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a  dut removal in CSCGroup1.
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
static void VRRP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return ;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf; 
}
#endif


 /* FUNCTION NAME: VRRP_GROUP_VrrpReceivePacketCallbackHandler
  *----------------------------------------------------------------------------------

 * PURPOSE: As long as  received a VRRP packet,it calls
 *          this function to request VRRP callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T  ifindex    -- the ifindex of vlan from where receive this packet.
 *      UI8_T    dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    src_mac   -- the source MAC address of this packet.
 *      UI32_T   ingress_vid    -- the vid of ingress
 *      UI32_T   src_port  -- the source port of the packet
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void VRRP_GROUP_VrrpReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI32_T    packet_length,
                                       UI32_T    ifindex,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    ingress_vid,
                                       UI32_T    src_port)
{
    VRRP_VM_RxVrrpPkt(mref_handle_p, ifindex, dst_mac, src_mac);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_RifActive_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal to VRRP_MGR to indicate that rifActive.
 * INPUT    : ifindex  -- the ifindex  of active rif
 *               addr_p  --  the addr_p of active rif
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/

static void VRRP_GROUP_RifActive_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI32_T ip_addr = 0;
    UI32_T ip_mask = 0;
    UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

    IP_LIB_ArraytoUI32(addr_p->addr, &ip_addr);
    IP_LIB_CidrToMask(addr_p->preflen, byte_mask);  
    IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

    VRRP_MGR_SignalRifUp(ifindex, ip_addr, ip_mask);

    return;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_RifDown_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal to VRRP_MGR to indicate that rifDown.
 * INPUT    : ifindex  -- the ifindex  of active rif
 *               addr_p  --  the addr_p of active rif
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static void VRRP_GROUP_RifDown_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI32_T ip_addr = 0;
    UI32_T ip_mask = 0;
    UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

    IP_LIB_ArraytoUI32(addr_p->addr, &ip_addr);
    IP_LIB_CidrToMask(addr_p->addrlen, byte_mask);
    IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

    VRRP_MGR_SignalRifDown(ifindex, ip_addr, ip_mask);

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_L3IfDestroy_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal to VRRP_MGR to indicate that layer 3 interface destroy.
 * INPUT    : ifindex  -- the ifindex  of destroy layer 3 interface
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/

static void VRRP_GROUP_L3IfDestroy_CallBack(UI32_T ifindex)
{
    VRRP_MGR_SignalVlanInterfaceDelete(ifindex);

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : VRRP_GROUP_HandleSysCallbackIPCMsg
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
static void VRRP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VRRP_GROUP_VrrpReceivePacketCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VRRP_GROUP_RifActive_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VRRP_GROUP_RifDown_CallBack);
            break;
            
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VRRP_GROUP_L3IfDestroy_CallBack);
            break;    

         default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }

}


