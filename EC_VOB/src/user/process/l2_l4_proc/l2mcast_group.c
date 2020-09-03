/* MODULE NAME:  l2mcast_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mcast group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/12/2007 - Wakka Tu, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "l_mm.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "l2mcast_group.h"
#include "vlan_lib.h"

#if (SYS_CPNT_L2MCAST == TRUE)
#include "igv3snp_init.h"
#include "igv3snp_mgr.h"
#include "igv3snp_defines.h"
#include "l2mcast_init.h"
#include "l2mcast_mgr.h"
#include "msl_pmgr.h"
#endif/*SYS_CPNT_L2MCAST*/

#include "mldsnp_mgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#include "xstp_om.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* 0.1second one tick
 */
#define L2MCAST_GROUP_TIMER_TICKS_100_MS (SYS_BLD_TICKS_PER_SECOND/IGMPSNP_ONE_SEC_TEN_TICKS)
#define L2MCAST_GROUP_EVENT_100_MS       0x0001L

#define L2MCAST_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(L2MCAST_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef union {
    SYS_CALLBACK_MGR_LPort_CBData_T                   sys_callback_mgr_lport_cbdata;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T             sys_callback_mgr_trunk_member_cbdata;
    SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T sys_callback_mgr_vlan_member_delete_by_trunk_cbdata;
    SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T     sys_callback_mgr_if_oper_status_changed_cbdata;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T             sys_callback_mgr_vlan_destroy_cbdata;
    SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T           sys_callback_mgr_vlan_member_add_cbdata;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T      sys_callback_mgr_l2mux_receive_packet_cbdata;
    SYS_CALLBACK_MGR_LportTcChange_CBData_T           sys_callback_mgr_lport_tc_change_cbdate;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    SYS_CALLBACK_MGR_REFINEList_CBData_T              sys_callback_mgr_refinelist_cbdata;
#endif
#if(SYS_CPNT_IGMPAUTH == TRUE)
    SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T    sys_callback_mgr_announce_radius_igmp_result_cbdata;
#endif
} L2MCAST_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(L2MCAST_GROUP_Syscallback_CBData_T))];
} L2MCAST_GROUP_Syscallback_Msg_T;

/* union all data type used for MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union L2MCAST_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_L2MCAST == TRUE)
    IGMPSNP_MGR_IPCMsg_T              igmpsnp_mgr_ipcmsg;
#endif
    BACKDOOR_MGR_Msg_T                backdoor_mgr_ipcmsg;
    L2MCAST_GROUP_Syscallback_Msg_T   sys_callback_ipcmsg;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                     cmgr_ipcmsg;
#endif
} L2MCAST_GROUP_MGR_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for l2mcast_group mgr thread
 */
static UI8_T l2mcast_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(L2MCAST_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)

 static void L2MCAST_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg);
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_InitiateProcessResource
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
void L2MCAST_GROUP_InitiateProcessResource(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_InitiateProcessResources();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for L2MCAST Group.
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
void L2MCAST_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_Create_InterCSC_Relation();
#endif
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_SetTransitionMode
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
static void L2MCAST_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_EnterTransitionMode
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
static void L2MCAST_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_EnterMasterMode
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
static void L2MCAST_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_EnterSlaveMode
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
static void L2MCAST_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_EnterSlaveMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_ProvisionComplete
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
static void L2MCAST_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_ProvisionComplete();
#endif
}


#if 0
static void L2MCAST_GROUP_LPortOperUpCallbackHandler(UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_LPortOperUp(lport_ifindex);
#endif
}

static void L2MCAST_GROUP_LPortNotOperUpCallbackHandler(UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_LPortNotOperUp(lport_ifindex);
#endif
}
#endif

static void L2MCAST_GROUP_LPortEnterForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex)
{
    #if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_LPortEnterForwardingCallbackHandler(xstp_id, lport_ifindex);
    #endif
}

static void L2MCAST_GROUP_LPortLeaveForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex)
{
  #if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_LPortLeaveForwardingCallbackHandler(xstp_id, lport_ifindex);
  #endif
}

