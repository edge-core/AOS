/* MODULE NAME:  ospf_pmgr.h
 * PURPOSE:
 *     OSPF PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    11/27/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef OSPF_PMGR_H
#define OSPF_PMGR_H

#include "sys_type.h"
#include "ospf_mgr.h"
#include "ospf_type.h"

/* FUNCTION NAME : OSPF_PMGR_InitiateProcessResource
* PURPOSE:
*      Initiate process resources for OSPF_PMGR.
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
BOOL_T OSPF_PMGR_InitiateProcessResource(void);

/* FUNCTION NAME : OSPF_PMGR_RouterOspfSet
* PURPOSE:
*     Set router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_RouterOspfUnset
* PURPOSE:
*     Unset router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_InterfaceAdd
* PURPOSE:
*     Add ospf  interface.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags);

/* FUNCTION NAME : OSPF_PMGR_InterfaceDelete
* PURPOSE:
*     Delete ospf interface.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : OSPF_PMGR_IpAddressAdd
* PURPOSE:
*     Add Ip address.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IpAddressAdd(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);

/* FUNCTION NAME : OSPF_PMGR_IpAddressDelete
* PURPOSE:
*     Delete ospf interface.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IpAddressDelete(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : OSPF_PMGR_InterfaceUp
* PURPOSE:
*     Notify interface up.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       None.
*
* NOTES:
*      None.
*/
void OSPF_PMGR_InterfaceUp(UI32_T vr_id, UI32_T ifindex);

/* FUNCTION NAME : OSPF_PMGR_InterfaceDown
* PURPOSE:
*     Notify interface down.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*      None.
*
* NOTES:
*      None.
*/
void OSPF_PMGR_InterfaceDown(UI32_T vr_id, UI32_T ifindex);

