/* MODULE NAME:  nsm_pmgr.h
 * PURPOSE:
 *     NSM PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    25/06/2007 - Charlie Chen, Created
 *    15/01/2008 - Vai Wang
 *    13/11/2008 - kh_shi
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef NSM_PMGR_H
#define NSM_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "nsm_mgr.h"
#include "nsm_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/****************************
 *    Initial functions
 ****************************/

/* FUNCTION NAME : NSM_PMGR_InitiateProcessResource
 * PURPOSE:
 *      Initiate process resources for NSM_PMGR.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE   -- Success
 *      FALSE  -- Fail
 *
 * NOTES:
 *      None.
 */
BOOL_T NSM_PMGR_InitiateProcessResource(void);


/********************************* 
 *  Interface related functions
 *********************************/

/* FUNCTION NAME : NSM_PMGR_CreateInterface
 * PURPOSE:
 *      Add a interface to NSM.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *      mac      --  MAC of interface
 *      flags    --  Flags of interface (IFF_UP/IFF_RUNNING)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_CreateInterface(UI32_T ifindex, UI8_T *mac, UI16_T flags);

/* FUNCTION NAME : NSM_PMGR_CreateLoopbackInterface
 * PURPOSE:
 *      Add a loopback interface to NSM.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *      mac      --  MAC of interface
 *      flags    --  Flags of interface (IFF_UP/IFF_RUNNING)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_CreateLoopbackInterface(UI32_T ifindex, UI8_T *mac, UI16_T flags);

/* FUNCTION NAME : NSM_PMGR_DeleteInterface
 * PURPOSE:
 *      Delete a interface from NSM.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_DeleteInterface(UI32_T ifindex);

/* FUNCTION NAME : NSM_PMGR_DeleteLoopbackInterface
 * PURPOSE:
 *      Delete a loopback interface from NSM.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_DeleteLoopbackInterface(UI32_T ifindex);

/* FUNCTION NAME : NSM_PMGR_GetNextInterface
 * PURPOSE:
 *      Find next interface in NSM.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *
 * OUTPUT:
 *      ifindex  --  Index of next interface
 *      flags    --  Flags of interface (IFF_UP/IFF_RUNNING)
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_GetNextInterface(UI32_T *ifindex, UI16_T *flags);


/* FUNCTION NAME : NSM_PMGR_SetIfFlags
 * PURPOSE:
 *      Set flags of a interface.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_SetIfFlags(UI32_T ifindex, UI16_T flags);


/* FUNCTION NAME : NSM_PMGR_UnsetIfFlags
 * PURPOSE:
 *      Unset flags of a interface.
 *
 * INPUT:
 *      ifindex  --  Index of interface
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to find vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR    --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_UnsetIfFlags(UI32_T ifindex, UI16_T flags);


/* FUNCTION NAME : NSM_PMGR_AddIPv4Rif
 * PURPOSE:
 *      Add a IPV4 Rif (IPIF) to a specified device (VLAN).
 *
 * INPUT:
 *      ifindex    --  The ifindex to bind IP address, currently vid_ifindex is used.
 *      ip_addr    --  The IP address to bind
 *      mask       --  The subnet mask to bind
 *      primary_interface  -- specify to add primary or secondary address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_ADDRESS_OVERLAPPED      -- address overlap
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_CANT_SET_ADDRESS_WITH_ZERO_IFINDEX  -- The specified interface in ZebOS do not have
 *                         ifindex(internal in ZebOS) need to wait.
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_SET_SECONDARY_FIRST            -- can not set secondary withou primary
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_CANT_CHANGE_SECONDARY   -- Can not change primary to secondary
 *      NSM_TYPE_RESULT_SAME_ADDRESS_EXIST      -- The same address already configured
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL         --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_AddIPv4Rif(UI32_T ifindex, UI8_T *ip_addr, UI8_T *mask, UI32_T primary_interface);
UI32_T NSM_PMGR_AddInetRif(UI32_T ifindex, L_INET_AddrIp_T *ip_addr, UI32_T primary_interface);


/* FUNCTION NAME : NSM_PMGR_DelIPv4Rif
 * PURPOSE:
 *      Delete a IPV4 Rif (IPIF) from a specified device (VLAN).
 *
 * INPUT:
 *      entry_p->ifindex    --  The ifindex to bind IP address, currently vid_ifindex is used.
 *      entry_p->ip_addr    --  The IP address to bind
 *      entry_p->mask       --  The subnet mask to bind
 *      entry_p->primary_interface  -- specify to add primary or secondary address
 *                        -- NSM_MGR_IPv4Rif_ACTIVE_MODE_PRIMARY -- primary
 *                        -- NSM_MGR_IPv4Rif_ACTIVE_MODE_SECONDARY -- secondary 
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_DelIPv4Rif(UI32_T ifindex, UI8_T *ip_addr, UI8_T *mask, UI32_T primary_interface);
UI32_T NSM_PMGR_DelInetRif(UI32_T ifindex, L_INET_AddrIp_T *ip_addr, UI32_T primary_interface);


/* FUNCTION NAME : NSM_PMGR_GetNextIPv4Rif
 * PURPOSE:
 *      Retrieve the next IPv4 RIF entry from nsm.
 *
 * INPUT:
 *      ifindex
 *      ip_addr
 *      mask
 *      primary_interface
 *
 * OUTPUT:
 *      ifindex
 *      ip_addr
 *      mask
 *      primary_interface
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_GetNextIPv4Rif(UI32_T *ifindex,UI8_T *ip_addr,UI8_T *mask,UI32_T *primary_infterface);
UI32_T NSM_PMGR_GetNextInetRif(UI32_T *ifindex, L_INET_AddrIp_T *ip_addr, UI32_T *primary_infterface);


/* FUNCTION NAME : NSM_PMGR_AddIPv6Rif
 * PURPOSE:
 *      Add a IPV6 Rif (IPIF) to a specified device (VLAN).
 *
 * INPUT:
 *      ifindex    --  The ifindex to bind IP address, currently vid_ifindex is used.
 *      ip_addr    --  The IP address to bind
 *      mask       --  The subnet mask to bind
 *      primary_interface  -- specify to add primary or secondary address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_ADDRESS_OVERLAPPED      -- address overlap
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_CANT_SET_ADDRESS_WITH_ZERO_IFINDEX  -- The specified interface in ZebOS do not have
 *                         ifindex(internal in ZebOS) need to wait.
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_SET_SECONDARY_FIRST            -- can not set secondary withou primary
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_CANT_CHANGE_SECONDARY   -- Can not change primary to secondary
 *      NSM_TYPE_RESULT_SAME_ADDRESS_EXIST      -- The same address already configured
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL         --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_AddIPv6Rif(UI32_T ifindex, L_INET_AddrIp_T *ip_addr);

/* FUNCTION NAME : NSM_PMGR_DelIPv6Rif
 * PURPOSE:
 *      Delete a IPV6 Rif (IPIF) from a specified device (VLAN).
 *
 * INPUT:
 *      entry_p->ifindex    --  The ifindex to bind IP address, currently vid_ifindex is used.
 *      entry_p->ip_addr    --  The IP address to bind
 *      entry_p->mask       --  The subnet mask to bind
 *      entry_p->primary_interface  -- specify to add primary or secondary address
 *                        -- NSM_MGR_IPv4Rif_ACTIVE_MODE_PRIMARY -- primary
 *                        -- NSM_MGR_IPv4Rif_ACTIVE_MODE_SECONDARY -- secondary 
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_DelIPv6Rif(UI32_T ifindex, L_INET_AddrIp_T *ip_addr);


/* FUNCTION NAME : NSM_PMGR_GetNextIPv6Rif
 * PURPOSE:
 *      Retrieve the next IPv6 RIF entry from nsm.
 *
 * INPUT:
 *      ifindex
 *      ip_addr
 *
 * OUTPUT:
 *      ifindex
 *      ip_addr
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 *
 * NOTES:
 *      None
 */
