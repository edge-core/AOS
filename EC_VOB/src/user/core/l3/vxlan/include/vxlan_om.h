/* MODULE NAME: VXLAN_OM.H
 * PURPOSE:
 *   Declares the APIs for VXLAN OM.
 * NOTES:
 *
 * HISTORY:
 *   04/21/2015 -- Kelly Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef VXLAN_OM_H
#define VXLAN_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysrsc_mgr.h"
#include "l_sort_lst.h"
#include "l_inet.h"
#include "vxlan_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define VXLAN_OM_IPCMSG_TYPE_SIZE sizeof(union VXLAN_OM_IpcMsg_Type_U)
/* command used in IPC message
 */
enum
{
    VXLAN_OM_IPC_GETUDPDSTPORT,
    VXLAN_OM_IPC_GETRUNNINGUDPDSTPORT,
    VXLAN_OM_IPC_GETFLOODRVTEPBYVNIIP,
    VXLAN_OM_IPC_GETNEXTFLOODRVTEPBYVNI,
    VXLAN_OM_IPC_GETNEXTFLOODRVTEP,
    VXLAN_OM_IPC_GETFLOODMULTICASTBYVNI,
    VXLAN_OM_IPC_GETNEXTFLOODMULTICAST,
    VXLAN_OM_IPC_GETNEXTPORTVLANVNIMAPBYVNI,
    VXLAN_OM_IPC_GETVLANVNIMAPENTRY,
    VXLAN_OM_IPC_GETNEXTVLANVNIMAPENTRY,
    VXLAN_OM_IPC_GETVNIBYVFI,
    VXLAN_OM_IPC_GETNEXTVNIENTRY,
    VXLAN_OM_IPC_GETSRCIF,
    VXLAN_OM_IPC_GETRUNNINGSRCIF,
    VXLAN_OM_IPC_GETVLANANDVNIBYVFI,
    VXLAN_OM_IPC_GETLPORTOFACCESSPORT,
    VXLAN_OM_IPC_GETNEXTACCESSPORT,
    VXLAN_OM_IPC_GETPORTVLANVNIMAPENTRY,
    VXLAN_OM_IPC_GETACCESSVXLANPORT,
    VXLAN_OM_IPC_GETVNIENTRY,
    VXLAN_OM_IPC_GETVLANNLPORTOFACCESSPORT,
};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in VXLAN_OM_IpcMsg_T.data
 */
#define VXLAN_OM_GET_MSG_SIZE(field_name)                        \
            (VXLAN_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((VXLAN_OM_IpcMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    L_INET_AddrIp_T ip;
    UI32_T          ifindex;
}VXLAN_OM_RVtepNhop_T;

typedef struct
{
    UI32_T              vni;            /* key */
    L_INET_AddrIp_T     ip;             /* key. remote VTEP IP address */
    UI32_T              vfi;
    L_INET_AddrIp_T     s_ip;           /* local VTEP IP address on vlan that be mapped vni */
    L_INET_AddrIp_T     nexthops_ip_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];     /* nexthop address for remote VTEP ip */
    UI32_T              nexthops_if_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];     /* nexthop interface for remote VTEP ip */
    UI32_T              nexthops_lport_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];  /* egress port of nexthop for remote VTEP ip */
    UI32_T              nexthop_cnt;
    UI16_T              vid;            /* egress VLAN specified by multicast group IP */
    UI32_T              lport;          /* egress port specified by multicast group IP */
    UI32_T              uc_vxlan_port;  /* assign VxLAN port number by chip */
    UI32_T              mc_vxlan_port;  /* assign VxLAN port number by chip */
    UI8_T               mac_ar[SYS_ADPT_MAC_ADDR_LEN];
}VXLAN_OM_RVtep_T;

typedef struct
{
    UI32_T      vni; /* key */
    UI32_T      vfi;
    UI32_T      bcast_group;
    UI32_T      nbr_of_acc_port;
}VXLAN_OM_VNI_T;

typedef struct
{
    UI32_T      flags_reserved; /* flag 8bits, reserved 24bits */
    UI32_T      vni_reserved;   /* vni 24bits, reserved 8bits */
}VXLAN_OM_VxlanHeader_T;

/* IPC message structure
 */
