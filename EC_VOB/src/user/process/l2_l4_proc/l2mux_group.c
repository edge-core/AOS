/* MODULE NAME:  l2mux_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mux group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/19/2007 - KH shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l2mux_group.h"
#include "l2_l4_proc_comm.h"
#include "lan_type.h"
#include "l_stdlib.h"
#if (SYS_CPNT_L2MUX == TRUE)
    #include "l2mux_init.h"
    #include "l2mux_mgr.h"
#endif

#if (SYS_CPNT_IML == TRUE)
    #include "iml_mgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_NDSNP == TRUE)
#include "ndsnp_pmgr.h"
#endif

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
#include "netcfg_pom_nd.h"
#endif

#if (SYS_CPNT_VRRP  == TRUE )
#include "vrrp_pmgr.h"
#include "vrrp_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for CSCGroup1 MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union L2MUX_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_MGR_IPCMsg_T  l2mux_mgr_ipcmsg;
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_IPCMsg_T    iml_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_msg_ipcmsg;
} L2MUX_GROUP_MGR_MSG_T;

#define L2MUX_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(L2MUX_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void L2MUX_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void L2MUX_GROUP_SetTransitionMode(void);
static void L2MUX_GROUP_EnterTransitionMode(void);
static void L2MUX_GROUP_EnterMasterMode(void);
static void L2MUX_GROUP_EnterSlaveMode(void);
static void L2MUX_GROUP_ProvisionComplete(void);
static void L2MUX_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for l2mux_group mgr thread
 */
static UI8_T l2mux_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_InitiateProcessResource
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
void L2MUX_GROUP_InitiateProcessResource(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_InitiateProcessResources();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_NDSNP == TRUE)
    NDSNP_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    NETCFG_POM_ND_InitiateProcessResource();
#endif

#if (SYS_CPNT_VRRP == TRUE)
    VRRP_PMGR_InitiateProcessResource();
    VRRP_POM_InitiateProcessResource();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2MUX_GROUP_Create_InterCSC_Relation
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
void L2MUX_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_Create_InterCSC_Relation();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_Create_All_Threads
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
void L2MUX_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_CreateTask();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_L2MUX_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_L2MUX_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_L2MUX_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          L2MUX_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn L2MUX Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_L2MUX_GROUP, thread_id, SYS_ADPT_L2MUX_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_Mgr_Thread_Function_Entry
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
static void L2MUX_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle =L2_L4_PROC_COMM_GetL2muxGroupTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)l2mux_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
    int sockfd;
#endif

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_L2MUX_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
    if (SYSFUN_OK == SYSFUN_CreateIPCSocketServer(SYS_BLD_SYS_CALLBACK_SOCKET_SERVER_L2MUX_OF_PACKETIN, &sockfd, 1 /*non-blocking*/))
    {
        SYS_CALLBACK_MGR_SetSocket(SYS_MODULE_L2MUX, sockfd);
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
            L2MUX_GROUP_SetTransitionMode();
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
                L2MUX_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_L2MUX == TRUE)
                    case SYS_MODULE_L2MUX:
                        need_resp=L2MUX_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_IML == TRUE)
                    case SYS_MODULE_IML:
                        need_resp=IML_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        L2MUX_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        L2MUX_GROUP_EnterMasterMode();

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
                        L2MUX_GROUP_EnterSlaveMode();

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
                        L2MUX_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        L2MUX_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
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
       if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_L2MUX_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif
    } /* end of while(1) */
    L_THREADGRP_Leave(tg_handle, member_id);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_SetTransitionMode
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
static void L2MUX_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_EnterTransitionMode
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
static void L2MUX_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_EnterMasterMode
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
static void L2MUX_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_EnterSlaveMode
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
static void L2MUX_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_L2MUX == TRUE)
    L2MUX_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_IML == TRUE)
    IML_MGR_EnterSlaveMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all provision complete function in CSCGroup1.
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
static void L2MUX_GROUP_ProvisionComplete(void)
{
   return;
}