/* FUNCTION NAME :  OSPF_PMGR_IfAuthenticationTypeSet
* PURPOSE:
*     Set OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  OSPF_PMGR_IfAuthenticationTypeUnset
* PURPOSE:
*     Unset OSPF interface authentication type.
*
* INPUT:
*     vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfAuthenticationKeySet
* PURPOSE:
*      Set OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfAuthenticationKeyUnset
* PURPOSE:
*      Unset OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfMessageDigestKeySet
* PURPOSE:
*      Set OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfMessageDigestKeyUnset
* PURPOSE:
*      Unset OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : OSPF_MGR_GetMultiProcSummaryAddrEntry
* PURPOSE:
*     
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcIfEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *entry);


/* FUNCTION NAME : OSPF_PMGR_GetNextMultiProcIfEntry
* PURPOSE:
*     
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcIfEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *entry);

/* FUNCTION NAME :  	OSPF_PMGR_IfMtuSet
* PURPOSE:
*      Set OSPF interface mtu.
*
* INPUT:
*      vr_id,
*      ifindex 
*      mtu
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMtuSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfMtuUnset
* PURPOSE:
*      Unset OSPF interface mtu.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMtuUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfMtuIgnoreSet
* PURPOSE:
*      Set OSPF interface mtu.
*
* INPUT:
*      vr_id,
*      ifindex 
*      mtuIgnore
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMtuIgnoreSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu_ignore, BOOL_T addr_flag, struct pal_in4_addr addr);
/* FUNCTION NAME :  	OSPF_PMGR_IfMtuIgnoreUnset
* PURPOSE:
*      Unset OSPF interface mtu ignore.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfMtuIgnoreUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfPrioritySet
* PURPOSE:
*      Set OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      priority
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfPriorityUnset
* PURPOSE:
*      Unset OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfCostSet
* PURPOSE:
*      Set OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      cost
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfCostUnset
* PURPOSE:
*      Unset OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfDeadIntervalSet
* PURPOSE:
*      Set OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfDeadIntervalUnset
* PURPOSE:
*      Unset OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfHelloIntervalSet
* PURPOSE:
*      Set OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfHelloIntervalUnset
* PURPOSE:
*      Unset OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfRetransmitIntervalSet
* PURPOSE:
*      Set OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfRetransmitIntervalUnset
* PURPOSE:
*      Unset OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfTransmitDelaySet
* PURPOSE:
*      Set OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      delay
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	OSPF_PMGR_IfTransmitDelayUnset
* PURPOSE:
*      Unset OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);


/* FUNCTION NAME :  	OSPF_PMGR_IfParamUnset 
 * PURPOSE: Unset OSPF interface parameter.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      Success/Error
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_PMGR_IfParamUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);


/* FUNCTION NAME : OSPF_PMGR_NetworkSet
* PURPOSE:
*     Set ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_NetworkUnset
* PURPOSE:
*     Delete ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_RouterIdSet
* PURPOSE:
*     Set ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*      router_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id);

/* FUNCTION NAME : OSPF_PMGR_RouterIdUnset
* PURPOSE:
*     Unset ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RouterIdUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_TimerSet
* PURPOSE:
*     Set ospf timer value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      delay,
*      hold.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold);

/* FUNCTION NAME : OSPF_PMGR_TimerUnset
* PURPOSE:
*     Set ospf timer value to default value.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_TimerUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : OSPF_PMGR_DefaultMetricUnset
* PURPOSE:
*     Unset ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_PassiveIfSet(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_PMGR_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_PassiveIfUnset(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_PMGR_CompatibleRfc1853Set
* PURPOSE:
*     Set ospf compatible rfc1853.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_CompatibleRfc1853Set(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_CompatibleRfc1853Unset
* PURPOSE:
*     Unset ospf compatible rfc1853.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_CompatibleRfc1853Unset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_SummaryAddressSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_SummaryAddressSet(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask);


/* FUNCTION NAME : OSPF_PMGR_SummaryAddressUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_SummaryAddressUnset(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask);


/* FUNCTION NAME : OSPF_PMGR_AutoCostSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ref_bandwidth.
*      
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AutoCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T ref_bandwidth);


/* FUNCTION NAME : OSPF_PMGR_AutoCostUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AutoCostUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_GetOspfRedistributeEntry
* PURPOSE:
*     Get ospf redistribute entry.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcRedistEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *entry);
/* FUNCTION NAME : OSPF_PMGR_GetNextOspfRedistributeEntry
* PURPOSE:
*     Getnext ospf redistribute entry.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcRedistEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *entry);
/* FUNCTION NAME : OSPF_PMGR_GetMultiProcSummaryAddrEntry
* PURPOSE:
*     Get ospf summary address
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry);


/* FUNCTION NAME : OSPF_PMGR_GetNextMultiProcSummaryAddrEntry
* PURPOSE:
*     Getnext ospf summary address
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry);

/* FUNCTION NAME : OSPF_PMGR_RedistributeProtoSet
* PURPOSE:
*     Set ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeProtoUnset
* PURPOSE:
*     Unset ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric);


/* FUNCTION NAME : OSPF_PMGR_RedistributeMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeTagSet
* PURPOSE:
*     Set ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      tag
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag);


/* FUNCTION NAME : OSPF_PMGR_RedistributeTagUnset
* PURPOSE:
*     Unset ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : OSPF_PMGR_RedistributeRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map);


/* FUNCTION NAME : OSPF_PMGR_RedistributeRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);
/* FUNCTION NAME : OSPF_PMGR_RedistributeFilterlistnameSet
* PURPOSE:
*     Set ospf redistribute filter list name.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeFilterlistnameSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *list_name);

/* FUNCTION NAME : OSPF_PMGR_RedistributeFilterlistnameUnset
* PURPOSE:
*     Unset ospf redistribute filter list name.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_RedistributeFilterlistnameUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *list_name);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoMetricTypeUnset( UI32_T vr_id, UI32_T proc_id );


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id,UI32_T metric);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id );


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoRoutemapUnset(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoAlwaysSet
* PURPOSE:
*     Set ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoAlwaysUnset
* PURPOSE:
*     Unset ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoSet
* PURPOSE:
*     Set ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : OSPF_PMGR_DefaultInfoUnset
* PURPOSE:
*     Unset ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : OSPF_PMGR_GetNextOspfNeighborEntry
* PURPOSE:
*     Getnext neighbor entry.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextOspfNeighborEntry(OSPF_MGR_NBR_ENTRY_T *nbr_entry_p);

/* FUNCTION NAME : OSPF_PMGR_GetOspfInterfaceEntry
* PURPOSE:
*     Get  ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetOspfInterfaceEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextOspfLsaEntry
* PURPOSE:
*     Getnext lsa entry.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextOspfLsaEntry(OSPF_MGR_LSA_ENTRY_T *lsa_entry_p);

/* FUNCTION NAME : OSPF_PMGR_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_AreaStubUnset
* PURPOSE:
*     Unset ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_AreaStubNoSummarySet
* PURPOSE:
*     Set ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_AreaStubNoSummaryUnset
* PURPOSE:
*     Unset ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost);

/* FUNCTION NAME : OSPF_PMGR_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : OSPF_PMGR_AreaRangeSet
* PURPOSE:
*     Set ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_PMGR_AreaRangeNoAdvertiseSet
* PURPOSE:
*     Set ospf area range no advertise.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_PMGR_AreaRangeUnset
* PURPOSE:
*     Unset ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : OSPF_PMGR_AreaNssaSet
* PURPOSE:
*     Set ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      nssa_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Nssa_Para_T *nssa_para);

/* FUNCTION NAME : OSPF_PMGR_AreaNssaUnset
* PURPOSE:
*     Unset ospf nssa area parameter or the whole area (if flag is 0)
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      flag.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag);

/* FUNCTION NAME : OSPF_PMGR_GetNextAreaPara
* PURPOSE:
*     Get nextospf area parameters.
*
* INPUT:
*     entry->first_flag,
*     entry->vr_id,
*     entry->proc_id
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextAreaPara(OSPF_TYPE_Area_Para_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetAreaPara
* PURPOSE:
*     Get  area parameters.
*
* INPUT:
*     
*     entry->vr_id
*     entry->proc_id
*     entry->area_id
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAreaPara(OSPF_TYPE_Area_Para_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextProcessStatus
* PURPOSE:
*     Unset ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      proc_id.
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextProcessStatus(UI32_T vr_id, UI32_T *proc_id);

/* FUNCTION NAME : OSPF_PMGR_AreaAggregateStatusSet
* PURPOSE:
*     set ospf area range status.
*
* INPUT:
*    vr_id
*    proc_id
*    area_id
*    type
*    range_addr,
*    range_mask,
*    status;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaAggregateStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T status);

/* FUNCTION NAME : OSPF_PMGR_AreaAggregateEffectSet
* PURPOSE:
*     set ospf area range effect.
*
* INPUT:
*    vr_id
*    proc_id
*    area_id
*    type
*    range_addr,
*    range_mask,
*    effect;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaAggregateEffectSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T effect);

/* FUNCTION NAME : OSPF_PMGR_GetAreaRangeTable
* PURPOSE:
*     Get ospf area range.
*
* INPUT:
*     entry->vr_id
*     entry->proc_id
*     entry->area_id
*     entry->type
*     entry->range_addr,
*     entry->range_mask;
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAreaRangeTable(OSPF_TYPE_Area_Range_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextAreaRangeTable
* PURPOSE:
*     Get nextospf area range.
*
* INPUT:
*     entry->indexlen
*     entry->vr_id
*     entry->proc_id
*     entry->area_id
*     entry->type
*     entry->range_addr,
*     entry->range_mask
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id, type, range_addr and range_mask.
*/
UI32_T OSPF_PMGR_GetNextAreaRangeTable(OSPF_TYPE_Area_Range_T *entry);