//UI32_T NSM_PMGR_GetNextIPv6Rif(UI32_T *ifindex, L_INET_AddrIp_T *ip_addr);


/* FUNCTION NAME : NSM_PMGR_CreateTunnel
 * PURPOSE:
 *      Create a tunnel interface in TCP/IP stack
 * INPUT:
 *      ifindex       --  tunnel interface index
 *      mode          --  tunnel mode
 *      local_ip      --  local ip address in L_INET_AddrIp_T format
 *      remote_ip  --   remote ip address in L_INET_AddrIp_T format
 *      ttl               --   TTL value for this interface
 * OUTPUT:
 *     None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES: 
 *      1. tunnel mode:  IPAL_TUNNEL_MODE_ISATAP or
 *                                 IPAL_TUNNEL_MODE_SIT
 *      2. For 6to4(SIT), if local_ip or remote_ip is NULL, means that address is “any?
 *      3. For ISATAP, only local address have to set
 */
UI32_T NSM_PMGR_CreateTunnel(UI32_T ifindex, UI32_T mode, L_INET_AddrIp_T *local_ip, 
                             L_INET_AddrIp_T *remote_ip, UI32_T ttl);


/* FUNCTION NAME : NSM_PMGR_DeleteTunnel
 * PURPOSE:
 *      Delete a tunnel interface in TCP/IP stack
 * INPUT:
 *      ifindex       --  tunnel interface index
 * OUTPUT:
 *     None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES: 
 */