typedef struct
{
    union VXLAN_OM_IpcMsg_Type_U
    {
        UI32_T  cmd;
        BOOL_T  ret_bool;
        UI32_T  ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI16_T                  arg_ui16;
        UI32_T                  arg_ui32;
        struct
        {
            UI16_T arg_ui16;
            UI32_T arg_ui32;
        } arg_grp_ui16_ui32;
        struct
        {
            UI16_T arg_ui16;
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui16_ui32_ui32;
        VXLAN_OM_RVtep_T    arg_rvtep;
        VXLAN_OM_VNI_T      arg_vni_entry;
    } data;
} VXLAN_OM_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: VXLAN_OM_InitiateSystemResources
 * PURPOSE : Initiate system resource for VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_InitiateSystemResources(void);

/* FUNCTION NAME: VXLAN_OM_AttachSystemResources
 * PURPOSE : Attch system resource for VXLAN OM  in the context of the calling process.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_AttachSystemResources(void);

/* FUNCTION NAME: VXLAN_OM_GetShMemInfo
 * PURPOSE : Provide shared memory information for SYSRSC.
 * INPUT   : None.
 * OUTPUT  : segid_p   -- shared memory segment ID.
 *           senglen_p -- length of the shared memory segment.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME: VXLAN_OM_ClearAll
 * PURPOSE : Clear all static memory used by VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_ClearAll(void);

/* FUNCTION NAME: VXLAN_OM_SetAllDefault
 * PURPOSE : Set default values for all data used by VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetAllDefault(void);

/* FUNCTION NAME: VXLAN_OM_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_SetUdpDstPort(UI16_T port_no);

/* FUNCTION NAME: VXLAN_OM_GetUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetUdpDstPort(UI16_T *port_no);

/* FUNCTION NAME: VXLAN_OM_GetRunningUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_OM_GetRunningUdpDstPort(UI16_T *port_no);

/* FUNCTION NAME: VXLAN_OM_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_AddFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_GetFloodRVtepByVniIp
 * PURPOSE : Get remote VTEP with specified VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetFloodRVtepByVniIp(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_GetFloodRVtepByVni
 * PURPOSE : Get next remote VTEP with VNI.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodRVtepByVni(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_GetNextFloodRVtep
 * PURPOSE : Get next remote VTEP with VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_AddFloodMulticast
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_AddFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_DelFloodMulticast
 * PURPOSE : Delete remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_GetFloodMulticastByVni
 * PURPOSE : Get remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetFloodMulticastByVni(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_GetNextFloodMulticast
 * PURPOSE : Get Next remote VTEP.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_OM_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : For delete, vni = 0;
 */
UI32_T VXLAN_OM_SetVlanVniMap(UI16_T vid, UI32_T vni);

/* FUNCTION NAME: VXLAN_OM_DelVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelVlanVniMap(UI16_T vid, UI32_T vni);

/* FUNCTION NAME: VXLAN_OM_SetPortVlanVniMap
 * PURPOSE : Configure Port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : 1. For delete, vni = 0;
 *           2. vid = 0 means per-port
 */
UI32_T VXLAN_OM_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni);

/* FUNCTION NAME: VXLAN_OM_DelPortVlanVniMap
 * PURPOSE : Configure port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vid = 0 means per-port
 */
UI32_T VXLAN_OM_DelPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni);

/* FUNCTION NAME: VXLAN_OM_GetNextPortVlanVniMapByVni
 * PURPOSE : Get next port (and VLAN) to VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * OUTPUT  : lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextPortVlanVniMapByVni(UI32_T vni, UI32_T *lport_p, UI16_T *vid_p);

/* FUNCTION NAME - VXLAN_OM_GetVlanVniMapEntry
 * PURPOSE : Get VLAN-VNI mapping.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetVlanVniMapEntry(UI16_T vid, UI32_T *vni);

/* FUNCTION NAME - VXLAN_OM_GetPortVlanVniMapEntry
 * PURPOSE : Get Port+VLAN to VNI mapping.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetPortVlanVniMapEntry(UI32_T lport, UI16_T vid, UI32_T *vni);

/* FUNCTION NAME - VXLAN_OM_GetNextVlanVniMapEntry
 * PURPOSE : Get next VLAN-VNI mapping.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vid -- VLAN ID
 *           vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vid=0 to get the first entry
 */
UI32_T VXLAN_OM_GetNextVlanVniMapEntry(UI16_T *vid, UI32_T *vni);

/* FUNCTION NAME - VXLAN_OM_SetVniEntry
 * PURPOSE : Set a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_SetVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME - VXLAN_OM_DelVniEntry
 * PURPOSE : Set a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_DelVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME - VXLAN_OM_GetVniEntry
 * PURPOSE : Get a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME - VXLAN_OM_GetNextVniEntry
 * PURPOSE : Get a next VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vni=0 to get the first entry
 */
UI32_T VXLAN_OM_GetNextVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME: VXLAN_OM_SetAccessVxlanPort
 * PURPOSE : Add/ Delete access VTEP.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 *           vxlan_port  -- output vxlan port.
 *           is_add      -- TRUE is create, FALSE is delete.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : If is_add = FALSE, shall input vxlan_port = 0.
 */