/* FUNCTION NAME : OSPF_PMGR_AreaVirtualLinkSet
* PURPOSE:
*     Set ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : OSPF_PMGR_AreaVirtualLinkUnset
* PURPOSE:
*     Unset ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : OSPF_PMGR_GetNextRoute
* PURPOSE:
*     Get nextospf route information.
*
* INPUT:
*     entry
*
* OUTPUT:
*     entry
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextRoute(OSPF_TYPE_Route_T *entry);
/* FUNCTION NAME : OSPF_PMGR_GetVirtualLinkEntry
* PURPOSE:
*     Get virtual link entry.
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextMultiProcVirtualLinkEntry
* PURPOSE:
*     
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextVirtualLinkEntry
* PURPOSE:
*     Getnext virtual link entry.
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetMultiProcVirtIfAuthMd5Entry
* PURPOSE:
*     Get virtual link md5 key of mib.
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcVirtIfAuthMd5Entry(OSPF_TYPE_Vlink_T *entry);


/* FUNCTION NAME : OSPF_PMGR_GetNextMultiProcVirtIfAuthMd5Entry
* PURPOSE: Get next virtual link md5 key of mib.
*     
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcVirtIfAuthMd5Entry(OSPF_TYPE_Vlink_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetMultiProcVirtNbrEntry
* PURPOSE:
*     Get virtual link nbr of mib.
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextMultiProcVirtIfAuthMd5Entry
* 
*PURPOSE: Get next virtual link nbr of mib.     
*
* INPUT:
*      
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNetworkAreaTable
* PURPOSE:
*     Get network entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->network_addr;
*      entry->network_pfx
*      
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextNetworkAreaTable
* PURPOSE:
*     Get next network entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->network_addr;
*      entry->network_pfx
*      
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, network_addr and network_pfx.
*/
UI32_T OSPF_PMGR_GetNextNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_AreaSummarySet
* PURPOSE:
*     Get next network entry.
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      val
*      
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_AreaSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T val);

