/* MODULE NAME:  vxlan_pmgr.c
 * PURPOSE:
 *     VXLAN PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "l_inet.h"
#include "vxlan_mgr.h"
#include "vxlan_type.h"
#include "string.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define VXLAN_PMGR_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = VXLAN_MGR_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    VXLAN_MGR_IpcMsg_T *msg_p = (VXLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_VXLAN;\
    msgbuf_p->msg_size = msg_size;

#define VXLAN_PMGR_SEND_WAIT_MSG_P() \
do{\
    if(SYSFUN_OK!=SYSFUN_SendRequestMsg(msgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,\
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))\
    {\
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);\
        return VXLAN_TYPE_RETURN_ERROR;\
    }\
}while(0)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T msgq_handle= 0;


/* FUNCTION NAME : VXLAN_PMGR_InitiateProcessResources
 * PURPOSE : Initiate process resources for VXLAN_PMGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 * NOTES   : None.
 */
BOOL_T VXLAN_PMGR_InitiateProcessResources(void)
{
    if(SYSFUN_OK!=SYSFUN_GetMsgQ(SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
     &msgq_handle))
    {
         printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
         return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME: VXLAN_PMGR_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetUdpDstPort(UI16_T port_no)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_ui16);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_SETUDPDSTPORT;

    msg_p->data.arg_ui16 = port_no;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_AddFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_ADDFLOODRVTEP;

    msg_p->data.arg_grp_ui32_ipaddr.arg_ui32 = vni;
    msg_p->data.arg_grp_ui32_ipaddr.arg_ipaddr = *ip_p;

    VXLAN_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_DelFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_DELFLOODRVTEP;
    msg_p->data.arg_grp_ui32_ipaddr.arg_ui32 = vni;
    msg_p->data.arg_grp_ui32_ipaddr.arg_ipaddr = *ip_p;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_AddFloodMulticast
 * PURPOSE : Flooding to which multicast group, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           m_ip_p -- IP address of multicast group
 *           vid -- VLAN ID
 *           lport -- logical port
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_AddFloodMulticast(UI32_T vni, L_INET_AddrIp_T *m_ip_p, UI16_T vid, UI32_T lport)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr_ui16_ui32);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_ADDFLOODMULTICAST;
    msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui32_1 = vni;
    msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ipaddr = *m_ip_p;
    msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui16 = vid;
    msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui32_2 = lport;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_DelFloodMulticast
 * PURPOSE : Delete flooding multicast group.
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_DelFloodMulticast(UI32_T vni)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_DELFLOODMULTICAST;
    msg_p->data.arg_ui32 = vni;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_AddVpn
 * PURPOSE : Create Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_PMGR_AddVpn(UI32_T vni)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_ADDVPN;
    msg_p->data.arg_ui32 = vni;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_DeleteVpn
 * PURPOSE : Destroy a Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_NOT_EXISTED /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_PMGR_DeleteVpn(UI32_T vni)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_DELETEVPN;
    msg_p->data.arg_ui32 = vni;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}


/* FUNCTION NAME: VXLAN_PMGR_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetVlanVniMap(UI16_T vid, UI32_T vni, BOOL_T is_add)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_grp_ui16_ui32_bool);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_SETVLANVNIMAP;
    msg_p->data.arg_grp_ui16_ui32_bool.arg_ui16 = vid;
    msg_p->data.arg_grp_ui16_ui32_bool.arg_ui32 = vni;
    msg_p->data.arg_grp_ui16_ui32_bool.arg_bool = is_add;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_SetPortVlanVniMap
 * PURPOSE : Configure port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni, BOOL_T is_add)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_grp_ui32_ui16_ui32_bool);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_SETPORTVLANVNIMAP;
    msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui16   = vid;
    msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui32_2 = vni;
    msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_bool   = is_add;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_PMGR_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetSrcIf(UI32_T ifindex)
{
    VXLAN_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = VXLAN_MGR_IPCCMD_SETSRCIF;

    msg_p->data.arg_ui32 = ifindex;

    VXLAN_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.ret_ui32;
}
/* LOCAL SUBPROGRAM BODIES
 */