UI32_T NSM_PMGR_DeleteTunnel(UI32_T ifindex);


/********************************* 
 *  Routing related functions
 *********************************/

/* FUNCTION NAME : NSM_PMGR_EnableIpForwarding
 * PURPOSE:
 *      Enable IP forwarding function of specific virtual router and IP address Type.
 *
 * INPUT:
 *      ifs  --  Index(vr_id, addr_type) of ip forwarding function to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to look up vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR  --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/* FUNCTION NAME : NSM_PMGR_DisableIpForwarding
 * PURPOSE:
 *      Disable IP forwarding function of specific virtual router and IP address Type.
 *
 * INPUT:
 *      ifs  --  Index(vr_id, addr_type) of ip forwarding function to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to look up vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR  --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/* FUNCTION NAME : NSM_PMGR_AddStaticIpCidrRoute
 * PURPOSE:
 *      Add a static route to rib of NSM.
 *
 * INPUT:
 *      ip_route_dest  --  route destination ip address
 *      ip_route_mask  --  route destination mask
 *      ip_route_next_hop --  next hop ip address
 *      ip_route_distance --  Administrative distance of the route entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL --  Fail to look up vr in nsm
 *      NSM_TYPE_RESULT_VRF_NOT_EXIST  --  Vrf in nsm not exists
 *      NSM_TYPE_RESULT_UNKNOWN_ERR  --  Unknown error
 *
 * NOTES:
 *      None.
 */
//UI32_T NSM_PMGR_AddStaticIpCidrRoute(UI8_T ip_route_dest[4], UI8_T ip_route_mask[4], UI8_T ip_route_next_hop[4], UI32_T ip_route_distance);
UI32_T NSM_PMGR_AddStaticIpCidrRoute(L_INET_AddrIp_T* ip_route_dest, L_INET_AddrIp_T *ip_route_next_hop,  UI32_T ip_route_if_index,UI32_T ip_route_distance);

/* FUNCTION NAME : NSM_PMGR_DeleteStaticIpCidrRoute
 * PURPOSE:
 *      Delete a static route from rib of NSM.
 *
 * INPUT:
 *      ip_route_dest  --  route destination ip address
 *      ip_route_mask  --  route destination mask
 *      ip_route_next_hop --  next hop ip address
 *      ip_route_distance --  administrative distance of the route entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL  --  Fail to look up vr in nsm
 *      NSM_TYPE_RESULT_ENTRY_NOT_EXIST  --  The specified route entry doesn't exist
 *
 * NOTES:
 *      None.
 */
//UI32_T NSM_PMGR_DeleteStaticIpCidrRoute(UI8_T ip_route_dest[4], UI8_T ip_route_mask[4], UI8_T ip_route_next_hop[4], UI32_T ip_route_distance);
UI32_T NSM_PMGR_DeleteStaticIpCidrRoute(L_INET_AddrIp_T *ip_route_dest, L_INET_AddrIp_T *ip_route_next_hop, UI32_T ip_route_if_index,UI32_T ip_route_distance);

/* FUNCTION NAME : NSM_PMGR_DelAllStaticIpv4CidrRoute
 * PURPOSE:
 *      Delete all static route entries from rib of nsm.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE   -- Success
 *      FALSE  -- Fail
 *
 * NOTES:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL  --  Fail to look up vr in nsm
 */
UI32_T NSM_PMGR_DelAllStaticIpv4CidrRoute(void);
//UI32_T NSM_PMGR_DelAllStaticInetCidrRoute(void);

