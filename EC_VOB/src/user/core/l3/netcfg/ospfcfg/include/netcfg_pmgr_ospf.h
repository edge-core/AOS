/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_OSPF.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for OSPF MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef NETCFG_PMGR_OSPF_H
#define NETCFG_PMGR_OSPF_H

#include <sys/types.h>
#include "sys_pal.h"
#include "netcfg_type.h"
#include "ospf_type.h"

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_OSPF_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_OSPF in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_OSPF_InitiateProcessResource(void);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterOspfSet
* PURPOSE:
*     Set router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterOspfUnset
* PURPOSE:
*     Unset router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME :  NETCFG_PMGR_OSPF_IfAuthenticationTypeSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  NETCFG_PMGR_OSPF_IfAuthenticationTypeUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfAuthenticationKeySet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfAuthenticationKeyUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfMessageDigestKeySet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfMessageDigestKeyUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfPrioritySet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfPriorityUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfCostSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfCostUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfDeadIntervalSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfDeadIntervalUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfHelloIntervalSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfHelloIntervalUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfRetransmitIntervalSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfRetransmitIntervalUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfTransmitDelaySet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfTransmitDelayUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_NetworkSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_NetworkUnset
* PURPOSE:
*     Unset ospf network.
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterIdSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterIdUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_TimerSet
* PURPOSE:
*     Set ospf timer.
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_TimerUnset
* PURPOSE:
*     Set ospf timer to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubNoSummarySet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubNoSummaryUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaDefaultCostSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaDefaultCostUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeNoAdvertiseSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaNssaSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Nssa_Para_T *nssa_para);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaNssaUnset
* PURPOSE:
*     Unset ospf area nssa.
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetNextPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextNetwork
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetNextNetwork(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaVirtualLinkSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaVirtualLinkUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para);


/* FUNCTION NAME : NETCFG_PMGR_OspfSummaryAddressSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfSummaryAddressSet(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask);


/* FUNCTION NAME : NETCFG_PMGR_OspfSummaryAddressUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfSummaryAddressUnset(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask);


/* FUNCTION NAME : NETCFG_PMGR_OspfAutoCostSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfAutoCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T ref_bandwidth);


/* FUNCTION NAME : NETCFG_PMGR_OspfAutoCostUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfAutoCostUnset(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeProtoSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeProtoUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricTypeSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricTypeUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeTagSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeTagUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeRoutemapSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeRoutemapUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric_type
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricTypeUnset(UI32_T vr_id, UI32_T proc_id );


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id );


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      route_map,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoRoutemapUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoRoutemapUnset(UI32_T vr_id, UI32_T proc_id );


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoAlwaysSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoAlwaysUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoSet
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id);


/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoUnset
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetOspfIfEntry
* PURPOSE:
*     Get ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetOspfIfEntryByIfindex
* PURPOSE:
*     Get ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextOspfIfEntry
* PURPOSE:
*     Get nex ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetNextOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextOspfIfEntryByIfindex
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry);


#endif /* NETCFG_PMGR_OSPF_H */