void VXLAN_OM_SetAccessVxlanPort(UI16_T vid, UI32_T lport, UI32_T vxlan_port, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_OM_GetVniByVfi
 * PURPOSE : Get VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : None.
 * RETURN  : vni, returns -1 when error
 * NOTES   : None.
 */
I32_T VXLAN_OM_GetVniByVfi(UI32_T vfi);

/* FUNCTION NAME: VXLAN_OM_GetAccessVxlanPort
 * PURPOSE : Get access VXLAN port.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 * OUTPUT  : vxlan_port  -- output vxlan port.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetAccessVxlanPort(UI16_T vid, UI32_T lport);

/* FUNCTION NAME: VXLAN_OM_SetL3If
 * PURPOSE : Create a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 *           l3_if       -- L3 interface value.
 *           is_add      -- TRUE is create, FALSE is delete.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : If is_add = FALSE, shall input l3_if = 0.
 */
void VXLAN_OM_SetL3If(UI16_T vid, UI32_T l3_if, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_OM_GetL3If
 * PURPOSE : Get a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : l3_if       -- L3 interface value.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetL3If(UI16_T vid);

/* FUNCTION NAME: VXLAN_OM_SetL3IfUseCount
 * PURPOSE : Set count of a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 *           is_add      -- TRUE is add, FALSE is minus.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetL3IfUseCount(UI16_T vid, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_OM_GetL3IfUseCount
 * PURPOSE : Get use count of a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : None.
 * RETURN  : use count   -- use count
 * NOTES   : None.
 */
UI16_T VXLAN_OM_GetL3IfUseCount(UI16_T vid);

/* FUNCTION NAME - VXLAN_OM_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T VXLAN_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME: VXLAN_OM_SetDebug
 * PURPOSE : Set debug flag.
 * INPUT   : flag.
 *           is_on
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetDebug(UI32_T flag, BOOL_T is_on);

/* FUNCTION NAME: VXLAN_OM_GetDebug
 * PURPOSE : Get debug flag.
 * INPUT   : flag.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetDebug(UI32_T flag);

/* FUNCTION NAME: VXLAN_OM_GetTotalRVtepNumber
 * PURPOSE : Get the total number of remote VTEP entries
 * INPUT   : is_uc   -- Ture: for unicast case.
 *                   -- FALSE: for multicast case.
 * OUTPUT  : None
 * RETURN  : used number.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetTotalRVtepNumber(BOOL_T is_uc);

/* FUNCTION NAME: VXLAN_OM_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_SetSrcIf(UI32_T ifindex);

/* FUNCTION NAME: VXLAN_OM_GetSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface index
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetSrcIf(UI32_T *ifindex);

/* FUNCTION NAME: VXLAN_OM_GetRunningSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface index
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_OM_GetRunningSrcIf(UI32_T *ifindex_p);

/* FUNCTION NAME: VXLAN_OM_GetVlanAndVniByVfi
 * PURPOSE : Get VLAN and VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : vid         -- vlan ID
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetVlanAndVniByVfi(UI32_T vfi, UI16_T *vid, UI32_T *vni);

/* FUNCTION NAME: VXLAN_OM_GetVlanMemberCounter
 * PURPOSE : Get member counter for specified VLAN.
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : member counter  -- output vlan member counter.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetVlanMemberCounter(UI16_T vid);

/* FUNCTION NAME: VXLAN_OM_GetLportOfAccessPort
 * PURPOSE : Get logical port number of access port on specified VLAN.
 * INPUT   : vxlan_port  -- vxlan port nunmber of access port
 *           vid         -- vlan ID
 * OUTPUT  : lport       -- logical port number
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetLportOfAccessPort(UI32_T vxlan_port, UI16_T vid, UI32_T *lport);

/* FUNCTION NAME: VXLAN_OM_GetNextAccessVxlanPort
 * PURPOSE : Get next access VXLAN port.
 * INPUT   : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 *           vxlan_port_p  -- output vxlan port.
 * OUTPUT  : TRUE/FALSE
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetNextAccessVxlanPort(UI16_T *vid_p, UI32_T *lport_p, UI32_T *vxlan_port_p);

/* FUNCTION NAME: VXLAN_OM_GetVlanNlportOfAccessPort
 * PURPOSE : Get VID and port from access VXLAN port.
 * INPUT   : vxlan_port    -- vxlan port.
 * OUTPUT  : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetVlanNlportOfAccessPort(UI32_T vxlan_port, UI16_T *vid_p, UI32_T *lport_p);
#endif /* End of VXLAN_OM_H */