/* FUNCTION NAME : NSM_PMGR_DelAllStaticIpv6CidrRoute
 * PURPOSE:
 *      Delete all static route entries from rib of nsm.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE   -- Success
 *      FALSE  -- Fail
 *
 * NOTES:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL  --  Fail to look up vr in nsm
 */
UI32_T NSM_PMGR_DelAllStaticIpv6CidrRoute(void);

/* FUNCTION NAME : NSM_PMGR_GetRouteNumber
 * PURPOSE:
 *      Get the number of route entry of the specified route type.
 *
 * INPUT:
 *      route_type  --  the valid value can be:
 *                      NSM_MGR_ROUTE_TYPE_IPV4_LOCAL:  IPv4 local route
 *                      NSM_MGR_ROUTE_TYPE_IPV4_STATIC: IPv4 static route
 *                      NSM_MGR_ROUTE_TYPE_IPV4_DYNAMIC:IPv4 dynamic route
 *                      NSM_MGR_ROUTE_TYPE_IPV4_ALL:    IPv4 route of all type listed above
 *                      NSM_MGR_ROUTE_TYPE_IPV6_LOCAL:  IPv6 local route
 *                      NSM_MGR_ROUTE_TYPE_IPV6_STATIC: IPv6 static route
 *                      NSM_MGR_ROUTE_TYPE_IPV6_DYNAMIC:IPv6 dynamic route
 *                      NSM_MGR_ROUTE_TYPE_IPV6_ALL:    IPv6 route of all type listed above
 *                      NSM_MGR_ROUTE_TYPE_ALL:         All route
 *
 * OUTPUT:
 *      route_num   --  The number of route entry of the specified route type
 *      fib_num     --  The number of fib route entries
 *      multipath_num-- The number of routes per ECMP
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK   -- Success
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL  --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_UNKNOWN_ERR  --  Unknown error
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_GetRouteNumber(NSM_MGR_RouteType_T route_type, UI32_T *route_num, UI32_T *fib_num, UI8_T *multipath_num);


/* FUNCTION NAME : NSM_PMGR_GetNextIpv4Route
 * PURPOSE:
 *      Retrieve the next route entry from rib of nsm.
 *
 * INPUT:
 *      entry_p->data.next_route_node  --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_rib_node    --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_hop_node    --  Set as NULL to retrieve the first next hop node.
 *
 * OUTPUT:
 *      entry_p->data.next_route_node  --  The value used to get the route entry next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the route entry
 *                                         next to the previous retrieved entry.
 *      entry_p->data.next_rib_node    --  The value used to get the rib node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the rib node
 *                                         next to the previous retrieved one.
 *      entry_p->data.next_hop_node    --  The value used to get the next hop node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the next hop node
 *                                         next to the previous retrieved next hop node.
 *      entry_p->data.distance         --  distance of the retrieved route entry
 *      entry_p->data.ip_route_dest_prefix_len  --  the prefix length of the route entry
 *      entry_p->data.num_of_next_hop  --  the number of element in array entry_p->ip_next_hop
 *      entry_p->data.ip_route_dest    --  the ip route address of the route entry
 *      entry_p->data.metric           --  metric of the retrieved route entry
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK  --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_EOF --  No more route to retrieve. It MIGHT contain
 *                              some route entries with this return value.
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *
 * NOTES:
 *      1. Only support to show next hop type == NEXTHOP_TYPE_IPV4
 *         NEXTHOP_TYPE_IPV4_IFINDEX, NEXTHOP_TYPE_IPV4_IFNAME
 */
#define NSM_PMGR_GetNextRoute NSM_PMGR_GetNextIpv4Route
UI32_T NSM_PMGR_GetNextIpv4Route(NSM_MGR_GetNextRouteEntry_T* entry_p);
UI32_T NSM_PMGR_GetNextInetRoute(NSM_MGR_IPC_GetNextInetRouteResp_T* resp);