static void L2MCAST_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_TrunkMemberAdd1st(trunk_ifindex, lport_ifindex);

    msl_pmgr_trk_memb_1st_add(IGMPSNP_DFLT_INSTANCE_ID, trunk_ifindex, lport_ifindex);
#endif/*SYS_CPNT_L2MCAST*/
}

static void L2MCAST_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_TrunkMemberAdd(trunk_ifindex, lport_ifindex);


    msl_pmgr_trk_memb_add(IGMPSNP_DFLT_INSTANCE_ID, trunk_ifindex, lport_ifindex);
#endif/*SYS_CPNT_L2MCAST*/
}

static void L2MCAST_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    msl_pmgr_trk_memb_del(IGMPSNP_DFLT_INSTANCE_ID, trunk_ifindex, lport_ifindex);

    L2MCAST_MGR_TrunkMemberDelete(trunk_ifindex, lport_ifindex);
#endif/*SYS_CPNT_L2MCAST*/
}

static void L2MCAST_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    msl_pmgr_trk_memb_last_del (IGMPSNP_DFLT_INSTANCE_ID, trunk_ifindex, lport_ifindex);

    L2MCAST_MGR_TrunkMemberDeleteLst(trunk_ifindex, lport_ifindex);
#endif/*SYS_CPNT_L2MCAST*/
}

static void L2MCAST_GROUP_VlanMemberDeleteByTrunkCallbackHandler(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    #if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_VlanMemberDeletedCallBack(vlan_ifindex, lport_ifindex);
    #endif
#endif
}

static void L2MCAST_GROUP_VlanCreateCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    UI32_T vid;

    VLAN_OM_ConvertFromIfindex (vlan_ifindex, &vid);
    msl_pmgr_l2_vlan_add(IGMPSNP_DFLT_INSTANCE_ID, vid);

    L2MCAST_MGR_VlanCreate(vlan_ifindex, vlan_status);

#endif/*SYS_CPNT_L2MCAST*/

}

static void L2MCAST_GROUP_VlanDestroyCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    UI32_T vid;

    L2MCAST_MGR_VlanDestroy(vlan_ifindex, vlan_status);

    VLAN_OM_ConvertFromIfindex (vlan_ifindex, &vid);
    msl_pmgr_l2_vlan_del(IGMPSNP_DFLT_INSTANCE_ID, vid);

#endif/*SYS_CPNT_L2MCAST*/

}


static void L2MCAST_GROUP_VlanMemberAddCallbackHandler(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    UI32_T vid;

    VLAN_OM_ConvertFromIfindex (vlan_ifindex, &vid);
    msl_pmgr_l2_port_add(IGMPSNP_DFLT_INSTANCE_ID, vid, lport_ifindex);

    L2MCAST_MGR_VlanMemberAdd(vlan_ifindex, lport_ifindex, vlan_status);

#endif/*SYS_CPNT_L2MCAST*/
}


#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static void
L2MCAST_GROUP_VlanMemberAddVlanListCallbackHandler(UI8_T *vlanlist,
                       UI32_T port, UI32_T status)
{
  UI32_T vid, vifindex;

  msl_pmgr_l2_port_vlanlist_add(IGMPSNP_DFLT_INSTANCE_ID, vlanlist, port);

  for(vid= 1; vid <= SYS_ADPT_MAX_NBR_OF_VLAN; vid++)
  {
      if (!SYS_CALLBACK_MGR_IS_MEMBER(vlanlist,vid))
          continue;

      VLAN_OM_ConvertToIfindex(vid, &vifindex);

#if (SYS_CPNT_L2MCAST == TRUE)
      L2MCAST_MGR_VlanMemberAdd(vifindex, port, status);
#endif/*SYS_CPNT_L2MCAST*/
  }

}



