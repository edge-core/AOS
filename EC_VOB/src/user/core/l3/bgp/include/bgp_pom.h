/* MODULE NAME:  bgp_pom.h
 * PURPOSE:
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    Date          -- Modifier,    Reason
 *    03/01/2011    -- Peter Yu,    Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */
#ifndef _BGP_POM_H
#define _BGP_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_inet.h"
#include "bgp_type.h"
#include "bgp_om.h"

/* NAME CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
 
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
BOOL_T BGP_POM_InitiateProcessResource(void);
//SYS_TYPE_Get_Running_Cfg_T BGP_POM_GetRunningDataByField(BGP_TYPE_Running_Data_T *data);
UI32_T BGP_POM_GetConfigDataByField(BGP_TYPE_Config_Data_T *data_p);

BOOL_T BGP_POM_GetNextRunCfgInstance(BGP_OM_RunCfgInstance_T * runcfg_instance_p);
BOOL_T BGP_POM_GetNextRunCfgConfederationPeer(UI32_T as_number, UI32_T *confederation_peer_p);
BOOL_T BGP_POM_GetNextRunCfgAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, BGP_OM_AggregateAddr_T *aggr_addr_p);
BOOL_T BGP_POM_GetNextRunCfgDistance(UI32_T as_number, BGP_OM_Distance_T *distance_p);
BOOL_T BGP_POM_GetNextRuncfgNeighbor(UI32_T as_number, BGP_OM_AfiSafiNeighbor_T *neighbor_p);
BOOL_T BGP_POM_GetNextRuncfgNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, BGP_OM_Network_T *network_p);
BOOL_T BGP_POM_GetRuncfgPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], BGP_OM_AfiSafiNeighbor_T *config_p);
BOOL_T BGP_POM_GetNextRuncfgPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], BGP_OM_AfiSafiNeighbor_T *config_p);
BOOL_T BGP_POM_GetNextRuncfgPeerGroupMember(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *ipaddr_p);

#endif /* _BGP_POM_H */