/* FUNCTION NAME : NSM_PMGR_GetNextIpv6Route
 * PURPOSE:
 *      Retrieve the next route entry from rib of nsm.
 *
 * INPUT:
 *      entry_p->data.next_route_node  --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_rib_node    --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_hop_node    --  Set as NULL to retrieve the first next hop node.
 *
 * OUTPUT:
 *      entry_p->data.next_route_node  --  The value used to get the route entry next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the route entry
 *                                         next to the previous retrieved entry.
 *      entry_p->data.next_rib_node    --  The value used to get the rib node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the rib node
 *                                         next to the previous retrieved one.
 *      entry_p->data.next_hop_node    --  The value used to get the next hop node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the next hop node
 *                                         next to the previous retrieved next hop node.
 *      entry_p->data.distance         --  distance of the retrieved route entry
 *      entry_p->data.ip_route_dest_prefix_len  --  the prefix length of the route entry
 *      entry_p->data.num_of_next_hop  --  the number of element in array entry_p->ip_next_hop
 *      entry_p->data.ip_route_dest    --  the ip route address of the route entry
 *      entry_p->data.metric           --  metric of the retrieved route entry
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK  --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_EOF --  No more route to retrieve. It MIGHT contain
 *                              some route entries with this return value.
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 *
 * NOTES:
 *      1. Only support to show next hop type == NEXTHOP_TYPE_IPV6
 *         NEXTHOP_TYPE_IPV6_IFINDEX, NEXTHOP_TYPE_IPV6_IFNAME
 */
UI32_T NSM_PMGR_GetNextIpv6Route(NSM_MGR_GetNextRouteEntry_T* entry_p);


/* FUNCTION NAME : NSM_PMGR_GetNextNRouteCache
 * PURPOSE:
 *      Retrieve the next N route cache entries from rib of nsm.
 *
 * INPUT:
 *      entry_p->data.next_route_node  --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_rib_node    --  Set as NULL to retrieve the first entry.
 *                                         
 *      entry_p->data.next_hop_node    --  Set as NULL to retrieve the first next hop node.
 *
 *                              num_p  --  number of request entries
 * OUTPUT:
 *      entry_p->data.next_route_node  --  The value used to get the route entry next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the route entry
 *                                         next to the previous retrieved entry.
 *      entry_p->data.next_rib_node    --  The value used to get the rib node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the rib node
 *                                         next to the previous retrieved one.
 *      entry_p->data.next_hop_node    --  The value used to get the next hop node next
 *                                         to the currently retrieved one.
 *                                         Retain the value to get the next hop node
 *                                         next to the previous retrieved next hop node.
 *      entry_p->data.distance         --  distance of the retrieved route entry
 *      entry_p->data.ip_route_dest_prefix_len  --  the prefix length of the route entry
 *      entry_p->data.num_of_next_hop  --  the number of element in array entry_p->ip_next_hop
 *      entry_p->data.ip_route_dest    --  the ip route address of the route entry
 *      entry_p->data.metric           --  metric of the retrieved route entry
 *                             num_p   --  number of actual get entries
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK  --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_EOF --  No more route to retrieve. It MIGHT contain
 *                              some route entries with this return value.
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 * NOTES:
 *      1. Only support to show next hop type == NEXTHOP_TYPE_IPV4
 *         NEXTHOP_TYPE_IPV4_IFINDEX, NEXTHOP_TYPE_IPV4_IFNAME
 */
UI32_T NSM_PMGR_GetNextNRouteCache(NSM_MGR_GetNextRouteEntry_T* entry_p, UI32_T *num_p);


/* FUNCTION NAME : NSM_PMGR_GetPmtu
 * PURPOSE:  
 *    Get path MTU for a specific destination ip address
 * INPUT:
 *    dest_ip  --  destination ip address for querying path MTU
 * OUTPUT:
 *    pmtu_p   --  result path MTU value
 * RETURN:
 *      NSM_TYPE_RESULT_OK  --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_EOF --  No more route to retrieve. It MIGHT contain
 *                              some route entries with this return value.
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 * NOTES:
 */
UI32_T NSM_PMGR_GetPmtu(L_INET_AddrIp_T *dest_ip, UI32_T *pmtu_p);


/* FUNCTION NAME : NSM_PMGR_SetPmtuExpireTime
 * PURPOSE:  
 *    Set path MTU expire time for route cache entries
 * INPUT:
 *    expire_time  --  path MTU expire time in miliseconds
 * OUTPUT:
 *    None
 * RETURN:
 *      NSM_TYPE_RESULT_OK  --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_EOF --  No more route to retrieve. It MIGHT contain
 *                              some route entries with this return value.
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL --  Fail to look up vrf in nsm
 *      NSM_TYPE_RESULT_INVALID_ARG  --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL  --  Fail to send ipc message
 * NOTES:
 */
