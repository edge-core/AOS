/* Module Name: NETCFG_OM_ROUTE.H
 * Purpose: Keep user configured static routing entry.
 *
 * Notes:
 *      1. In real world, user may configure multiple backup routing to same 
 *         network, but Phase2 engine, only one routing entry could be activated
 *         at Phase2 engine.
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *      12/18/2007  --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

#ifndef NETCFG_OM_ROUTE_H
#define NETCFG_OM_ROUTE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "route_mgr.h"
#include "leaf_2096.h"
#include "leaf_4001.h"
#include "l_inet.h"
#include "sys_adpt.h"

/* NAME CONSTANT DECLARATIONS
 */
#define ROUTE_VALID     1
#define ROUTE_INVALID   0

#define NETCFG_OM_ROUTE_ACTIVE_ROUTE    1
#define NETCFG_OM_ROUTE_BACKUP_ROUTE    2

#define NETCFG_OM_ROUTE_IP_CIDR_ROUTE_PROTOCOL_DHCP                1217
#define NETCFG_OM_ROUTE_IP_CIDR_ROUTE_PROTOCOL_STATIC              VAL_ipCidrRouteProto_netmgmt

#define ROUTE_NH_TYPE_IP4               1   /* IPv4 nexthop */
#define ROUTE_NH_TYPE_IP4_IFINDEX       2   /* IPv4 nexthop with ifindex */
#define ROUTE_NH_TYPE_IFINDEX           3   /* Directly connected */
#define ROUTE_NH_TYPE_IP6               4   /* IPv6 nexthop */
#define ROUTE_NH_TYPE_IP6_IFINDEX       5   /* IPv6 nexthop with ifindex */

/* Operation on ipv4 entries */
#define NETCFG_OM_ROUTE_FLAGS_IPV4                1                                       
/* Operation on ipv6 entries */
#define NETCFG_OM_ROUTE_FLAGS_IPV6		    (1 << 1)

#define IP_FORWARDING_ENABLED			1
#define IP_FORWARDING_DISABLED			0


/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_OM_ROUTE_IP_IS_ALL_ZEROS(ip_addr) \
    ((ip_addr[0]==0) && \
     (ip_addr[1]==0) && \
     (ip_addr[2]==0) && \
     (ip_addr[3]==0))

#define NETCFG_OM_ROUTE_IP_CMP(ip_addr1, ip_addr2) \
    (memcmp((ip_addr1), (ip_addr2), SYS_ADPT_IPV4_ADDR_LEN))

#define NETCFG_OM_ROUTE_IP_APPLY_MASK(ip_addr, mask, result) \
    result[0]=ip_addr[0] & mask[0];\
    result[1]=ip_addr[1] & mask[1];\
    result[2]=ip_addr[2] & mask[2];\
    result[3]=ip_addr[3] & mask[3]

#define NETCFG_OM_ROUTE_MSGBUF_TYPE_SIZE sizeof(union NETCFG_OM_ROUTE_IPCMsg_Type_U)

enum
{
    NETCFG_OM_ROUTE_IPC_GETNEXTSTATICIPCIDRROUTE = 0,
    NETCFG_OM_ROUTE_IPC_GETIPFORWARDINGSTATUS,
    NETCFG_OM_ROUTE_IPC_GETRUNNINGECMPBALANCEMODE,
    NETCFG_OM_ROUTE_IPC_GETECMPBALANCEMODE,
};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_OM_ROUTE_IPCMsg_T.data
 */
#define NETCFG_OM_ROUTE_GET_MSG_SIZE(field_name)                       \
            (NETCFG_OM_ROUTE_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_OM_ROUTE_IPCMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */

/* Keys: vr_id
 */
typedef struct NETCFG_OM_ROUTE_IpForwardingStatus_S
{   
    UI32_T vr_id;
    UI8_T status_bitmap;

}NETCFG_OM_ROUTE_IpForwardingStatus_T;

typedef struct
{
    union NETCFG_OM_ROUTE_IPCMsg_Type_U
    {
        UI32_T cmd;         /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool; /*respond bool return*/
        UI32_T result_ui32; /*respond ui32 return*/ 
    } type;

    union
    {
        BOOL_T        bool_v;
        UI32_T        ui32_v;    
        ROUTE_MGR_IpCidrRouteEntry_T         arg_route_entry;
        NETCFG_OM_ROUTE_IpForwardingStatus_T arg_ip_forwarding;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        } u32a1_u32a2;

    } data;
} NETCFG_OM_ROUTE_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : NETCFG_OM_ROUTE_InitateProcessResources
 * PURPOSE:
 *          Initialize semophore & create a list to store route entry.
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
 *      None.
 */
