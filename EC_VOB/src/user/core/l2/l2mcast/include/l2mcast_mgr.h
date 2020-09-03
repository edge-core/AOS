/*-----------------------------------------------------------------------------
 * Module   : l2mcast_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Process VLAN/SWCTRL callback message.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 12/03/2001 - Lyn Yeh, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef	L2MCAST_MGR_H
#define	L2MCAST_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <sys_type.h>
#include "sysfun.h"
#include "l_mm.h"
/* NAMING CONSTANT DECLARATIONS
 */
typedef enum /* function number */
{
    L2MCAST_MGR_L2MCAST_TASK_TASK_FUNCTION_NUMBER = 0,
    L2MCAST_MGR_L2MCAST_TASK_DISPATCH_MSG_FUNCTION_NUMBER,
    L2MCAST_MGR_L2MCAST_MGR_ROWSTATUS_FUNCTION_NUMBER

}L2MCAST_MGR_L2MCAST_Function_Number_T ;


typedef enum /* error code */
{
    L2MCAST_MGR_L2MCAST_TASK_ERROR_NUMBER = 0,
    L2MCAST_MGR_L2MCAST_TASK_DISPATCH_MSG_ERROR_NUMBER,
    L2MCAST_MGR_L2MCAST_MGR_ROWSTATUS_ERROR_NUMBER

}L2MCAST_MGR_L2MCAST_Error_Number_T;

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function initiates the IGMP snooping module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_SetTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterSlaveMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReportSyslogMessage
 *-------------------------------------------------------------------------
 * PURPOSE : This function is an API to use syslog to report error message.
 * INPUT   : error_type    - the type of error, defined in l2mcast_mgr.h
 *           error_msg     - defined in syslog_mgr.h
 *           function_name - the specific function which error is occured.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
/* void L2MCAST_MGR_ReportSyslogMessage(UI8_T error_type, UI8_T error_msg, UI8_T *function_name); */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE : The function notify us provision is completed.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ProvisionComplete(void);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* -----------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for l2mcast mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T L2MCAST_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


/* Callback functions
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortOperUp
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the port
 *           operation mode is up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortOperUp(UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortNotOperUp
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the port
 *           operation mode is not up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortNotOperUp(UI32_T lport_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortEnterForwardingCallbackHandler
 *-------------------------------------------------------------------------
 * PURPOSE : spanning tree notify port is enter forwarding
 *           operation mode is not up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortEnterForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortLeaveForwardingCallbackHandler
 *-------------------------------------------------------------------------
 * PURPOSE : spanning tree notify port is enter forwarding
 *           operation mode is not up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortLeaveForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberAdd1st
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the first
 *           trunk member is added.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberAdd1st(UI32_T trunk_ifindex, UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberAdd
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the
 *           2nd/3rd/4th trunk member is added.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberAdd(UI32_T trunk_ifindex, UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode
 * INPUT   : SWCTRL uses this funtion to notify IGMPSNP that the last
 *           2nd/3rd/4th trunk member is deleted.
 * OUTPUT  : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberDelete(UI32_T trunk_ifindex, UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberDeleteLast
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that last trunk
 *           member is deleted.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberDeleteLst(UI32_T trunk_ifindex, UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanMemberAdd
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan member
 *           is deleted.
 * INPUT   : vlan_ifindex  - vlan interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanMemberAdd(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan member
 *           is deleted.
 * INPUT   : vlan_ifindex  - vlan interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanMemberDelete(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanDestory
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan is
 *           destroyed.
 * INPUT   : vlan_ifindex - vlan interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanDestroy(UI32_T vlan_ifindex, UI32_T vlan_status);

void L2MCAST_MGR_VlanCreate(UI32_T vlan_ifindex, UI32_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ErpsTcn
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to process EPRS TCN
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ErpsTcn(
    UI32_T  rp_ifidx[2],
    BOOL_T  rp_isblk[2]);

/* Added by Steven.Gao on 2008/5/26, for IPI IGMPSNP */
void L2MCAST_MGR_XSTPTopoChange(BOOL_T is_mstp_mode,UI32_T xstid,UI32_T lport,BOOL_T is_root,UI32_T tc_timer/*,UI8_T *vlan_bit_map*/);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanPortModeChange
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to vlan port mode change
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanPortModeChange(UI32_T lport, UI32_T vlan_port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReceiveIgmpsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE : This function process receiver igmp packet
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ReceiveIgmpsnpPacketCallback(L_MM_Mref_Handle_T	*mref_handle_p,
                                                UI8_T               dst_mac[6],
                                                UI8_T 	            src_mac[6],
                                                UI16_T 	            tag_info,
                                                UI16_T              type,
                                                UI32_T              pkt_length,
                                                UI32_T              lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReceiveMldsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE : This function process receiver mld packet
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ReceiveMldsnpPacketCallback(L_MM_Mref_Handle_T * mref_handle_p, 
                                             UI8_T dst_mac [ 6 ], 
                                             UI8_T src_mac [ 6 ], 
                                             UI16_T tag_info, 
                                             UI16_T type, 
                                             UI32_T pkt_length, 
                                             UI32_T ip_ext_opt_len, 
                                             UI32_T lport);

#if(SYS_CPNT_IGMPAUTH == TRUE)            
void L2MCAST_MGR_RadiusIgmpAuthCallback(UI32_T result, UI32_T port_ifindex, UI32_T src_ip, UI32_T dst_ip, UI32_T vid, UI8_T *src_mac, UI8_T msg_type);
#endif

#endif /* L2MCAST_MGR_H */
