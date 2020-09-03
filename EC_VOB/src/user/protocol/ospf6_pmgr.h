/* MODULE NAME:  ospf_pmgr.h
 * PURPOSE:
 *     OSPF PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *       7/13/2009          Steven.Gao      Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef OSPF6_PMGR_H
#define OSPF6_PMGR_H

#include "sys_type.h"

#include "ospf6_type.h"

#include "ospf6_mgr.h"

/* FUNCTION NAME : OSPF6_PMGR_InitiateProcessResource
* PURPOSE:
*      Initiate process resources for OSPF6_PMGR.
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
BOOL_T OSPF6_PMGR_InitiateProcessResource(void);


UI32_T OSPF6_PMGR_SignalL3IfCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags);
UI32_T OSPF6_PMGR_SignalL3IfDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

UI32_T OSPF6_PMGR_SignalL3IfRifCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, UI32_T primary);
UI32_T OSPF6_PMGR_SignalL3IfRifDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);

UI32_T OSPF6_PMGR_SignalL3IfUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);
UI32_T OSPF6_PMGR_SignalL3IfDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);


UI32_T OSPF6_PMGR_IfParamCreate(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id);
UI32_T OSPF6_PMGR_IfParamDestroy(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id);

UI32_T OSPF6_PMGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T priority);
UI32_T OSPF6_PMGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T cost);
UI32_T OSPF6_PMGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_PMGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_PMGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval);
UI32_T OSPF6_PMGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T delay);
UI32_T OSPF6_PMGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag);

UI32_T OSPF6_PMGR_IfRouterSet(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T format, UI32_T id);
UI32_T OSPF6_PMGR_IfRouterUnset(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T id, UI32_T check_flag);


UI32_T OSPF6_PMGR_RouterOspfSet(UI32_T vr_id , char * tag);
UI32_T OSPF6_PMGR_RouterOspfUnset(UI32_T vr_id, char * tag);



UI32_T OSPF6_PMGR_ABRTypeSet(UI32_T vr_id, char * tag, UI32_T type);
UI32_T OSPF6_PMGR_ABRTypeUnset(UI32_T vr_id, char * tag);


UI32_T OSPF6_PMGR_AreaDefaultCostSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T cost);
UI32_T OSPF6_PMGR_AreaDefaultCostUnset(UI32_T vr_id, char * tag, UI32_T area_id);

UI32_T OSPF6_PMGR_AreaRangeSet(OSPF6_TYPE_Area_Range_T * range) ;
UI32_T OSPF6_PMGR_AreaRangeUnset(OSPF6_TYPE_Area_Range_T * range) ;


UI32_T OSPF6_PMGR_AreaStubSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_PMGR_AreaStubUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_PMGR_AreaStubNoSummarySet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);
UI32_T OSPF6_PMGR_AreaStubNoSummaryUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format);


UI32_T OSPF6_PMGR_AreaVirtualLinkSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T peer, UI32_T type, UI32_T value);
UI32_T OSPF6_PMGR_AreaVirtualLinkUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T peer, UI32_T type);


UI32_T OSPF6_PMGR_DefaultMetricSet(UI32_T vr_id, char * tag, UI32_T metric);
UI32_T OSPF6_PMGR_DefaultMetricUnset(UI32_T vr_id, char * tag);


UI32_T OSPF6_PMGR_PassiveIfSet(UI32_T vr_id, char * tag, UI32_T ifindex);
UI32_T OSPF6_PMGR_PassiveIfUnset(UI32_T vr_id, char * tag, UI32_T ifindex);


UI32_T OSPF6_PMGR_PassiveIfGet(UI32_T vr_id, char * tag, UI32_T ifindex);
UI32_T OSPF6_PMGR_PassiveIfGetNext(UI32_T vr_id, char * tag, UI32_T * ifindex);


UI32_T OSPF6_PMGR_ConcurrentDDSet(UI32_T vr_id, char * tag, UI32_T number);
UI32_T OSPF6_PMGR_ConcurrentDDUnset(UI32_T vr_id, char * tag);



UI32_T OSPF6_PMGR_RedistributeEntry_Get(OSPF6_TYPE_Multi_Proc_Redist_T * data);
UI32_T OSPF6_PMGR_RedistributeEntry_GetNext(OSPF6_TYPE_Multi_Proc_Redist_T *data);

UI32_T OSPF6_PMGR_RedistributeProtoSet(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_PMGR_RedistributeProtoUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_PMGR_RedistributeMetricTypeSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric_type);
UI32_T OSPF6_PMGR_RedistributeMetricTypeUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_PMGR_RedistributeMetricSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric);
UI32_T OSPF6_PMGR_RedistributeMetricUnset(UI32_T vr_id, char * tag, UI32_T proto);
UI32_T OSPF6_PMGR_RedistributeRoutemapSet(UI32_T vr_id, char * tag, UI32_T proto, char *route_map);
UI32_T OSPF6_PMGR_RedistributeRoutemapUnset(UI32_T vr_id, char * tag, UI32_T proto);


UI32_T OSPF6_PMGR_RouterIdSet(UI32_T vr_id, char * tag, UI32_T router_id);
UI32_T OSPF6_PMGR_RouterIdUnset(UI32_T vr_id, char * tag);

UI32_T OSPF6_PMGR_DelayTimerSet(UI32_T vr_id, char * tag, UI32_T delay);
UI32_T OSPF6_PMGR_HoldTimerSet(UI32_T vr_id, char * tag, UI32_T hold);
UI32_T OSPF6_PMGR_TimerUnset(UI32_T vr_id, char * tag);


UI32_T OSPF6_PMGR_Process_Get(OSPF6_MGR_OSPF_ENTRY_T *ospf_entry_p);
UI32_T OSPF6_PMGR_Process_GetNext(OSPF6_MGR_OSPF_ENTRY_T *ospf_entry_p);

UI32_T OSPF6_PMGR_Area_Get(OSPF6_TYPE_Area_T *entry);
UI32_T OSPF6_PMGR_Area_GetNext(OSPF6_TYPE_Area_T *entry);


UI32_T OSPF6_PMGR_AreaRange_Get(OSPF6_TYPE_Area_Range_T *entry);
UI32_T OSPF6_PMGR_AreaRange_GetNext(OSPF6_TYPE_Area_Range_T *entry);


UI32_T OSPF6_PMGR_LinkScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);
UI32_T OSPF6_PMGR_AreaScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);
UI32_T OSPF6_PMGR_ASScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p);

UI32_T OSPF6_PMGR_Interface_Get(OSPF6_TYPE_Interface_T *entry);
UI32_T OSPF6_PMGR_Interface_GetNext(OSPF6_TYPE_Interface_T *entry);

UI32_T OSPF6_PMGR_IfParam_Get(OSPF6_TYPE_IfParam_T *entry);
UI32_T OSPF6_PMGR_IfParam_GetNext(OSPF6_TYPE_IfParam_T *entry);


UI32_T OSPF6_PMGR_Neighbor_Get(OSPF6_MGR_NBR_ENTRY_T *nbr_entry_p);
UI32_T OSPF6_PMGR_Neighbor_GetNext(OSPF6_MGR_NBR_ENTRY_T *nbr_entry_p);


UI32_T OSPF6_PMGR_VirtNeighbor_Get(OSPF6_MGR_VNBR_ENTRY_T *nbr_entry_p);
UI32_T OSPF6_PMGR_VirtNeighbor_GetNext(OSPF6_MGR_VNBR_ENTRY_T *nbr_entry_p);


UI32_T OSPF6_PMGR_Route_GetNext(OSPF6_TYPE_Route_T *entry);


UI32_T OSPF6_PMGR_VirtualLink_Get(OSPF6_TYPE_Vlink_T *entry);
UI32_T OSPF6_PMGR_VirtualLink_GetNext(OSPF6_TYPE_Vlink_T *entry);

UI32_T OSPF6_PMGR_ClearOspf6Process(UI32_T vr_id, char * tag); 

#endif