void NETCFG_OM_ROUTE_InitateProcessResources(void);


/* FUNCTION NAME : NETCFG_OM_ROUTE_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_OM_ROUTE.
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
BOOL_T NETCFG_OM_ROUTE_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* FUNCTION NAME : NETCFG_OM_ROUTE_EnableIpForwarding
 * PURPOSE:
 *      Enable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully enable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/* FUNCTION NAME : NETCFG_OM_ROUTE_DisableIpForwarding
 * PURPOSE:
 *      Disable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully disable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/* FUNCTION NAME : NETCFG_OM_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query 
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_GetIpForwardingStatus(NETCFG_OM_ROUTE_IpForwardingStatus_T *ifs);

/* ROUTINE NAME: NETCFG_OM_ROUTE_AddStaticIpCidrRoute
 *
 * FUNCTION: Add static route.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES: None.
 */
UI32_T NETCFG_OM_ROUTE_AddStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry);

/* ROUTINE NAME: NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute
 *
 * FUNCTION:
 *      Delete route using 4 keys.
 *
 * INPUT:
 *      entry.ip_cidr_route_dest
 *      entry.ip_cidr_route_mask  - IP mask of static route
 *      entry.ip_cidr_route_metric
 *      entry.ip_cidr_route_next_hop
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry);

/* ROUTINE NAME: NETCFG_OM_ROUTE_DeleteAllStaticIpCidrRoutes
 *
 * FUNCTION:
 *      Delete all static routing configuration.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES: None.
 */
UI32_T NETCFG_OM_ROUTE_DeleteAllStaticIpCidrRoutes(UI32_T action_flags);

/* ROUTINE NAME: NETCFG_OM_ROUTE_GetSameRoute
 *
 * FUNCTION:
 *      Get a route entry by keys.
 *      If a same route entry with different distance, delete nexthop 
 *      if nh_replace is set to TRUE;
 *
 * INPUT:
 *      entry
 *      nh_replace
 *
 * OUTPUT:
 *      entry->ip_cidr_route_distance --- if the same route is found
 *                      with different distance and nh_replace is 
 *                      set to FALSE.
 *
 * RETURN:
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      keys(entry.ip_cidr_route_dest, entry.ip_cidr_route_mask,
 *          entry.ip_cidr_route_next_hop).
 */
UI32_T NETCFG_OM_ROUTE_GetSameRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry, BOOL_T nh_replace);


/* ROUTINE NAME: NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute
 *
 * FUNCTION:
 *      Get total number of nexthops of a route.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      nh_nr.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry, UI32_T *nh_nr);


/* ROUTINE NAME: NETCFG_OM_ROUTE_IsRouteExisted
 *
 * FUNCTION:
 *      Get route using 4 keys.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry.ip_cidr_route_dest, entry.ip_cidr_route_mask,
 *          entry.ip_cidr_route_metric,  entry.ip_cidr_route_next_hop).
 */
UI32_T NETCFG_OM_ROUTE_IsRouteExisted(ROUTE_MGR_IpCidrRouteEntry_T *entry);

/* ROUTINE NAME: NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute
 *
 * FUNCTION:
 *      Get next route using 4 keys.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry->ip_cidr_route_dest,
 *             entry->ip_cidr_route_mask,
 *             entry->ip_cidr_route_next_hop,
 *             entry->ip_cidr_route_distance)
 */
UI32_T NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry);

/* ROUTINE NAME: ROUTECFG_OM_CompareRoute
 *
 * FUNCTION:
 *      Compare the keys of 2 route entries.
 *
 * INPUT:
 *      e1
 *      e2
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      > 0 : if e1 > e2
 *      < 0 : if e1 < e2
 *      = 0 : if e1 = e2
 *
 * NOTES:
 *      Compare ip_cidr_route_dest, ip_cidr_route_mask,
 *              ip_cidr_route_tos, ip_cidr_route_next_hop.
 */