static void
L2MCAST_GROUP_VlanMemberDelVlanListCallbackHandler(UI8_T *vlanlist,
                       UI32_T port, UI32_T status)
{
  UI32_T vid, vifindex;

  for(vid= 1; vid <= SYS_ADPT_MAX_NBR_OF_VLAN; vid++)
  {
      if (!SYS_CALLBACK_MGR_IS_MEMBER(vlanlist,vid))
          continue;

      VLAN_OM_ConvertToIfindex(vid, &vifindex);

#if (SYS_CPNT_L2MCAST == TRUE)
      L2MCAST_MGR_VlanMemberDelete(vifindex, port, status);
#endif/*SYS_CPNT_L2MCAST*/
  }

    msl_pmgr_l2_port_vlanlist_del(IGMPSNP_DFLT_INSTANCE_ID, vlanlist, port);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_GROUP_VlanListCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when vlans are  created/ports added to vlan/ports removed from vlans.
 *
 * INPUT   : subid -- the detail action to do
 *                  msg--- msg data
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
 static void L2MCAST_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg)
{
        UI32_T vid_ifindex,lport_index,vlan_status,vid;

        switch(subid)
        {
          case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            vlan_status = msg->arg.arg1.value;
            for(vid= 1; vid<=SYS_ADPT_MAX_NBR_OF_VLAN;vid++)
            {
             if(SYS_CALLBACK_MGR_IS_MEMBER(msg->list.vlanlist,vid))
             {
               VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
               L2MCAST_GROUP_VlanCreateCallbackHandler(vid_ifindex,vlan_status);
             }
           }
          break;

          case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            lport_index = msg->arg.arg2.value[0];
            vlan_status = msg->arg.arg2.value[1];

            L2MCAST_GROUP_VlanMemberAddVlanListCallbackHandler(&(msg->list.vlanlist), lport_index, vlan_status);
          break;

          case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
             lport_index = msg->arg.arg2.value[0];
             vlan_status = msg->arg.arg2.value[1];

             L2MCAST_GROUP_VlanMemberDelVlanListCallbackHandler(&(msg->list.vlanlist), lport_index, vlan_status);
          break;
            default:
            break;
       }

}

#endif
static void L2MCAST_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    UI32_T vid;

    L2MCAST_MGR_VlanMemberDelete(vlan_ifindex, lport_ifindex, vlan_status);

    VLAN_OM_ConvertFromIfindex (vlan_ifindex, &vid);
    msl_pmgr_l2_port_del(IGMPSNP_DFLT_INSTANCE_ID, vid, lport_ifindex);

#endif/*SYS_CPNT_L2MCAST*/

}

static void L2MCAST_GROUP_ErpsTcn(
    UI32_T  rp_ifidx[2],
    BOOL_T  rp_isblk[2])
{
}

/* Added by Steven.Gao on 2008/5/26, for IPI IGMPSNP */
static void L2MCAST_GROUP_XSTPTopoChange(BOOL_T is_mstp_mode,UI32_T xstid,UI32_T lport,BOOL_T is_root,UI32_T tc_timer/*,UI8_T *vlan_bit_map*/)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_XSTPTopoChange(is_mstp_mode, xstid, lport, is_root, tc_timer/*, vlan_bit_map*/);
#endif
}

static void L2MCAST_GROUP_VlanPortModeChange(UI32_T lport, UI32_T vlan_mode)
{
    L2MCAST_MGR_VlanPortModeChange(lport, vlan_mode);
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_HandleHotInertion
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
static void L2MCAST_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return ;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    L2MCAST_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_HandleHotRemoval
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
static void L2MCAST_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return ;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    L2MCAST_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif

static void L2MCAST_GROUP_ReceiveIgmpsnpPacketCallback(L_MM_Mref_Handle_T   *mref_handle_p,
                                                   UI8_T            dst_mac[6],
                                                   UI8_T            src_mac[6],
                                                   UI16_T           tag_info,
                                                   UI16_T           type,
                                                   UI32_T           pkt_length,
                                                   UI32_T           lport)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_ReceiveIgmpsnpPacketCallback(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, lport);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif
}
static void L2MCAST_GROUP_ReceiveMldsnpPacketCallback(L_MM_Mref_Handle_T    *mref_handle_p,
                                                   UI8_T            dst_mac[6],
                                                   UI8_T            src_mac[6],
                                                   UI16_T           tag_info,
                                                   UI16_T           type,
                                                   UI32_T           pkt_length,
                                                   UI32_T           ip_ext_opt_len,
                                                   UI32_T           lport)
{
#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_MGR_ReceiveMldsnpPacketCallback(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, ip_ext_opt_len, lport);
#endif
}