/* FUNCTION NAME : OSPF_PMGR_GetAreaTable
* PURPOSE:
*     Get area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAreaTable(OSPF_TYPE_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextAreaTable
* PURPOSE:
*     Get next area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id.
*/
UI32_T OSPF_PMGR_GetNextAreaTable(OSPF_TYPE_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetStubAreaTable
* PURPOSE:
*     Get stub area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      entry->stub_tos;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetStubAreaTable(OSPF_TYPE_Stub_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextStubAreaTable
* PURPOSE:
*     Get next stub area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*      entry->stub_tos;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id, area_id and stub_tos.
*/
UI32_T OSPF_PMGR_GetNextStubAreaTable(OSPF_TYPE_Stub_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_StubAreaMetricSet
* PURPOSE:
*     Set stub area metric(default-cost).
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      stub_tos;
*      metric;
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_StubAreaMetricSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric);


/* FUNCTION NAME : OSPF_PMGR_StubAreaMetricTypeSet
* PURPOSE:
*     Set stub area metric-type
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      stub_tos;
*      metric;
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_StubAreaMetricTypeSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric_type);

/* FUNCTION NAME : OSPF_PMGR_StubAreaStatusSet
* PURPOSE:
*     Set stub area rowstatus.
*
* INPUT:
*      vr_id;
*      proc_id;
*      area_id;
*      stub_tos;
*      status;
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_StubAreaStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T status);

/* FUNCTION NAME : OSPF_PMGR_GetNssaTable
* PURPOSE:
*     Get nssa area entry.
*
* INPUT:
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNssaTable(OSPF_TYPE_Nssa_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextNssaTable
* PURPOSE:
*     Get next nssa area entry.
*
* INPUT:
*      entry->indexlen;
*      entry->vr_id;
*      entry->proc_id;
*      entry->area_id;
*
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      1.if entry->indexlen is 0,get the first entry in the proc_id
*      2.key are vr_id, proc_id and area_id.
*/
UI32_T OSPF_PMGR_GetNextNssaTable(OSPF_TYPE_Nssa_Area_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetPassIfTable
* PURPOSE:
*     Get ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      key are vr_id, proc_id, ifindex and addr.
*/
UI32_T OSPF_PMGR_GetPassIfTable(OSPF_TYPE_Passive_If_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetNextPassIfTable
* PURPOSE:
*     Get next ospf passive interface.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id,
*      entry->ifindex,
*      entry->addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
*
* NOTES:
*      key are vr_id, proc_id, ifindex and addr.
*/
UI32_T OSPF_PMGR_GetNextPassIfTable(OSPF_TYPE_Passive_If_T *entry);

/*wang.tong add */
/* FUNCTION NAME : OSPF_PMGR_GetIfParamEntry
* PURPOSE:
*     Get IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetIfParamEntry(UI32_T vr_id, UI32_T vrf_id , OSPF_TYPE_IfParam_T *entry);


/* FUNCTION NAME : OSPF_PMGR_GetNextIfParamEntry
* PURPOSE:
*     Get next IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      indexlen
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextIfParamEntry(UI32_T vr_id, UI32_T vrf_id, int indexlen, OSPF_TYPE_IfParam_T *entry);

/* FUNCTION NAME : OSPF_PMGR_SetIfParamEntry
* PURPOSE:
*     Set IfParam entry.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_SetIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry);

/* FUNCTION NAME: OSPF_PMGR_GetOperatingIfParamEntry
* PURPOSE:
*     Get operating IfParam entry for address or default value.
*
* INPUT:
*      vr_id
*      vrf_id
*      entry
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/NETCFG_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetOperatingIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_PMGR_SetAreaLimit
 *PURPOSE: Set area limit.
 *          
 *INPUT:   vr_id, proc_id, status
 *          
 *OUTPUT:
 *
 *NOTES: 
 *       
 *=============================================================
 */

UI32_T OSPF_PMGR_AreaLimitSet(UI32_T vr_id, UI32_T proc_id, UI32_T limit);


/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetMultiProcessSystemEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessSystem_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetNextMultiProcessSystemEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              indexlen
 *              entry
 *          
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetNextMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessSystem_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetMultiProcessNbrEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              entry
 *          
 *OUTPUT:
 *                  entry
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetMultiProcessNbrEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessNbr_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetNextMultiProcessNbrEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              indexlen
 *              entry
 *          
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetNextMultiProcessNbrEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessNbr_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetMultiProcessLsdbEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetMultiProcessLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessLsdb_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetNextMultiProcessLsdbEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              indexlen
 *              entry
 *          
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetNextMultiProcessLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessLsdb_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetMultiProcessExtLsdbEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetMultiProcessExtLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessExtLsdb_T *entry);



/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetNextMultiProcessExtLsdbEntry
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              indexlen
 *              entry
 *          
 *OUTPUT:entry
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetNextMultiProcessExtLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessExtLsdb_T *entry);


/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetIfAuthMd5Key
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              vrf_id
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */

UI32_T OSPF_PMGR_GetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_PMGR_GetNextIfAuthMd5Key
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              vrf_id
 *              indexlen
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */
UI32_T OSPF_PMGR_GetNextIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);

/*============================================================
 *FUNCTION NAME: OSPF_PMGR_SetIfAuthMd5Key
 *PURPOSE: 
 *          
 *INPUT:   
 *              vr_id
 *              vrf_id
 *              entry
 *          
 *OUTPUT:
 *
 *RETRUN:
 *
 *NOTES: 
 *       
 *=============================================================
 */

UI32_T OSPF_PMGR_SetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry);
/*end.     wang.tong*/