UI32_T NSM_PMGR_SetPmtuExpireTime(UI32_T expire_time);


/* FUNCTION NAME : NSM_PMGR_SnmpGetRoute
 * PURPOSE:
 *      Get route entry from nsm for SNMP request.
 *
 * INPUT:
 *      entry_p
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_ENTRY_NOT_EXIST
 *
 * NOTES:
 */
UI32_T NSM_PMGR_SnmpGetRoute(NSM_TYPE_IpCidrRouteEntry_T* entry_p);

/* FUNCTION NAME : NSM_PMGR_SnmpGetNextRoute
 * PURPOSE:
 *      Get Next route entry from nsm for SNMP request.
 *
 * INPUT:
 *      entry_p
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_ENTRY_NOT_EXIST
 *
 * NOTES:
 */
UI32_T NSM_PMGR_SnmpGetNextRoute(NSM_TYPE_IpCidrRouteEntry_T* entry_p);

/* FUNCTION NAME : NSM_PMGR_SnmpSetRoute
 * PURPOSE:
 *      Set route entry to nsm for SNMP request.
 *
 * INPUT:
 *      entry_p
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_INVALID_ARG
 *      NSM_TYPE_RESULT_FAIL
 *
 * NOTES:
 */
UI32_T NSM_PMGR_SnmpSetRoute(NSM_TYPE_IpCidrRouteEntry_T* entry_p);


/* FUNCTION NAME : NSM_PMGR_FindBestRoute
 * PURPOSE:
 *      Find the best routes for the specified destionation IP address.
 * INPUT:
 *      dest_addr   -- destionation IP address to be checked.
 *      count       -- maximal number of nexthop accepted (i.e. prepared array size)
 *
 * OUTPUT:
 *      nexthop -- array of nexthop address
 *      ifindex -- array of nexthop ifindex
 *      count   -- number of output nexthop
 *      owner   -- route type
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK              
 *      NSM_TYPE_RESULT_INVALID_ARG
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_VRF_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_ENTRY_NOT_EXIST
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL   
 *
 * NOTES:
 *      1. We assume that vr_id as 0, vrf_id as 0.
 */
UI32_T NSM_PMGR_FindBestRoute(L_INET_AddrIp_T *dest_ip, L_INET_AddrIp_T *nexthop, UI32_T *ifindex, UI32_T *count, UI32_T *owner);


/* FUNCTION NAME : NSM_PMGR_Enable_IPv4Forwarding
 * PURPOSE:
 *      Enable IPv4 forwarding in TCP/IP stack
 * INPUT:
 *   None
 * OUTPUT:
 *   None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_Enable_IPv4Forwarding();


/* FUNCTION NAME : NSM_PMGR_Disable_IPv4Forwarding
 * PURPOSE:
 *      Disable IPv4 forwarding in TCP/IP stack
 * INPUT:
 *   None
 * OUTPUT:
 *   None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_Disable_IPv4Forwarding();


/* FUNCTION NAME : NSM_PMGR_GetIPv4Forwarding
 * PURPOSE:
 *      Get IPv4 forwarding status from TCP/IP stack
 * INPUT:
 *    None
 * OUTPUT:
 *     status_p  --  IPv4 forwarding status
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_GetIPv4Forwarding(BOOL_T *status_p);


/* FUNCTION NAME : NSM_PMGR_Enable_IPv6Forwarding
 * PURPOSE:
 *      Enable IPv6 forwarding in TCP/IP stack
 * INPUT:
 *   None
 * OUTPUT:
 *   None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_Enable_IPv6Forwarding();


/* FUNCTION NAME : NSM_PMGR_Disable_IPv6Forwarding
 * PURPOSE:
 *      Disable IPv6 forwarding in TCP/IP stack
 * INPUT:
 *   None
 * OUTPUT:
 *   None
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_Disable_IPv6Forwarding();


/* FUNCTION NAME : NSM_PMGR_GetIPv6Forwarding
 * PURPOSE:
 *      Get IPv6 forwarding status from TCP/IP stack
 * INPUT:
 *    None
 * OUTPUT:
 *     status_p  --  IPv6 forwarding status
 * RETURN:
 *      NSM_TYPE_RESULT_OK                      --  There exists other route to retrieve.
 *      NSM_TYPE_RESULT_IF_NOT_EXIST            -- the specified ifindex do not exist
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST        -- Can not find Master structure in ZebOS
 *      NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY     -- Can not change secondary to primary
 *      NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST   -- Must delete secondary address first
 *      NSM_TYPE_RESULT_ADDRESS_NOT_EXIST       -- The address do not exist
 *      NSM_TYPE_RESULT_UNKNOWN_ERR             -- Error without specified reason
 *      NSM_TYPE_RESULT_INVALID_ARG             --  Invalid argument
 *      NSM_TYPE_RESULT_SEND_MSG_FAIL           --  Fail to send ipc message
 * NOTES:
 */