#if(SYS_CPNT_IGMPAUTH == TRUE)
static void L2MCAST_GROUP_RadiusIgmpAuthCallbackHandler(UI32_T   result,
    UI32_T   auth_port,
    UI32_T   ip_address,
    UI8_T    *auth_mac,
    UI32_T   vlan_id,
    UI32_T   src_ip,
    UI8_T    msg_type)
{

    L2MCAST_MGR_RadiusIgmpAuthCallback(result, auth_port, src_ip, ip_address, vlan_id, auth_mac, msg_type);
}
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_HandleSysCallbackIPCMsg
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
static void L2MCAST_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        #if 0
        /* Layer 2 port Oper Status UP/Down, including phyiscal ports, trunk ports and LACP ports */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_LPortOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_LPortNotOperUpCallbackHandler);
            break;
        #endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_LPortEnterForwardingCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_LPortLeaveForwardingCallbackHandler);
            break;
        /* TRUNK member-ship change */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

        /* VLAN Create/Delete */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanCreateCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanDestroyCallbackHandler);
            break;

        /* VLAN Member Ship Change */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanMemberAddCallbackHandler);
            break;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanListCallbackHandler);
               break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanMemberDeleteByTrunkCallbackHandler);
            break;

        /* IGMP Snooping Packet receiving */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_ReceiveIgmpsnpPacketCallback);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_ReceiveMldsnpPacketCallback);
            break;

#if(SYS_CPNT_IGMPAUTH == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_IGMPAUTH_RESULT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_RadiusIgmpAuthCallbackHandler);
            break;