/* FUNCTION NAME: L2MUX_GROUP_L2muxReceiveSTAPacketCallbackHandler
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request STA callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_GROUP_L2muxReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no,
                                       UI32_T    packet_class)
{
    #if (SYS_CPNT_L2MUX == TRUE)

    if (L2MUX_MGR_PreprocessPkt_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                             pkt_length,unit_no,port_no,packet_class))
    {
        /* MREF had been handled and released
         */
        return;
    }

    switch (packet_class)
    {
        case LAN_TYPE_IP_PACKET:
            /* syscallback handle ND packet dispatch here 
             * mref release should be control in SYS_CALLBACK_MGR()*/
             if(FALSE == SYS_CALLBACK_MGR_HandleReceiveNdPacket(
                                    SYS_MODULE_L2MUX,
                                    mref_handle_p,
                                    dst_mac,
                                    src_mac,
                                    tag_info,
                                    type,
                                    pkt_length,
                                    unit_no,
                                    port_no))
            {
                
                /* don't need to announce to upper layer */
                break;
            }
            
            /* annouce to upper layer */
#if (SYS_CPNT_IML == TRUE)
            IML_MGR_RxLanPacket(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length,
                                unit_no, port_no);
#else
            L2MUX_MGR_IP_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                      pkt_length,unit_no,port_no);
#endif
            break;
        case LAN_TYPE_STA_PACKET:
            L2MUX_MGR_STA_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                       pkt_length,unit_no,port_no);
            break;
        case LAN_TYPE_GVRP_PACKET:
            L2MUX_MGR_GVRP_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                        pkt_length,unit_no,port_no);
            break;

#if (SYS_CPNT_LLDP == TRUE)
        case LAN_TYPE_LLDP_PACKET:
            L2MUX_MGR_LLDP_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                        pkt_length,unit_no,port_no);
            break;
#endif  /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_CLUSTER == TRUE)
        case LAN_TYPE_CLUSTER_PACKET:
            L2MUX_MGR_CLUSTER_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                           pkt_length,unit_no,port_no);
            break;
#endif

#if(SYS_CPNT_CFM == TRUE)
        case LAN_TYPE_CFM_PACKET:
            L2MUX_MGR_CFM_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                          pkt_length,unit_no,port_no);
            break;
#endif

#if(SYS_CPNT_PPPOE_IA == TRUE)
        case LAN_TYPE_PPPOED_PACKET:
            L2MUX_MGR_PPPOED_CallbackFunc(
                mref_handle_p,  dst_mac,src_mac,    tag_info,
                type,           pkt_length,         unit_no,
                port_no);
            break;
#endif

        default:
            L_MM_Mref_Release(&mref_handle_p);
            /* printf("%s: Invalid packet class = %lu\n",__FUNCTION__,packet_class); */
            break;
    }
    #else
        L_MM_Mref_Release(&mref_handle_p);
    #endif
}

/* FUNCTION NAME: L2MUX_GROUP_L2muxReceiveIPPacketCallbackHandler
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a IP packet,it calls
 *          this function to request IML function to handle this packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES: None.
 */
void L2MUX_GROUP_L2muxReceiveIPPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no,
                                       UI32_T    packet_class)
{
#if (SYS_CPNT_IML == TRUE)
    IML_MGR_RxLanPacket(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length,
                    unit_no, port_no);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif /* SYS_CPNT_IML */
}


void L2MUX_GROUP_ImlReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    ether_type,
                                       UI32_T    packet_length,
                                       UI32_T    l_port)
{
#if (SYS_CPNT_IML == TRUE)
    IML_MGR_RecvPacket(mref_handle_p,dst_mac,src_mac,tag_info,ether_type,
                       packet_length,l_port);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif
}


static void 
L2MUX_GROUP_RxSnoopDhcpPacketCallbackHandler(
  L_MM_Mref_Handle_T *mref_handle_p,
  UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
  UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
  UI16_T              tag_info,
  UI16_T              ether_type,
  UI32_T              pkt_len,
  UI32_T              ing_lport)
{

#if (SYS_CPNT_IML == TRUE)
        IML_MGR_RecvPacket(mref_handle_p,
                           dst_mac,
                           src_mac,
                           tag_info,
                           ether_type,
                           pkt_len,
                           ing_lport);
#else
        L_MM_Mref_Release(&mref_handle_p);
#endif

}



/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_HandleSysCallbackIPCMsg
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
static void L2MUX_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, L2MUX_GROUP_L2muxReceivePacketCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, L2MUX_GROUP_L2muxReceiveIPPacketCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, L2MUX_GROUP_RxSnoopDhcpPacketCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}