//int ROUTECFG_OM_CompareRoute(ROUTE_MGR_IpCidrRouteEntry_T e1, ROUTE_MGR_IpCidrRouteEntry_T e2);

/* ROUTINE NAME: ROUTECFG_OM_ToggleDebug
 *
 * FUNCTION:
 *      Toggle Debug output.
 *
 * INPUT:
 *      mode :
 *          = 0, disable output
 *          else, enable output.
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
//void ROUTECFG_OM_ToggleDebug(int mode);

//void ROUTECFG_OM_DumpRoute(ROUTE_MGR_IpCidrRouteEntry_T entry);

//void ROUTECFG_OM_DumpRouteWithStr(char *str, ROUTE_MGR_IpCidrRouteEntry_T entry);

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)

/*
 * ROUTINE NAME: ROUTECFG_OM_GetManagementVlanDefaultGateway
 *
 * FUNCTION: Get default gateway address for management vlan.
 *
 * INPUT: None.
 *
 * OUTPUT: mgmt_default_gateway_p - pointer to store default gateway for management vlan
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_EXIST : gateway address do not exist
 *      NETCFG_TYPE_INVALID_ARG : Invalid argument
 *
 * NOTES: 
 *      1. The management vlan function is for L3 product. Only L3 product should call this function.
 *      2. Currently, just support one default gateway.
 */
UI32_T ROUTECFG_OM_GetManagementVlanDefaultGateway(UI8_T mgmt_default_gateway_p[SYS_ADPT_IPV4_ADDR_LEN]);

#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber
 *
 * FUNCTION: Get number of static route.
 *
 * INPUT: None.
 *
 * OUTPUT: static_route_num - Number of static route.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber(UI32_T action_flags, UI32_T *static_route_num);


/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber
 *
 * FUNCTION: Update the number of static route based on the current count.
 *
 * INPUT: changed_count  -- The changed count that will apply to the current
 *                          number of static route. Positive value will increase
 *                          the count and negative value will decrease the count.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES: 1. This is private om api that will only called by functions in
 *           routecfg.c
 */
void NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber(UI32_T action_flags, I32_T changed_count);

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_SetStaticIpCidrRouteNumber
 *
 * FUNCTION: Set the number of static route.
 *
 * INPUT: count  -- The count to be set.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES: 
 */
void NETCFG_OM_ROUTE_SetStaticIpCidrRouteNumber(UI32_T action_flags, UI32_T count);

/*
 * ROUTINE NAME: ROUTECFG_OM_SetDefaultGatewayCompatible
 *
 * FUNCTION: Set the user configured default gateway for L2 product
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_SetDefaultGatewayCompatible(L_INET_AddrIp_T *gateway);

/*
 * ROUTINE NAME: ROUTECFG_OM_SetDhcpDefaultGateway
 *
 * FUNCTION: Set the DHCP configured default gateway  
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_SetDhcpDefaultGateway(L_INET_AddrIp_T *gateway);

/*
 * ROUTINE NAME: ROUTECFG_OM_GetDefaultGatewayCompatible
 *
 * FUNCTION: Get the user configured default gateway for L2 product
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_GetDefaultGatewayCompatible(L_INET_AddrIp_T *gateway);

/*
 * ROUTINE NAME: ROUTECFG_OM_GetDhcpDefaultGateway
 *
 * FUNCTION:Get the DHCP configured default gateway 
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_GetDhcpDefaultGateway(L_INET_AddrIp_T *gateway);

/*
 * ROUTINE NAME: ROUTECFG_OM_DeleteDefaultGatewayCompatible
 *
 * FUNCTION: Delete user configured gateway for L2 product
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_DeleteDefaultGatewayCompatible();
BOOL_T  ROUTECFG_OM_DeleteIpv6DefaultGatewayCompatible();
/*
 * ROUTINE NAME: ROUTECFG_OM_DeleteDhcpDefaultGateway
 *
 * FUNCTION: Delete the DHCP configured default gateway
 *
 * INPUT: gateway
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T  ROUTECFG_OM_DeleteDhcpDefaultGateway();
BOOL_T  ROUTECFG_OM_DeleteIpv6DhcpDefaultGateway();

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_GetRunningEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get running ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_ROUTE_GetRunningEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_GetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_OM_ROUTE_GetEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
void NETCFG_OM_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx);

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#endif /* NETCFG_OM_ROUTE_H */