//UI32_T NSM_PMGR_GetIPv6Forwarding(BOOL_T *status_p);

/* FUNCTION NAME : NSM_PMGR_SetMultipathNumber
 * PURPOSE:
 *      Set multipath numbers installed to FIB.
 *
 * INPUT:
 *      multipaths
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_SetMultipathNumber(UI8_T multipaths);


/* FUNCTION NAME : NSM_PMGR_GetMultipathNumber
 * PURPOSE:
 *      Get multipath numbers installed to FIB.
 *
 * INPUT:
 *      ipcmsg_p
 *
 * OUTPUT:
 *      ipcmsg_p->data.multipath_num
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_VR_LOOKUP_FAIL
 *      NSM_TYPE_RESULT_MASTER_NOT_EXIST
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_GetMultipathNumber(UI8_T *multipaths);

BOOL_T NSM_PMGR_PrefetchIpv4Route(NSM_MGR_RouteType_T type, BOOL_T show_database);
BOOL_T NSM_PMGR_PostfetchIpv4Route(void);
UI32_T NSM_PMGR_CLIGetNextNIpv4Route(UI32_T start_line, UI8_T *buffer, UI32_T max_fetch_lines, UI32_T *fetched_lines_p);


//add by simon: nsm_rtadv

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRa
 *-----------------------------------------------------------------------------
 * PURPOSE : enable/disable  the sending of IPv6 router advertisement transmissions
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_RA: when TRUE, enable RA transmission; when FALSE, suppress RA transmission
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRa(UI32_T vid_ifindex, BOOL_T enable_RA);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between router advertisement (RA) transmissions
 *
 * INPUT   : vid_ifindex: vlan ifindex
 *           max: max interval in seconds
 *           min: min interval in seconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NSM_TYPE_RESULT_OK
 *           NSM_TYPE_RESULT_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaInterval(UI32_T vid_ifindex, UI32_T max, UI32_T min);
UI32_T NSM_PMGR_UnsetRaInterval(UI32_T vid_ifindex);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the router lifetime value in IPv6 router advertisements on an interface
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T seconds: lifetime in second
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaLifetime(UI32_T vid_ifindex, UI32_T seconds);
UI32_T NSM_PMGR_UnsetRaLifetime(UI32_T vid_ifindex);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 neighbor solicitation retransmissions on an interface
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T msec: transmissions interval in msec
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetNsInterval(UI32_T vid_ifindex, UI32_T msec);
UI32_T NSM_PMGR_UnsetNsInterval(UI32_T vid_ifindex);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the amount of time that a remote IPv6 node is considered reachable
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T milliseconds: reachable time in millisecond
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaReachableTime(UI32_T vid_ifindex, UI32_T milliseconds);
UI32_T NSM_PMGR_UnsetRaReachableTime(UI32_T vid_ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_managed: when TRUE enable M flag, when FALSE disable M flag
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaManagedConfigFlag(UI32_T vid_ifindex, BOOL_T enable_managed);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "other stateful configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_other: when TRUE enable O flag, when FALSE disable O flag
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaOtherConfigFlag(UI32_T vid_ifindex, BOOL_T enable_other);



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :  configure which IPv6 prefixes are included in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  L_INET_AddrIp_T* addr: IPV6 prefix that are included in RA. addr.addr and addr.prefix_length must not empty
 *                  UI32_T vlifetime : valid lifetime in second
 *                  UI32_T plifetime: preferred lifetime in second
 *                  BOOL_T enable_on_link: when TRUE set to on-link, when FALSE set to off-link
 *                  BOOL_T enable_autoconf: when TRUE set to autoconfig, when FALSE set to no-autoconfig
 *  
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_SetRaPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* addr, UI32_T vlifetime, UI32_T plifetime, BOOL_T enable_on_link, BOOL_T enable_autoconf);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRUnsetRaPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :  configure which IPv6 prefixes are included in IPv6 router advertisements as default
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  L_INET_AddrIp_T* addr: IPV6 prefix that are included in RA. addr.addr and addr.prefix_length must not empty
 *  
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NSM_PMGR_UnsetRaPrefix(UI32_T vid_ifindex,L_INET_AddrIp_T* addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaRouterPreference
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                 preference: high = 0x01, medium=0x00, low=0x03
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NSM_PMGR_SetRaRouterPreference(UI32_T vid_ifindex, UI32_T preference);

UI32_T NSM_PMGR_SetDefaultRaHopLimit(UI32_T value);
UI32_T NSM_PMGR_SetRaHopLimit(UI32_T vid_ifindex, UI32_T value);
UI32_T NSM_PMGR_UnsetRaHopLimit(UI32_T vid_ifindex);
UI32_T NSM_PMGR_SetRaMtu6(UI32_T vid_ifindex, UI32_T value);
UI32_T NSM_PMGR_UnsetRaMtu6(UI32_T vid_ifindex);

/*
    for backdoor usage:
*/
void NSM_PMGR_BackdoorSetDebugFlag(UI32_T flag);