#endif

        /* XSTP Topology Change */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_XSTP_LPORT_TC_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_XSTPTopoChange);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_VlanPortModeChange);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ERPS_FLUSH_FDB_NOTIFY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L2MCAST_GROUP_ErpsTcn);
            break;
        default:
            SYSFUN_Debug_Printf("\r\n%s: received callback_event that is not handled(%d)",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void L2MCAST_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
{
    CMGR_IpcMsg_T *cmgr_msg_p;

    if (ipc_msg_p == NULL)
    {
        return;
    }

    cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;
    switch (cmgr_msg_p->type.cmd)
    {
    case CMGR_IPC_VLAN_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
            {
                L2MCAST_GROUP_VlanDestroyCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            L2MCAST_GROUP_VlanCreateCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            L2MCAST_GROUP_VlanDestroyCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        break;

    case CMGR_IPC_VLAN_MEMBER_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_2 ==
                    TRUE)
            {
                L2MCAST_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
            }
            L2MCAST_GROUP_VlanMemberAddCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            L2MCAST_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;

    case CMGR_IPC_PORT_VLAN_MODE_CHANGE:
        L2MCAST_GROUP_VlanPortModeChange(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;

    case CMGR_IPC_XSTP_PORT_STATE_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
            {
                L2MCAST_GROUP_LPortLeaveForwardingCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            L2MCAST_GROUP_LPortEnterForwardingCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            L2MCAST_GROUP_LPortLeaveForwardingCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        break;

    case CMGR_IPC_XSTP_PORT_TOPO_CHANGE:
    {
        UI32_T  tc_timer;
        BOOL_T  is_mstp_mode, is_root;

        if (XSTP_OM_GetPortTcInfo(cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2, &is_mstp_mode,
                &is_root, &tc_timer) == FALSE)
        {
            printf("%s: failed to get xstp tc info (%lu, %lu)\r\n", __func__,
                (unsigned long) cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
                (unsigned long) cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
            break;
        }
        L2MCAST_GROUP_XSTPTopoChange(is_mstp_mode,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2, is_root, tc_timer);
    }
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_Mgr_Thread_Function_Entry
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
static void L2MCAST_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    L_THREADGRP_Handle_T tg_handle;
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)l2mcast_group_mgrtd_ipc_buf;
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    SYSFUN_MsgQ_T        ipc_msgq_handle_data;
    UI32_T               member_id, received_events, local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    BOOL_T               need_resp = FALSE;
    void                 *l2mcast_timer_id;

    /* join the thread group
     */
    tg_handle = L2_L4_PROC_COMM_GetL2McastGroupTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_L2MCAST_GROUP_MGR_THREAD_PRIORITY, &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.", __FUNCTION__);
        return;
    }


    /* create data ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_L2MCAST_GROUP_DATA_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle_data) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.", __FUNCTION__);
        return;
    }


    l2mcast_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(l2mcast_timer_id, L2MCAST_GROUP_TIMER_TICKS_100_MS, L2MCAST_GROUP_EVENT_100_MS);

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
    while (1)
    {

        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE|
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             L2MCAST_GROUP_EVENT_100_MS,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            L2MCAST_GROUP_SetTransitionMode();
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
            if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                L2MCAST_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

                /* handle request message based on cmd
                 */

                switch (msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
                    case SYS_MODULE_IGMPSNP:

                        need_resp = IGMPSNP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_MLDSNP == TRUE)
                   case SYS_MODULE_MLDSNP:
                        need_resp= MLDSNP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
                    case SYS_MODULE_SYS_CALLBACK:

                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        L2MCAST_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        L2MCAST_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    case SYS_MODULE_BACKDOOR:

                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:

                        L2MCAST_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer auth_protocol group has
                         * entered transition mode but lower layer auth_protocol groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer auth_protocol group. In this case, the IPCFAIL
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

                        L2MCAST_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer auth_protocol group has
                         * entered transition mode but lower layer auth_protocol group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer auth_protocol group. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:

                        L2MCAST_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        L2MCAST_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        L2MCAST_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        L2MCAST_GROUP_HandleHotRemoval(msgbuf_p);
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
                        printf("\r\n%s: Invalid IPC req cmd.", __FUNCTION__);
                        need_resp = FALSE;
                }

                /* release thread group execution permission
                 */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
                    printf("\r\n%s: SYSFUN_SendResponseMsg fail.", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
            if (SYSFUN_ReceiveMsg(ipc_msgq_handle_data, 0, SYSFUN_TIMEOUT_NOWAIT,
                L2MCAST_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch (msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
                    case SYS_MODULE_SYS_CALLBACK:

                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        L2MCAST_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
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
                        printf("\r\n%s: Invalid IPC req cmd.", __FUNCTION__);
                        need_resp = FALSE;
                }

                /* release thread group execution permission
                 */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle_data, msgbuf_p) != SYSFUN_OK))
                    printf("\r\n%s: SYSFUN_SendResponseMsg fail.", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        if (local_events & L2MCAST_GROUP_EVENT_100_MS)
        {
            if (L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

            IGMPSNP_MGR_TimerHandler();

#if(SYS_CPNT_MLDSNP == TRUE)
            MLDSNP_MGR_ProcessTimerTick();
#endif
            if (L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

            local_events ^= L2MCAST_GROUP_EVENT_100_MS;
        }

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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_L2MCAST_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_Create_All_Threads
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
void L2MCAST_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_INIT_Create_Tasks();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_L2MCAST_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_L2MCAST_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_L2MCAST_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          L2MCAST_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("\r\n%s:Spawn CSCGroup1 MGR thread fail.", __FUNCTION__);
    }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_L2MCAST_GROUP, thread_id, SYS_ADPT_L2MCAST_GROUP_SW_WATCHDOG_TIMER);
#endif
}
