/* MODULE NAME: VXLAN_MGR.H
 * PURPOSE:
 *   Declares the APIs for VXLAN MGR.
 * NOTES:
 *
 * HISTORY:
 *   04/21/2015 -- Kelly Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef VXLAN_MGR_H
#define VXLAN_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_mm.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define VXLAN_MGR_IPCMSG_TYPE_SIZE sizeof(union VXLAN_MGR_IpcMsg_Type_U)
/* command used in IPC message
 */
enum VXLAN_MGR_IPCCMD_E
{
    VXLAN_MGR_IPCCMD_SETUDPDSTPORT,
    VXLAN_MGR_IPCCMD_ADDFLOODRVTEP,
    VXLAN_MGR_IPCCMD_DELFLOODRVTEP,
    VXLAN_MGR_IPCCMD_ADDFLOODMULTICAST,
    VXLAN_MGR_IPCCMD_DELFLOODMULTICAST,
    VXLAN_MGR_IPCCMD_ADDVPN,
    VXLAN_MGR_IPCCMD_DELETEVPN,
    VXLAN_MGR_IPCCMD_SETVLANVNIMAP,
    VXLAN_MGR_IPCCMD_SETPORTVLANVNIMAP,
    VXLAN_MGR_IPCCMD_SETSRCIF,
};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in VXLAN_MGR_IpcMsg_T.data
 */