/* FUNCTION NAME : NSM_PMGR_ReadNetlink
 * PURPOSE:
 *      Force NSM to read netlink message from kernel.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NSM_TYPE_RESULT_OK
 *      NSM_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NSM_PMGR_ReadNetlink(void);

#if (SYS_CPNT_IP_TUNNEL == TRUE)

UI32_T NSM_PMGR_CreateTunnelInterface (UI32_T tid_ifindex);
BOOL_T NSM_PMGR_IsTunnelInterfaceExist(UI32_T ifindex);
UI32_T NSM_PMGR_DeleteTunnelInterface (UI32_T tid_ifindex);
UI32_T NSM_PMGR_SetTunnelSource(UI32_T tid_ifindex, L_INET_AddrIp_T *ip_addr);
UI32_T NSM_PMGR_UnsetTunnelSource(UI32_T tid_ifindex);
#if 0
UI32_T NSM_PMGR_SetTunnelLocalIpv6(UI32_T tid_ifindex, L_INET_AddrIp_T *ip_addr);
UI32_T NSM_PMGR_UnsetTunnelLocalIpv6(UI32_T tid_ifindex);
#endif
UI32_T NSM_PMGR_SetTunnelDestination(UI32_T tid_ifindex, L_INET_AddrIp_T *ip_addr);
UI32_T NSM_PMGR_UnsetTunnelDestination(UI32_T tid_ifindex);

UI32_T NSM_PMGR_SetTunnelMode(UI32_T tid_ifindex, char*  mode_str);
UI32_T NSM_PMGR_UnsetTunnelMode(UI32_T tid_ifindex);

UI32_T NSM_PMGR_SetTunnelTtl(UI32_T tid_ifindex, UI32_T ttl);
UI32_T NSM_PMGR_UnsetTunnelTtl(UI32_T tid_ifindex);

UI32_T NSM_PMGR_AddStaticIpTunnelRoute(L_INET_AddrIp_T* ip_route_dest, UI32_T tid_ifindex, UI32_T ip_route_distance);
UI32_T NSM_PMGR_DeleteStaticIpTunnelRoute(L_INET_AddrIp_T *ip_route_dest, UI32_T tid_ifindex, UI32_T ip_route_distance);

#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VRRP == TRUE)
/* FUNCTION NAME : NSM_PMGR_AddVrrpVirtualIp
 * PURPOSE:
 *     Add VRRP virtual IP address
 * INPUT:
 *     vip  --  virtual ip address information
 * OUTPUT:
 *     None
 * RETURN:
 * NOTES:
 */
UI32_T NSM_PMGR_AddVrrpVirtualIp(L_INET_AddrIp_T *vip);

/* FUNCTION NAME : NSM_PMGR_DelVrrpVirtualIp
 * PURPOSE:
 *     Delete VRRP virtual IP address
 * INPUT:
 *     vip  --  virtual ip address information
 * OUTPUT:
 *     None
 * RETURN:
 * NOTES:
 */
UI32_T NSM_PMGR_DelVrrpVirtualIp(L_INET_AddrIp_T *vip);
#endif

#endif    /* End of NSM_PMGR_H */