/* FUNCTION NAME : OSPF_PMGR_GetAutoCost
* PURPOSE:
*     Get autocost reference bandwidth.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ref_bandwidth.
*      
* OUTPUT:
*      None
*
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAutoCost(UI32_T vr_id, UI32_T proc_id, UI32_T *ref_bandwidth);

/* FUNCTION NAME : OSPF_PMGR_GetAsbrBorderRouter
* PURPOSE:
*     Get ASBR information.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAsbrBorderRouter(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_GetAbrBorderRouter
* PURPOSE:
*     Get ABR information.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetAbrBorderRouter(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_GetNextOspfEntry
* PURPOSE:
*     Getnext ospf entry.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextOspfEntry(OSPF_MGR_OSPF_ENTRY_T *ospf_entry_p);

/* FUNCTION NAME : OSPF_PMGR_GetOspfEntry
* PURPOSE:
*     Get ospf entry.
*
* INPUT:
*      entry->vr_id
*      entry->proc_id
*      entry->area_id
* OUTPUT:
*      entry
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetOspfEntry(OSPF_MGR_OSPF_ENTRY_T *ospf_entry_p);

/* FUNCTION NAME : OSPF_PMGR_ClearOspfProcess
* PURPOSE:
*     clear ip ospf process.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_ClearOspfProcess(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : OSPF_PMGR_ClearOspfProcessAll
* PURPOSE:
*     clear all ip ospf process.
*
* INPUT:
*      
*      
*
* OUTPUT:
*      None
* RETURN:
*       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_ClearOspfProcessAll(UI32_T vr_id);

/* FUNCTION NAME : OSPF_PMGR_GetNextOspfIfEntryByIfindex
* PURPOSE:
*     Get nex ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*
* NOTES:
*      None.
*/
UI32_T OSPF_PMGR_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : OSPF_PMGR_GetOspfMultiProcessRouteNexthopEntry
 * PURPOSE:
 *      Get OSPF route information for SNMP.
 *
 * INPUT:
 *      entry_p
 *      key are,
 *      process_id  -- Process ID of an OSPF instance.
 *      dest        -- The destination IP address of this route.
 *      pfx_len     -- The prefix length of this route.
 *      next_hop    -- The nexthop IP address of this route.
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_PMGR_GetOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p);

