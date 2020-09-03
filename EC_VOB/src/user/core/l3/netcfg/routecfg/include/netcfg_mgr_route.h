/* Module Name: netcfg_mgr_route.h
 * Purpose:
 *      NETCFG_MGR_ROUTE provides Layer-3 routing configuration management access-point for
 *      upper layer.
 *
 * Notes:
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       12/18/2007 --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

#ifndef NETCFG_MGR_ROUTE_H
#define NETCFG_MGR_ROUTE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "l_inet.h"
#include "leaf_4001.h"
#include "sys_adpt.h"
#include "l_inet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE sizeof(union NETCFG_MGR_ROUTE_IPCMsg_Type_U)
/* Operation on ipv4 entries */
#define NETCFG_MGR_ROUTE_FLAGS_IPV4                1
/* Operation on ipv6 entries */
#define NETCFG_MGR_ROUTE_FLAGS_IPV6		    (1 << 1)

#define IP_FORWARDING_ENABLED			1
#define IP_FORWARDING_DISABLED			0


/* For Exceptional Handler */
enum NETCFG_MGR_ROUTE_FUN_NO_E
{
    NETCFG_MGR_ROUTE_ADD_DHCP_DEFAULT_GATEWAY_FUN_NO = 1,
    NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_DYNAMIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_DEFAULT_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_GET_IP_CIDR_ROUTE_NUMBER_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_ALL_DYNAMIC_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_ALL_STATIC_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_GET_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_GET_NEXT_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_GET_CONFIG_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_GET_NEXT_CONFIG_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_GET_CONFIG_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_GET_NEXT_CONFIG_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_GET_NEXT_ALL_TYPE_IP_CIDR_ROUTE_ENTRY_FUN_NO,
    NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_GET_DEFAULT_ROUTE_FUN_NO,
    NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_COMPATIBLE_FUN_NO,
    NETCFG_MGR_ROUTE_DELETE_DEFAULT_ROUTE_COMPATIBLE_FUN_NO,
    NETCFG_MGR_ROUTE_SET_IP_FORWARDING_FUN_NO,
    NETCFG_MGR_ROUTE_SET_IP_NO_FORWARDING_FUN_NO,
    NETCFG_MGR_ROUTE_SIGNAL_RIF_ACTIVE_FUN_NO,
    NETCFG_MGR_ROUTE_SIGNAL_RIF_NOT_IN_SERVICE_FUN_NO,
    NETCFG_MGR_ROUTE_SIGNAL_RIF_DESTROY_FUN_NO,
    NETCFG_MGR_ROUTE_SET_ROW_STATUS_FUN_NO,
    NETCFG_MGR_ROUTE_GET_ROW_STATUS_FUN_NO

};

/* definitions of command in CSCA which will be used in ipc message
 */
enum
{
/* Route Configuration */
    NETCFG_MGR_ROUTE_IPCCMD_SETDHCPDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPFORWARDING,
    NETCFG_MGR_ROUTE_IPCCMD_DISABLEIPFORWARDING,
    NETCFG_MGR_ROUTE_IPCCMD_GETIPFORWARDINGSTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_ADDSTATICROUTE,
    NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTE,
    NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTEBYNETWORK,
    NETCFG_MGR_ROUTE_IPCCMD_SETIPCIDRROUTEROWSTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTEROWSTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_GETNEXTIPCIDRROUTEROWSTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_ADDDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEDHCPDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_GETDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEDYNAMICROUTE,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEALLROUTES,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEALLSTATICROUTES,
    NETCFG_MGR_ROUTE_IPCCMD_DELETEALLDYNAMICROUTES,
    NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETNEXTIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETALLTYPEIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETNEXTALLTYPEIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETNEXTSTATICIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTENUMBER,
    NETCFG_MGR_ROUTE_IPCCMD_SETSTATICIPCIDRROUTEMETRICS,
    NETCFG_MGR_ROUTE_IPCCMD_GETNEXTRUNNINGSTATICIPCIDRROUTEENTRY,
    NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGDEFAULTGATEWAY,
    NETCFG_MGR_ROUTE_IPCCMD_GETREVERSEPATHIPMAC,
    NETCFG_MGR_ROUTE_IPCCMD_SETMROUTESTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGMROUTESTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_SETM6ROUTETATUS,
    NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGM6ROUTESTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_GETM6ROUTESTATUS,
    NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPSWROUTE,
    NETCFG_MGR_ROUTE_IPCCMD_GETIPSWROUTE,
    NETCFG_MGR_ROUTE_IPCCMD_SETECMPBALANCEMODE,
    NETCFG_MGR_ROUTE_IPCCMD_FINDBESTROUTE
};