#define VXLAN_MGR_GET_MSG_SIZE(field_name)                       \
            (VXLAN_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((VXLAN_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
/* IPC message structure
 */
typedef struct
{
    union VXLAN_MGR_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
        UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI16_T arg_ui16;
        UI32_T arg_ui32;
        struct
        {
            UI16_T arg_ui16;
            UI32_T arg_ui32;
            BOOL_T arg_bool;
        } arg_grp_ui16_ui32_bool;
        struct
        {
            UI32_T arg_ui32_1;
            UI16_T arg_ui16;
            UI32_T arg_ui32_2;
            BOOL_T arg_bool;
        } arg_grp_ui32_ui16_ui32_bool;
        struct
        {
            UI32_T arg_ui32_1;
            L_INET_AddrIp_T arg_ipaddr;
            UI16_T arg_ui16;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ipaddr_ui16_ui32;

        struct
        {
            UI32_T          arg_ui32;
            L_INET_AddrIp_T arg_ipaddr;
        } arg_grp_ui32_ipaddr;
    } data;
} VXLAN_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: VXLAN_MGR_InitiateSystemResources
 * PURPOSE : Initiate system resources for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_InitiateSystemResources(void);

/* FUNCTION NAME: VXLAN_MGR_Create_InterCSC_Relation
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME: VXLAN_MGR_SetTransitionMode
 * PURPOSE : Process SetTransitionMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_SetTransitionMode(void);

/* FUNCTION NAME: VXLAN_MGR_EnterTransitionMode
 * PURPOSE : Process EnterTransitionMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterTransitionMode(void);

/* FUNCTION NAME: VXLAN_MGR_EnterMasterMode
 * PURPOSE : Process EnterMasterMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterMasterMode(void);

/* FUNCTION NAME: VXLAN_MGR_EnterSlaveMode
 * PURPOSE : Process EnterSlaveMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterSlaveMode(void);


/* FUNCTION NAME: VXLAN_MGR_ProcessReceivedPacket
 * PURPOSE : Process the received VTEP packet.
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_unit      -- unit
 *           src_port      -- src_port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_ProcessReceivedPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T *dst_mac,
                                  UI8_T *src_mac,
                                  UI16_T tag_info,
                                  UI16_T type,
                                  UI32_T pkt_length,
                                  UI32_T src_unit,
                                  UI32_T src_port);

/* FUNCTION NAME: VXLAN_MGR_RouteChange_CallBack
 * PURPOSE : when nsm has ipv4/ipv6 route change, it will call back to VXLAN.
 *           If there's a remote VTEP route change, shall update driver and OM.
 * INPUT   : address_family  -- ipv4 or ipv6
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RouteChange_CallBack(UI32_T address_family);

/* FUNCTION NAME: VXLAN_MGR_VlanCreate_Callback
 * PURPOSE : Service the callback from VLAN_MGR when a vlan is created
 * INPUT   : vid_ifidx     -- specify which vlan has just been created
 *           vlan_status
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanCreate_Callback(UI32_T vid_ifindex, UI32_T vlan_status);

/* FUNCTION NAME: VXLAN_MGR_VlanDestroy_Callback
 * PURPOSE : Service the callback from VLAN_MGR when a vlan is destroyed
 * INPUT   : vid_ifidx     -- specify which vlan has just been destroyed
 *           vlan_status
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanDestroy_Callback(UI32_T vid_ifindex, UI32_T vlan_status);

/* FUNCTION NAME: VXLAN_MGR_VlanMemberAdd_Callback
 * PURPOSE : Service the callback from VXLAN_MGR when a lport is added to a
 *           vlan's member set.
 * INPUT   : vid_ifidx     -- specify which vlan's member set to be add to
 *           lport_ifindex -- specify which port to be added to the member set
 *           vlan_status   --
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanMemberAdd_Callback(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);

/* FUNCTION NAME: VXLAN_MGR_VlanMemberDelete_Callback
 * PURPOSE : Service the callback from VXLAN_MGR when a port is remove from
 *           vlan's member set.
 * INPUT   : vid_ifidx     -- specify which vlan's member set to be deleted from
 *           lport_ifindex -- specify which port to be deleted
 *           vlan_status   --
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanMemberDelete_Callback(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);

/* FUNCTION NAME: VXLAN_MGR_RifActive_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif active.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifActive_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME: VXLAN_MGR_RifDown_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif down.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifDown_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberAdd1st_CallBack
 * PURPOSE : Handle the callback event happening when the first port is added
 *           to a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk member is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberAdd_CallBack
 * PURPOSE : Service the callback from SWCTRL when the port joins the trunk
 *           as the 2nd or the following member
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk member is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberDelete_CallBack
 * PURPOSE : Handle the callback event happening when a logical port is deleted
 *           from a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberDeleteLst_CallBack
 * PURPOSE : Handle the callback event happening when the last port is deleted
 *           from a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk port is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME: VXLAN_MGR_RifCreated_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif created.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifCreated_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME: VXLAN_MGR_RifDestroyed_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif destroyed.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifDestroyed_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME: VXLAN_MGR_PortLinkDown_CallBack
 * PURPOSE : Handle the callback event happening when the link is down.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_PortLinkDown_CallBack(UI32_T ifindex);

/* FUNCTION NAME: VXLAN_MGR_PvidChange_CallBack
 * PURPOSE : Handle the callback event happening when the pvid of port change
 * INPUT   : lport_ifindex -- which logical port
 *           old_pvid      -- old pvid
 *           new_pvid      -- new pvid
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_PvidChange_CallBack(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid);

/* FUNCTION NAME: VXLAN_MGR_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetUdpDstPort(UI16_T port_no);

/* FUNCTION NAME: VXLAN_MGR_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_AddFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p);

/* FUNCTION NAME: VXLAN_MGR_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_DelFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p);

/* FUNCTION NAME: VXLAN_MGR_AddFloodMulticast
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
UI32_T VXLAN_MGR_AddFloodMulticast(UI32_T vni, L_INET_AddrIp_T *m_ip_p, UI16_T vid, UI32_T lport);

/* FUNCTION NAME: VXLAN_MGR_DelFloodMulticast
 * PURPOSE : Delete flooding multicast group.
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_DelFloodMulticast(UI32_T vni);

/* FUNCTION NAME: VXLAN_MGR_AddVpn
 * PURPOSE : Create Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_MGR_AddVpn(UI32_T vni);

/* FUNCTION NAME: VXLAN_MGR_DeleteVpn
 * PURPOSE : Destroy a Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_NOT_EXISTED /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_MGR_DeleteVpn(UI32_T vni);

/* FUNCTION NAME: VXLAN_MGR_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetVlanVniMap(UI16_T vid, UI32_T vni, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_MGR_SetPortVlanVniMap
 * PURPOSE : Configure Port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vid = 0 means per-port access port
 */
UI32_T VXLAN_MGR_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_MGR_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetSrcIfVlan(UI32_T ifindex);

/* FUNCTION NAME: VXLAN_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message for VXLAN MGR.
 * INPUT   : msgbuf_p -- input request ipc message buffer
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTES   : None.
 */
BOOL_T VXLAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME: VXLAN_MGR_TaskMain
 * PURPOSE : listen
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : never returns.
 * NOTES   : None.
 */
void VXLAN_MGR_TaskMain();
#endif /* End of VXLAN_MGR_H */