/* FUNCTION NAME : OSPF_PMGR_GetNextOspfMultiProcessRouteNexthopEntry
 * PURPOSE:
 *      Get next OSPF route information for SNMP.
 *
 * INPUT:
 *      entry_p
 *      key are,
 *      process_id  -- Process ID of an OSPF instance.
 *      dest        -- The destination IP address of this route.
 *      pfx_len     -- The prefix length of this route.
 *      next_hop    -- The nexthop IP address of this route.
 *
 * OUTPUT:
 *      entry_p
 *
 * RETURN:
 *      OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_PMGR_GetNextOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p);

/* FUNCTION NAME :  OSPF_PMGR_AreaAuthenticationTypeSet
 * PURPOSE:
 *     Set OSPF area authentication type.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      area_id
 *      type : NETCFG_TYPE_OSPF_AUTH_NULL / NETCFG_TYPE_OSPF_AUTH_SIMPLE /NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC
 *
 * OUTPUT:
 *      None
 * RETURN:
 *       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_PMGR_AreaAuthenticationTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T type);

/* FUNCTION NAME :  OSPF_PMGR_AreaAuthenticationTypeUnSet
 * PURPOSE:
 *     Set OSPF area authentication type.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      area_id
 *
 * OUTPUT:
 *      None
 * RETURN:
 *       OSPF_TYPE_RESULT_SUCCESS/OSPF_TYPE_RESULT_SEND_MSG_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T OSPF_PMGR_AreaAuthenticationTypeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id);

// peter add for mib
UI32_T OSPF_PMGR_GetRouterId(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_PMGR_GetVersionNumber(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetAreaBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetASBdrRtrStatus (UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetASBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_PMGR_GetExternLsaCount(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetExternLsaCksumSum(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_PMGR_GetOriginateNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetRxNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_GetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T limit);
UI32_T OSPF_PMGR_GetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_PMGR_GetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T interval);
UI32_T OSPF_PMGR_GetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret);
UI32_T OSPF_PMGR_SetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status);
UI32_T OSPF_PMGR_SetAreaStatus(OSPF_TYPE_Area_T *area);

UI32_T OSPF_PMGR_GetHostEntry(OSPF_TYPE_HostEntry_T *host_entry);
UI32_T OSPF_PMGR_GetNextHostEntry(OSPF_TYPE_HostEntry_T *host_entry);

UI32_T OSPF_PMGR_SetHostMetric(UI32_T vr_id, UI32_T proc_id,
		      UI32_T ip_addr, UI32_T tos, UI32_T metric);

UI32_T OSPF_PMGR_SetHostStatus(UI32_T vr_id, UI32_T proc_id,
		      UI32_T ip_addr, UI32_T tos, UI32_T status);
              
UI32_T OSPF_PMGR_SetIfAuthKey(UI32_T addr, char *auth_key);
UI32_T OSPF_PMGR_SetIfType(UI32_T addr, UI32_T value);
UI32_T OSPF_PMGR_SetIfAdminStat(UI32_T addr, UI32_T value);

UI32_T OSPF_PMGR_SetIfPollInterval(UI32_T addr, UI32_T value);

UI32_T OSPF_PMGR_GetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);
UI32_T OSPF_PMGR_GetNextIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);
UI32_T OSPF_PMGR_SetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p);

#endif