/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(ip_addr) \
    ((ip_addr[0]==0) && \
     (ip_addr[1]==0) && \
     (ip_addr[2]==0) && \
     (ip_addr[3]==0))

#define NETCFG_MGR_ROUTE_IP6_IS_ALL_ZEROS(ip_addr) \
    ((ip_addr[0]==0) && \
     (ip_addr[1]==0) && \
     (ip_addr[2]==0) && \
	 (ip_addr[3]==0) && \
	 (ip_addr[4]==0) && \
	 (ip_addr[5]==0) && \
	 (ip_addr[6]==0) && \
	 (ip_addr[7]==0) && \
	 (ip_addr[8]==0) && \
	 (ip_addr[9]==0) && \
	 (ip_addr[10]==0) && \
	 (ip_addr[11]==0) && \
	 (ip_addr[12]==0) && \
	 (ip_addr[13]==0) && \
	 (ip_addr[14]==0) && \
     (ip_addr[15]==0))

#define NETCFG_MGR_ROUTE_IP_CMP(ip_addr1, ip_addr2) \
    (memcmp((ip_addr1), (ip_addr2), SYS_ADPT_IPV4_ADDR_LEN))

#define NETCCFG_MGR_ROUTE_IP_APPLY_MASK(ip_addr, mask, result) \
    result[0]=ip_addr[0] & mask[0];\
    result[1]=ip_addr[1] & mask[1];\
    result[2]=ip_addr[2] & mask[2];\
    result[3]=ip_addr[3] & mask[3]


/* Macro function for calculation of ipc msg_buf size based on structure name
 */
#define NETCFG_MGR_ROUTE_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_ROUTE_IPCMsg_T*)0)->data.field_name))
/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in csca pmgr and mgr
 */
typedef struct
{
    union NETCFG_MGR_ROUTE_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        BOOL_T  bool_v;
        UI8_T   ui8_v;
        I8_T    i8_v;
        UI32_T  ui32_v;
        UI16_T  ui16_v;
        I32_T   i32_v;
        I16_T   i16_v;
        UI8_T   ip4_v[SYS_ADPT_IPV4_ADDR_LEN];
//	UI8_T   ip6_v[SYS_ADPT_IPV6_ADDR_LEN];
        int     int_v;

        NETCFG_TYPE_IpCidrRouteEntry_T ip_cidr_route_entry_v;
	    L_INET_AddrIp_T addr_ip_v;

    	struct
    	{
    		UI32_T vr_id;
    		UI8_T addr_type;
    		UI32_T forward_status;
    	} ip_forwarding_v;

    	struct
    	{
    		L_INET_AddrIp_T src_ip;
    		L_INET_AddrIp_T dst_ip;
    	} addrip_addrip;

        struct
        {
            L_INET_AddrIp_T dst_addr;
            L_INET_AddrIp_T src_addr;
            L_INET_AddrIp_T nexthop_addr;
            UI32_T out_ifindex;
        } route_v;


        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        } u32a1_u32a2;

        struct
        {
            UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN];
            UI32_T distance;
        } ip_mask_next_hop_distance;


        struct
        {
            UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN];
            UI32_T tos;
            UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN];
            UI32_T u32_a1;
        } ip_mask_tos_next_hop_u32a1;

        struct
        {
            UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN];
        } ip_mask;

        struct
        {
            UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN];
            UI32_T u32_a1;
        } ip_u32a1;

        struct
        {
            UI8_T ip_a1[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T ip_a2[SYS_ADPT_IPV4_ADDR_LEN];
        } ipa1_ipa2;

        struct
        {
            UI8_T ip_a1[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T ip_a2[SYS_ADPT_IPV4_ADDR_LEN];
            UI32_T u32_a1;
        } ipa1_ipa2_u32a1;

        struct
        {
            L_INET_AddrIp_T addr_1;
            L_INET_AddrIp_T addr_2;
            UI8_T mac[SYS_ADPT_MAC_ADDR_LEN];
        } arg_grp_addrx2_mac;

        struct
        {
            L_INET_AddrIp_T ip_a1;
            L_INET_AddrIp_T ip_a2;
            UI32_T          u32_a3;
            UI32_T          u32_a4;
        } ipa1_ipa2_u32a3_u32a4;

    } data; /* contains the supplemntal data for the corresponding cmd */
} NETCFG_MGR_ROUTE_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_MGR_ROUTE_SetTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ROUTE_SetTransitionMode(void);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ROUTE_EnterTransitionMode (void);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_ROUTE_EnterMasterMode (void);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_ROUTE_EnterSlaveMode (void);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_ROUTE_Create_InterCSC_Relation(void);


/*---------------------------------------
 *  Initialization
 *---------------------------------------
 */
/* FUNCTION NAME : NETCFG_MGR_ROUTE_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_ROUTE used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_ROUTE_InitiateProcessResources(void);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_MGR_ROUTE.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T NETCFG_MGR_ROUTE_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


UI32_T NETCFG_MGR_ROUTE_DeleteDhcpDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway);

/* FUNCTION NAME : NETCFG_MGR_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query
 *
 * OUTPUT:
 *      *forward_status_p   -- 1 (IP_FORWARDING_ENABLED)
 *                             0 (IP_FORWARDING_DISABLED)
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_ROUTE_GetIpForwardingStatus(UI32_T vr_id, UI8_T addr_type, UI32_T *forward_status_p);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_AddStaticRouteToTunnelInterface
 *
 * FUNCTION: Add static route to tunnel interface when tunnel interface is active .
 *
 * INPUT:
 *      nhop_tunnel_ifindex    -- nexthop tunnel ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *      NETCFG_TYPE_TABLE_FULL
 *
 * NOTES:
 *      Add configured static route to nsm and kernel when tunnel interface is active
 */

UI32_T NETCFG_MGR_ROUTE_AddStaticRouteToTunnelInterface(UI32_T nhop_tunnel_ifindex);

/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_DeleteStaticRouteToTunnelInterface
 *
 * FUNCTION: Delete static route to tunnel interface when tunnel interface is inactive .
 *
 * INPUT:
 *      nhop_tunnel_ifindex    -- nexthop tunnel ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *      NETCFG_TYPE_TABLE_FULL
 *
 * NOTES:
 *      Delete configured static route to nsm and kernel when tunnel interface is inactive
 */

UI32_T NETCFG_MGR_ROUTE_DeleteStaticRouteToTunnelInterface(UI32_T nhop_tunnel_ifindex);
#endif /* end of SYS_CPNT_IP_TUNNEL */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME: NETCFG_MGR_ROUTE_UpdateStaticRoute
 * PURPOSE:
 *      update static routes to linux kernel.
 *
 * INPUT:
 *      ifindex     -- ifindex of the rif
 *      addr_p      -- ip address of the rif
 *      is_active   -- whether the rif is active or de-active
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      Normal vlan interface will set interface rif to both NSM and kernel,
 *      but craft interface's rif will only set to kernel. So when craft interface
 *      rif activated, NSM won't help us to write static routes to kernel, we need
 *      to update those static routes whose next-hop is in craft interface's subnet.
 *      Further more, where rif on interface up/down, we need to check whether need
 *      to delete/add static route which conflict with that rif into kernel.
 */
UI32_T NETCFG_MGR_ROUTE_UpdateStaticRoute(UI32_T ifindex, L_INET_AddrIp_T *addr_p, BOOL_T is_active);
#endif /* SYS_CPNT_CRAFT_PORT == TRUE */

#if (SYS_CPNT_NSM != TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ROUTE_SignalRifUp
 *-----------------------------------------------------------------------------
 * PURPOSE : handler function of RIF activate for NETCFG_MGR_ROUTE
 *
 * INPUT   : ipaddr_p  -- RIF's ip address
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
void NETCFG_MGR_ROUTE_SignalRifUp(L_INET_AddrIp_T* ipaddr_p);


/* FUNCTION NAME : NETCFG_MGR_ROUTE_L3IfOperStatusChanged_CallBack
 * PURPOSE:
 *      Handle the callback message for L3 interface operation status change.
 *
 * INPUT:
 *      vid_ifindex -- interface index
 *      status : VAL_ifOperStatus_up, interface up.
 *               VAL_ifOperStatus_down, interface down.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_MGR_ROUTE_L3IfOperStatusChanged_CallBack(UI32_T ifindex, UI32_T oper_status);
#endif

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx);

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#endif /* NETCFG_MGR_ROUTE */

